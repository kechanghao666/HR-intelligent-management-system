# HR智能问答系统

基于C++17、Qt Widgets和SQLite的公司HR制度管理与问答桌面程序。

## 功能

- 账号注册、登录与用户管理。
- 制度文档管理、文本处理、关键词检索及相关条款查看。
- 本地制度问答与可选讯飞在线问答。
- 表单模板、文档查看、统计图表及报告导出。

## 构建

将项目放在英文路径（例如C:\HRSystem）。安装Qt 6的MinGW x64桌面套件，包含Core、Gui、Widgets、Network、Sql、PrintSupport和Xml模块。在相应Qt命令行环境执行：

```powershell
mkdir build-public
cd build-public
qmake ../HRSystem.pro CONFIG+=release
mingw32-make -j4
.\bin\release\HRSystem.exe
```

程序首次启动创建本机SQLite数据库，之后通过登录或注册入口进入系统。数据库及设置保存在当前用户的应用数据目录。

## 在线问答配置

根据实际讯飞接口设置XFYUN_API_PASSWORD，或XFYUN_APP_ID、XFYUN_API_KEY、XFYUN_API_SECRET。程序也支持从本机QSettings读取配置。凭据由使用者自行提供，不写入源码。

## 发布到其他Windows电脑

使用与编译器匹配的windeployqt收集Qt运行库和插件，包括Windows平台插件及SQLite驱动。复制完整部署目录后运行。源码仓库不保存本机数据库、账户记录或编译产物。

作者：柯长浩。
