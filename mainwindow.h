#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QObject>
#include <QMainWindow>
#include <QTabWidget>
#include <QMenuBar>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QDateTime>
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QShowEvent>
#include <QToolButton>
#include <QWidgetAction>
#include "user.h"
#include "database.h"
#include "qaengine.h"
#include "modernqawidget.h"
#include "aiassistantwidget.h"
#include "adminpanel.h"
#include "chartwidget.h"
#include "weatherwidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const User& currentUser, Database* database, QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onTabChanged(int index);
    void onLogoutClicked();
    void onSettingsClicked();
    void onHelpClicked();
    void onAboutClicked();
    void onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMainWindow();
    void onQuitApplication();
    void updateStatusBar();
    void onUserActivity();
    
    // 崩溃保护槽函数
    void onCrashDetected();
    void onComponentError();
    void onEmergencyRestore();

signals:
    void logoutRequested();

private:
    User currentUser_;
    Database* database_;
    QAEngine* qaEngine_;
    
    // UI组件
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;
    QTabWidget* tabWidget_;
    QWidget* qaTab_;
    QWidget* adminTab_;
    QWidget* chartTab_;
    
    // 工具栏和菜单
    QMenuBar* menuBar_;
    QToolBar* toolBar_;
    QStatusBar* statusBar_;
    QMenu* fileMenu_;
    QMenu* toolsMenu_;
    QMenu* helpMenu_;
    QMenu* systemTrayMenu_;
    
    // 动作
    QAction* logoutAction_;
    QAction* settingsAction_;
    QAction* helpAction_;
    QAction* aboutAction_;
    QAction* showAction_;
    QAction* quitAction_;
    
    // 系统托盘
    QSystemTrayIcon* systemTrayIcon_;
    
    // 功能组件
    ModernQAWidget* qaWidget_;
    AIAssistantWidget* aiAssistantWidget_;
    AdminPanel* adminPanel_;
    ChartWidget* chartWidget_;
    WeatherWidget* weatherWidget_;
    
    // 状态相关
    QTimer* statusTimer_;
    QTimer* activityTimer_;
    QDateTime lastActivityTime_;
    
    // 崩溃保护和恢复
    QTimer* crashProtectionTimer_;
    QTimer* autoSaveTimer_;
    int crashCounter_;
    QDateTime lastCrashTime_;
    QString crashLogFile_;

    // 私有方法
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupSystemTray();
    void setupConnections();
    void applyStyles();
    void centerWindow();
    void createTabs();
    void setupPermissions();
    void showWelcomeMessage();
    void logUserActivity();
    void checkUserSession();
    
    // 崩溃保护相关
    void setupCrashProtection();
    void onCrashProtectionCheck();
    void onAutoSave();
    void handleComponentCrash(const QString& componentName);
    void restoreFromCrash();
    void logCrashEvent(const QString& details);
    void recreateFailedComponents();
    bool isComponentHealthy(QWidget* component);
    
    // 样式相关
    void createGradientBackground();
    void setupWindowIcon();
    void applyWindowStyle();
};

#endif // MAINWINDOW_H
