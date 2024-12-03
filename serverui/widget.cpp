#include "widget.h"
#include "ui_widget.h"
#include <QString>
#include <cmath>
#include <limits>
#include <QByteArray>
#include <QMap>
#include <QPair>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTextStream>
#include <QDebug>
#include <QCryptographicHash>
#include <QDateTime>
#include <QtSql/QSqlDatabase>
#include <QRandomGenerator>
#include <QtSql/QSqlError>


QMap<QString, QPair<QString, int>> otpStorage;
QMap<QTcpSocket*, QDateTime> clientConnectionTimes;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
    , tcpServer(new QTcpServer(this))
    , clientSocket(nullptr)
    , currentCounter(0)

{
    ui->setupUi(this);
    if (!tcpServer->listen(QHostAddress::Any, 12345)) {
        qWarning() << "Server could not start!";
    } else {
        qDebug() << "Server started!";
    }


    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setDatabaseName("DNS=TestSQL01;DRIVER={SQL Server};SERVER=LAPTOP-8G40GCUN;DATABASE=Test01;UID=sa;PWD=123456");
    if (!db.open()) {
        qWarning() << "Unable to open database:" << db.lastError();
    } else {
        qDebug() << "Database opened successfully";
    }
    connect(tcpServer, &QTcpServer::newConnection, this, &Widget::onNewConnection);
    //connect(ui->pushButton, &QPushButton::clicked, this, &Widget::onGenerateOtpClicked);
}


void Widget::appendToTextEdit(const QString &message) {
    ui->textEdit->append(message); // 将消息添加到文本框
}


QString Widget::generateHOTP(const QString &secret, int counter) {
    QByteArray secretBytes = secret.toUtf8();
    QByteArray counterBytes = QByteArray::number(counter, 10).leftJustified(8, '0');

    // Prepare the input for HMAC-SHA1
    QByteArray hashInput = secretBytes.append(counterBytes);

    // Compute HMAC-SHA1
    QCryptographicHash hash(QCryptographicHash::Sha1);
    hash.addData(hashInput);
    QByteArray hashValue = hash.result();

    // Dynamic truncation to determine the offset
    quint32 offset = hashValue.mid(hashValue.size() - 4).toUInt(nullptr, 16) % hashValue.size();

    // Extract 4 bytes from the offset position
    quint32 binaryCode = ((hashValue[offset] & 0x7f) << 24)
                         | ((hashValue[offset + 1] & 0xff) << 16)
                         | ((hashValue[offset + 2] & 0xff) << 8)
                         | ((hashValue[offset + 3] & 0xff));

    // Compute the OTP value modulo 10^6 to get a 6-digit number
    binaryCode = binaryCode % static_cast<quint32>(pow(10, 6));

    // Format the OTP as a 6-digit string, padded with leading zeros if necessary
    QString otp = QString::number(binaryCode, 10).rightJustified(6, '0');
    return otp;
}




void Widget::storeOTP(const QString &userId, const QString &otp, int counter) {
    otpStorage[userId] = qMakePair(otp, counter);
}

bool Widget::verifyOTP(const QString &userId, const QString &otp) {
    if (otpStorage.contains(userId)) {
        QPair<QString, int> otpData = otpStorage[userId];
        if (otpData.first == otp) {
            otpStorage.remove(userId); // OTP used, remove it
            return true;
        }
    }
    return false;
}

void Widget::onNewConnection()
{

    clientSocket = tcpServer->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &Widget::onReadyRead);


    clientConnectionTimes[clientSocket] = QDateTime::currentDateTime();
    QTextStream out(clientSocket);
    out << "REQUEST_TIME\n";
    out.flush();
}

void Widget::onReadyRead()
{
    QTextStream in(clientSocket);
    QString request = in.readLine().trimmed();

    if (request == "CLIENT_TIME") {
        QString clientTimeStr = in.readLine().trimmed();
        QDateTime clientTime = QDateTime::fromString(clientTimeStr, "yyyy-MM-dd HH:mm:ss");
        if (clientTime.isValid()) {
            QString logMessage = QString("客户端时间: %1").arg(clientTime.toString(Qt::ISODate));
            appendToTextEdit(logMessage);
            // 这里可以添加处理时间的逻辑，比如发送回服务器或其他操作
        } else {
            QString logMessage = "Received invalid client time format.";
            appendToTextEdit(logMessage);
        }
    } else if (request.startsWith("USERNAME:")) {
        QStringList parts = request.split(":", Qt::SkipEmptyParts);
        if (parts.size() == 3) { // 正确的格式应该是 "USERNAME:<username>:<password>"
            QString username = parts[1];
            QString password = parts[2];

            int authenticated = loginManager(db, username, password);
            if(authenticated!=0){
                onGenerateOtpClicked(username);
            }
        } else {
            QString unknownMessage = QString("收到格式错误的登录请求: %1").arg(request);
            appendToTextEdit(unknownMessage);
        }
    }   else if(request.startsWith("OTP:")){
        QStringList parts2 = request.split(":", Qt::SkipEmptyParts);
        if (parts2.size() == 4){
            QString username2 = parts2[2];
            QString password2 = parts2[3];
            QString otp = parts2[1];
            int authenticated = loginManager(db, username2, password2);
            if(verifyOTP(username2,otp)){
                QTextStream out(clientSocket);
                out << "OTP:" << (authenticated) << "\n";
                out.flush();
            }
        }
    }
    else {
        QString unknownMessage = QString("收到未知消息: %1").arg(request);
        appendToTextEdit(unknownMessage);
    }
}

void Widget::onDisconnected()
{
    clientSocket->deleteLater();
    clientSocket = nullptr;
    clientConnectionTimes.remove(clientSocket);
}

int Widget::loginManager(QSqlDatabase& db, const QString& username, const QString& password) {
    QSqlQuery query(db);
    QString hashedPassword = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
    QString sql = "SELECT password,time,quanxian FROM Table_3 WHERE Username = :username";
    query.prepare(sql);
    query.bindValue(":username", username);
    query.bindValue("time", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    if (!query.exec() || !query.next()) {
        qDebug() << "Error: User login failed. User not found or query error." << query.lastError().text();
        return false;
    }

    QString storedHashedPassword = query.value(0).toString();
    if (hashedPassword == storedHashedPassword) {
        QString userquanxian = query.value(2).toString();
        if (userquanxian == "SecurityAdministrator")
        {
            return 1;
        }
        if(userquanxian == "LogAdministrator"){
            return 2;
        }
    }


    return 0;
}

void Widget::onGenerateOtpClicked(QString userid)
{
    QString secret = "123456"; // 替换为您的秘密密钥
    currentCounter++; // 增加计数器
    QString randomNumber = generateRandomNumber(); // 生成随机数R

    QString otp = generateHOTP(secret, currentCounter); // 使用N和秘密密钥生成OTP

    // 存储OTP信息（这里仅作为示例，实际应用中可能需要更安全的存储方式）
    // 注意：这里不存储OTP本身，只存储用于验证的N和R（或生成的哈希值）
    storeOTP(userid,otp, currentCounter);  // 如果需要存储OTP，可以使用这个函数

    // 更新UI
    updateUiWithOtpInfo(currentCounter, randomNumber);

    // 可以在这里添加将OTP发送给用户的逻辑
    QTextStream out(clientSocket);
    out << "USERNAME:" << otp;
    out.flush();
}

QString Widget::generateRandomNumber()
{
    // 生成一个6位的随机数作为R
    int randomInt = QRandomGenerator::global()->bounded(100000, 999999);
    return QString::number(randomInt);
}

void Widget::updateUiWithOtpInfo(int counter, QString randomNumber)
{
    ui->lineEdit->setText(QString::number(counter)); // 更新计数器N的显示
    ui->lineEdit_2->setText(randomNumber); // 更新随机数R的显示
}


Widget::~Widget()
{
    db.close();
    delete ui;
}

