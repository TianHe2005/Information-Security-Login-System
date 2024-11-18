## 一、系统概述
本安全审计登录注册管理员系统是为了保障系统安全，对用户登录、注册行为进行有效管理和审计的应用程序。它提供了可靠的身份验证机制和全面的审计功能，确保只有授权的管理员能够访问和操作系统相关功能。
## 二、功能特点
### （一）登录功能
多因素身份验证
系统支持多种身份验证方式，包括但不限于用户名和密码、验证码（可集成图形验证码、短信验证码等机制），增强了登录的安全性，防止恶意登录尝试。
密码安全策略
对用户密码有严格的要求，如长度限制、包含大小写字母、数字和特殊字符等，通过正则表达式判断，并且对用户输入的密码进行加密存储，使用先进的加密算法（如MD5）保障密码安全。
登录审计
每次登录尝试都会被详细记录，包括登录时间、IP 地址、用户名等信息。对于连续多次失败的登录尝试，可以触发安全预警机制，如暂时锁定账户或通知管理员。
### （二）注册功能
信息完整性验证
在用户注册过程中，要求用户填写完整且准确的信息，如姓名、联系方式、邮箱等，并对输入信息进行格式验证，确保数据的合法性。
唯一性检查
对用户名、邮箱等关键信息进行唯一性检查，防止重复注册，确保系统用户信息的准确性和唯一性。
注册审批（可选）
对于新注册的用户，可以设置管理员审批流程，新用户注册信息将提交给管理员审核，只有审核通过后用户才能正常登录和使用系统，进一步加强系统的安全性。
### （三）管理员功能
用户管理
管理员可以查看、编辑和删除系统中的用户信息，包括重置用户密码、修改用户权限等操作。
能够对用户进行分类管理，如根据用户角色、部门等划分不同的用户组，方便批量管理和权限分配。
审计日志查询
管理员可以查询和分析系统的登录注册审计日志，根据不同的条件（如时间范围、用户名、IP 地址等）进行筛选和排序，以便及时发现异常登录注册行为，并采取相应的措施。
安全设置更新
可以对系统的安全设置进行更新，如修改密码策略、验证码类型和有效期、登录失败锁定策略等，确保系统安全机制能够适应不断变化的安全环境。
## 三、技术架构
客户端 - 服务端（C/S）
数据库用的是sql server,通过ODBC连接
使用Qt的network模块通过TCP/IP 协议通信

## 四、安装与部署
环境要求
服务器环境：，需要安装 
数据库环境：安装并配置 [sql server]，创建必要的数据库和表结构，可以使用提供的数据库初始化脚本。
安装步骤
下载系统安装包（![下载地址](https://www.microsoft.com/zh-cn/sql-server/sql-server-downloads?msockid=3b801dec7c1467ee227f08d87d1166ff)到服务器指定目录。
解压安装包，按照安装向导的提示进行配置，包括数据库连接配置、服务器端口设置等。
启动系统服务，可通过命令行或启动脚本启动后端服务，前端页面可以通过 Web 服务器进行部署和访问。
## 五、使用说明
用户登录
在登录页面输入用户名和密码，根据系统要求输入验证码（如果有）。
如果是首次登录，可能需要修改初始密码或完成其他初始化步骤。
用户注册
访问注册页面，填写必填信息，提交注册申请。
如果有注册审批流程，等待管理员审核通过后即可登录。
管理员操作
使用管理员账号登录系统后，通过导航菜单进入用户管理、审计日志查询和安全设置等功能页面，按照页面提示进行相应操作。
## 六、安全注意事项
定期更新
为了保持系统的安全性，应及时更新系统版本，包括后端代码、前端页面和依赖的第三方库，以修复可能存在的安全漏洞。
网络安全防护
确保服务器所在网络环境的安全性，部署防火墙、入侵检测系统等网络安全设备，防止外部网络攻击。
数据备份
定期对系统数据（特别是用户信息和审计日志）进行备份，防止数据丢失或损坏，并将备份数据存储在安全的位置。
安全意识培训
对系统管理员和用户进行安全意识培训，让他们了解安全审计系统的重要性和正确的使用方法，避免因人为疏忽导致的安全问题。
