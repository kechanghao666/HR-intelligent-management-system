#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include "database.h"
#include "loginwindow.h"
#include "registerwindow.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置高DPI支持（Qt 6.x中这些属性已默认启用，无需手动设置）
    // app.setAttribute(Qt::AA_UseHighDpiPixmaps);     // Qt 6中已过时，默认启用
    // app.setAttribute(Qt::AA_EnableHighDpiScaling);  // Qt 6中已过时，默认启用
    
    // 设置应用程序信息
    app.setApplicationName("HR智能问答系统");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Company HR System");
    app.setOrganizationDomain("hr.company.com");
    
    // 设置应用程序样式
    app.setStyle(QStyleFactory::create("Fusion"));
    
    // 加载样式表 - 先检查用户主题设置
    QSettings settings("HRSystem", "Settings");
    int themeIndex = settings.value("appearance/theme", 0).toInt();
    
    QString styleFileName = ":/styles/main.qss";  // 默认浅色主题
    if (themeIndex == 1) {
        // 深色主题
        styleFileName = ":/styles/modern.qss";
        QPalette pal = app.palette();
        pal.setColor(QPalette::Window, QColor(30,30,30));
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Base, QColor(45,45,45));
        pal.setColor(QPalette::AlternateBase, QColor(60,60,60));
        pal.setColor(QPalette::ToolTipBase, Qt::white);
        pal.setColor(QPalette::ToolTipText, Qt::white);
        pal.setColor(QPalette::Text, Qt::white);
        pal.setColor(QPalette::Button, QColor(45,45,45));
        pal.setColor(QPalette::ButtonText, Qt::white);
        pal.setColor(QPalette::BrightText, Qt::red);
        pal.setColor(QPalette::Highlight, QColor(59, 130, 246));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        app.setPalette(pal);
    }
    
    QFile styleFile(styleFileName);
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream styleStream(&styleFile);
        app.setStyleSheet(styleStream.readAll());
        styleFile.close();
    }
    
    // 创建数据目录
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    
    // 初始化数据库
    Database* database = new Database();
    if (!database->initialize()) {
        QMessageBox::critical(nullptr, "错误", "无法初始化数据库，程序将退出。");
        return -1;
    }
    
    // 创建登录窗口
    LoginWindow* loginWindow = new LoginWindow(database);
    loginWindow->show();
    
    // 【内存管理修复】连接登录成功信号，确保窗口对象正确管理
    MainWindow* mainWindow = nullptr; // 主窗口指针，确保生命周期管理
    
    QObject::connect(loginWindow, &LoginWindow::loginSuccessful, 
        [&](const User& user) {
            loginWindow->hide();
            // 如果已有主窗口，先清理
            if (mainWindow) {
                mainWindow->deleteLater();
                mainWindow = nullptr;
            }
            
            mainWindow = new MainWindow(user, database);
            mainWindow->showMaximized();  // 登录成功后默认最大化显示
            
            // 连接登出信号，确保窗口正确切换
            QObject::connect(mainWindow, &MainWindow::logoutRequested, [&]() {
                if (mainWindow) {
                    mainWindow->deleteLater();
                    mainWindow = nullptr;
                }
                if (loginWindow) {
                    loginWindow->show();
                    loginWindow->clearFields(); // 清空登录表单
                }
            });
        });
    
    // 【内存管理修复】连接注册窗口信号，确保注册窗口对象正确管理
    RegisterWindow* registerWindow = nullptr; // 注册窗口指针，确保生命周期管理
    
    QObject::connect(loginWindow, &LoginWindow::showRegisterWindow, [&]() {
        loginWindow->hide();
        
        // 如果已有注册窗口，先清理
        if (registerWindow) {
            registerWindow->deleteLater();
            registerWindow = nullptr;
        }
        
        registerWindow = new RegisterWindow(database);
        registerWindow->show();
        
        // 连接注册成功信号
        QObject::connect(registerWindow, &RegisterWindow::registrationSuccessful, 
            [&](const User& user) {
                Q_UNUSED(user)
                // 注册成功后，关闭注册窗口并返回登录窗口
                if (registerWindow) {
                    registerWindow->hide();
                    registerWindow->deleteLater();
                    registerWindow = nullptr;
                }
                if (loginWindow) {
                    loginWindow->show();  // 显示登录窗口
                }
            });
        
        // 连接返回登录信号
        QObject::connect(registerWindow, &RegisterWindow::showLoginWindow, [&]() {
            if (registerWindow) {
                registerWindow->hide();
                registerWindow->deleteLater();
                registerWindow = nullptr;
            }
            if (loginWindow) {
                loginWindow->show();  // 返回登录界面
            }
        });
    });
    
    int result = app.exec();
    
    // 【内存管理修复】程序退出时清理所有资源
    qDebug() << "[程序退出] 开始清理资源...";
    
    // 清理窗口对象
    if (mainWindow) {
        mainWindow->deleteLater();
        mainWindow = nullptr;
    }
    if (registerWindow) {
        registerWindow->deleteLater();
        registerWindow = nullptr;
    }
    if (loginWindow) {
        loginWindow->deleteLater();
        loginWindow = nullptr;
    }
    
    // 清理数据库连接
    if (database) {
        delete database;
        database = nullptr;
    }
    
    qDebug() << "[程序退出] 资源清理完成";
    return result;
}
