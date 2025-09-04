#include "mainwindow.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QStringConverter>

MainWindow::MainWindow(const User& currentUser, Database* database, QWidget *parent)
    : QMainWindow(parent)
    , currentUser_(currentUser)
    , database_(database)
    , qaEngine_(nullptr)
    , centralWidget_(nullptr)
    , mainLayout_(nullptr)
    , tabWidget_(nullptr)
    , qaTab_(nullptr)
    , adminTab_(nullptr)
    , chartTab_(nullptr)
    , menuBar_(nullptr)
    , toolBar_(nullptr)
    , statusBar_(nullptr)
    , fileMenu_(nullptr)
    , toolsMenu_(nullptr)
    , helpMenu_(nullptr)
    , systemTrayMenu_(nullptr)
    , logoutAction_(nullptr)
    , settingsAction_(nullptr)
    , helpAction_(nullptr)
    , aboutAction_(nullptr)
    , showAction_(nullptr)
    , quitAction_(nullptr)
    , systemTrayIcon_(nullptr)
    , qaWidget_(nullptr)
    , aiAssistantWidget_(nullptr)
    , adminPanel_(nullptr)
    , chartWidget_(nullptr)
    , statusTimer_(nullptr)
    , activityTimer_(nullptr)
    , lastActivityTime_(QDateTime::currentDateTime())
    , crashProtectionTimer_(nullptr)
    , autoSaveTimer_(nullptr)
    , crashCounter_(0)
    , lastCrashTime_(QDateTime::currentDateTime())
    , crashLogFile_("")
{
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupSystemTray();
    setupConnections();
    setupCrashProtection();
    centerWindow();
    applyStyles();
    createTabs();
    setupPermissions();
    showWelcomeMessage();
}

MainWindow::~MainWindow()
{
    if (statusTimer_) {
        statusTimer_->stop();
    }
    if (activityTimer_) {
        activityTimer_->stop();
    }
    if (crashProtectionTimer_) {
        crashProtectionTimer_->stop();
    }
    if (autoSaveTimer_) {
        autoSaveTimer_->stop();
    }
    delete qaEngine_;
}

void MainWindow::setupUI()
{
    setWindowTitle(QString("HR智能问答系统 - 欢迎 %1 (%2)")
                   .arg(currentUser_.getName())
                   .arg(currentUser_.isHRAdmin() ? "HR管理员" : "员工"));
    
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    setupWindowIcon();
    
    // 创建现代化中心组件
    centralWidget_ = new QWidget();
    centralWidget_->setObjectName("mainWidget");
    setCentralWidget(centralWidget_);
    
    mainLayout_ = new QVBoxLayout(centralWidget_);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(16);
    
    // 创建现代化标签页组件
    tabWidget_ = new QTabWidget();
    tabWidget_->setTabPosition(QTabWidget::North);
    tabWidget_->setMovable(false);
    tabWidget_->setDocumentMode(true);
    tabWidget_->setObjectName("mainTabWidget");
    
    // 顶部加入天气小组件
    weatherWidget_ = new WeatherWidget(this);
    weatherWidget_->setFixedWidth(320);
    // 以悬浮工具形式叠加在中心区，不占主布局高度
    weatherWidget_->setParent(this);
    weatherWidget_->move(20, 50);
    weatherWidget_->show();
    mainLayout_->addWidget(tabWidget_);
}

void MainWindow::setupMenuBar()
{
    menuBar_ = this->menuBar();
    
    // 账号菜单
    fileMenu_ = menuBar_->addMenu("账号(&A)");
    
    logoutAction_ = new QAction("注销(&L)", this);
    logoutAction_->setShortcut(QKeySequence("Ctrl+L"));
    logoutAction_->setStatusTip("注销当前用户");
    fileMenu_->addAction(logoutAction_);
    
    fileMenu_->addSeparator();
    
    quitAction_ = new QAction("退出(&Q)", this);
    quitAction_->setShortcut(QKeySequence::Quit);
    quitAction_->setStatusTip("退出程序");
    fileMenu_->addAction(quitAction_);
    
    // 工具菜单
    toolsMenu_ = menuBar_->addMenu("工具(&T)");
    
    settingsAction_ = new QAction("设置(&S)", this);
    settingsAction_->setShortcut(QKeySequence("Ctrl+,"));
    settingsAction_->setStatusTip("系统设置");
    toolsMenu_->addAction(settingsAction_);
    
    // 帮助菜单
    helpMenu_ = menuBar_->addMenu("帮助(&H)");
    
    helpAction_ = new QAction("使用帮助(&H)", this);
    helpAction_->setShortcut(QKeySequence::HelpContents);
    helpAction_->setStatusTip("查看使用帮助");
    helpMenu_->addAction(helpAction_);
    
    helpMenu_->addSeparator();
    
    aboutAction_ = new QAction("关于(&A)", this);
    aboutAction_->setStatusTip("关于本程序");
    helpMenu_->addAction(aboutAction_);
    
    // 在菜单栏右侧添加工具栏按钮
    menuBar_->addSeparator();
    
    // 创建现代化工具栏按钮
    QToolButton* logoutBtn = new QToolButton();
    logoutBtn->setText("注销");
    logoutBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    connect(logoutBtn, &QToolButton::clicked, logoutAction_, &QAction::trigger);
    
    QToolButton* settingsBtn = new QToolButton();
    settingsBtn->setText("设置");
    settingsBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    connect(settingsBtn, &QToolButton::clicked, settingsAction_, &QAction::trigger);
    
    QToolButton* helpBtn = new QToolButton();
    helpBtn->setText("帮助");
    helpBtn->setToolButtonStyle(Qt::ToolButtonTextOnly);
    connect(helpBtn, &QToolButton::clicked, helpAction_, &QAction::trigger);
    
    // 创建QWidgetAction并添加按钮
    QWidgetAction* logoutWidgetAction = new QWidgetAction(this);
    logoutWidgetAction->setDefaultWidget(logoutBtn);
    menuBar_->addAction(logoutWidgetAction);
    
    QWidgetAction* settingsWidgetAction = new QWidgetAction(this);
    settingsWidgetAction->setDefaultWidget(settingsBtn);
    menuBar_->addAction(settingsWidgetAction);
    
    QWidgetAction* helpWidgetAction = new QWidgetAction(this);
    helpWidgetAction->setDefaultWidget(helpBtn);
    menuBar_->addAction(helpWidgetAction);
}

void MainWindow::setupToolBar()
{
    // 不再创建单独的工具栏，功能已合并到菜单栏
    toolBar_ = nullptr;
}

void MainWindow::setupStatusBar()
{
    statusBar_ = this->statusBar();
    
    // 创建状态标签
    QLabel* userLabel = new QLabel(QString("当前用户: %1 (%2)")
                                   .arg(currentUser_.getName())
                                   .arg(currentUser_.getDepartment()));
    statusBar_->addWidget(userLabel);
    
    statusBar_->addPermanentWidget(new QLabel("就绪"));
    
    // 创建状态更新定时器
    statusTimer_ = new QTimer(this);
    connect(statusTimer_, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    statusTimer_->start(60000); // 每分钟更新一次
    
    // 创建活动检测定时器
    activityTimer_ = new QTimer(this);
    connect(activityTimer_, &QTimer::timeout, this, &MainWindow::checkUserSession);
    activityTimer_->start(300000); // 每5分钟检查一次
}

void MainWindow::setupSystemTray()
{
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        return;
    }
    
    systemTrayIcon_ = new QSystemTrayIcon(this);
    systemTrayIcon_->setIcon(windowIcon());
    systemTrayIcon_->setToolTip("HR智能问答系统");
    
    // 创建系统托盘菜单
    systemTrayMenu_ = new QMenu(this);
    
    showAction_ = new QAction("显示主窗口", this);
    systemTrayMenu_->addAction(showAction_);
    
    systemTrayMenu_->addSeparator();
    systemTrayMenu_->addAction(quitAction_);
    
    systemTrayIcon_->setContextMenu(systemTrayMenu_);
    systemTrayIcon_->show();
}

void MainWindow::setupConnections()
{
    // 菜单动作连接
    connect(logoutAction_, &QAction::triggered, this, &MainWindow::onLogoutClicked);
    connect(settingsAction_, &QAction::triggered, this, &MainWindow::onSettingsClicked);
    connect(helpAction_, &QAction::triggered, this, &MainWindow::onHelpClicked);
    connect(aboutAction_, &QAction::triggered, this, &MainWindow::onAboutClicked);
    connect(quitAction_, &QAction::triggered, this, &MainWindow::onQuitApplication);
    
    // 系统托盘连接
    if (systemTrayIcon_) {
        connect(systemTrayIcon_, &QSystemTrayIcon::activated, 
                this, &MainWindow::onSystemTrayActivated);
        connect(showAction_, &QAction::triggered, this, &MainWindow::onShowMainWindow);
    }
    
    // 标签页切换连接
    connect(tabWidget_, &QTabWidget::currentChanged, this, &MainWindow::onTabChanged);
}

void MainWindow::centerWindow()
{
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void MainWindow::applyStyles()
{
    createGradientBackground();
    applyWindowStyle();
    
    // 应用更现代的样式
    setStyleSheet(styleSheet() +
        "QTabWidget::pane {"
        "    border: 1px solid #e1e8ed;"
        "    border-radius: 10px;"
        "    background: white;"
        "    margin-top: -1px;"
        "    padding: 5px;"
        "}"
        
        "QTabBar::tab {"
        "    padding: 14px 25px;"
        "    margin-right: 4px;"
        "    background: #f8f9fa;"
        "    border: 1px solid #e1e8ed;"
        "    border-bottom: none;"
        "    border-top-left-radius: 10px;"
        "    border-top-right-radius: 10px;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    color: #2c3e50;"
        "}"
        
        "QTabBar::tab:selected {"
        "    background: white;"
        "    border-bottom: 3px solid #4facfe;"
        "    color: #4facfe;"
        "}"
        
        "QTabBar::tab:hover:!selected {"
        "    background: #e3f2fd;"
        "    color: #4facfe;"
        "}"
        
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: #f0f0f0;"
        "    width: 10px;"
        "    border-radius: 5px;"
        "    margin: 0;"
        "}"
        
        "QScrollBar::handle:vertical {"
        "    background: #c0c0c0;"
        "    border-radius: 5px;"
        "    min-height: 20px;"
        "}"
        
        "QScrollBar::handle:vertical:hover {"
        "    background: #a0a0a0;"
        "}"
        
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    height: 0;"
        "    background: none;"
        "}"
        
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
        "    background: none;"
        "}"
    );
}

void MainWindow::createTabs()
{
    // 调试输出用户信息
    qDebug() << "[权限控制] 当前用户信息:";
    qDebug() << "  工号:" << currentUser_.getEmployeeId();
    qDebug() << "  姓名:" << currentUser_.getName();
    qDebug() << "  部门:" << currentUser_.getDepartment();
    qDebug() << "  角色:" << static_cast<int>(currentUser_.getRole()) << 
                (currentUser_.isHRAdmin() ? "(HR管理员)" : "(普通员工)");
    qDebug() << "  状态:" << static_cast<int>(currentUser_.getStatus());
    
    // 创建问答引擎
    qaEngine_ = new QAEngine(database_, this);
    
    // 问答标签页 - 所有用户都可以访问
    qaWidget_ = new ModernQAWidget(qaEngine_, database_, currentUser_, this);
    qaTab_ = qaWidget_;
    tabWidget_->addTab(qaTab_, "智能问答");
    qDebug() << "[权限控制] ✅ 已添加智能问答标签页";
    
    // AI助手标签页 - 简洁的AI问答体验
    aiAssistantWidget_ = new AIAssistantWidget(this);
    tabWidget_->addTab(aiAssistantWidget_, "AI助手");
    qDebug() << "[权限控制] ✅ 已添加AI助手标签页";
    
    // 权限检查：只有HR管理员才能看到管理功能
    if (currentUser_.isHRAdmin()) {
        qDebug() << "[权限控制] ✅ 用户是HR管理员，添加管理员专用功能";
        
        adminPanel_ = new AdminPanel(database_, currentUser_, this);
        adminTab_ = adminPanel_;
        tabWidget_->addTab(adminTab_, "管理后台");
        qDebug() << "[权限控制] ✅ 已添加管理后台标签页";
        
        // 数据可视化标签页 - 仅HR管理员可见
        chartWidget_ = new ChartWidget(database_, currentUser_, this);
        chartTab_ = chartWidget_;
        tabWidget_->addTab(chartTab_, "数据统计");
        qDebug() << "[权限控制] ✅ 已添加数据统计标签页";
        
    } else {
        qDebug() << "[权限控制] ⚠️ 用户是普通员工，不添加管理员功能";
        // 明确设置为nullptr以防止意外使用
        adminPanel_ = nullptr;
        adminTab_ = nullptr;
        chartWidget_ = nullptr;
        chartTab_ = nullptr;
    }
    
    qDebug() << "[权限控制] 标签页创建完成，总数:" << tabWidget_->count();
}

void MainWindow::setupPermissions()
{
    qDebug() << "[权限设置] 开始设置用户权限";
    
    // 所有用户都可以使用设置功能
    if (settingsAction_) {
        settingsAction_->setEnabled(true);
        qDebug() << "[权限设置] ✅ 设置功能已启用";
    }
    
    // 根据用户角色设置其他权限
    if (!currentUser_.isHRAdmin()) {
        qDebug() << "[权限设置] 配置普通员工权限限制";
        
        // 确保普通员工无法访问管理员功能
        if (chartWidget_) {
            chartWidget_->setVisible(false);
            qDebug() << "[权限设置] ⚠️ 检测到图表组件，已隐藏";
        }
        if (adminPanel_) {
            adminPanel_->setVisible(false);
            qDebug() << "[权限设置] ⚠️ 检测到管理面板，已隐藏";
        }
        
        // 隐藏相关标签页（如果存在）
        for (int i = 0; i < tabWidget_->count(); ++i) {
            QString tabText = tabWidget_->tabText(i);
            if (tabText == "数据统计" || tabText == "管理后台") {
                tabWidget_->removeTab(i);
                qDebug() << "[权限设置] ⚠️ 移除了标签页:" << tabText;
                i--; // 调整索引，因为移除了一个标签页
            }
        }
        
    } else {
        qDebug() << "[权限设置] 配置HR管理员权限";
        // HR管理员可以使用所有功能
        qDebug() << "[权限设置] ✅ HR管理员拥有所有权限";
    }
    
    qDebug() << "[权限设置] 权限设置完成";
}

void MainWindow::showWelcomeMessage()
{
    QString welcomeMsg;
    if (currentUser_.isHRAdmin()) {
        welcomeMsg = QString("欢迎您，%1！您可以使用智能问答功能，也可以管理系统数据。")
                     .arg(currentUser_.getName());
    } else {
        welcomeMsg = QString("欢迎您，%1！您可以使用智能问答功能查询HR制度信息。")
                     .arg(currentUser_.getName());
    }
    
    statusBar_->showMessage(welcomeMsg, 5000);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (systemTrayIcon_ && systemTrayIcon_->isVisible()) {
        hide();
        systemTrayIcon_->showMessage("HR智能问答系统", 
                                    "程序已最小化到系统托盘", 
                                    QSystemTrayIcon::Information, 2000);
        event->ignore();
    } else {
        // 正常退出时清理崩溃日志文件
        QFile crashLog(crashLogFile_);
        if (crashLog.exists()) {
            crashLog.remove();
            qDebug() << "[正常退出] 已清理崩溃日志文件";
        }
        
        // 记录正常退出标记
        QSettings settings;
        settings.setValue("app/normalExit", true);
        settings.setValue("app/lastExitTime", QDateTime::currentDateTime());
        
        event->accept();
    }
}

void MainWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    onUserActivity();
}

void MainWindow::onTabChanged(int index)
{
    QString tabName = tabWidget_->tabText(index);
    statusBar_->showMessage(QString("切换到：%1").arg(tabName), 2000);
    onUserActivity();
}

void MainWindow::onLogoutClicked()
{
    int ret = QMessageBox::question(this, "确认注销", 
                                   "确定要注销当前用户吗？",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        logUserActivity();
        this->hide(); // 先隐藏窗口
        emit logoutRequested();
    }
}

void MainWindow::onSettingsClicked()
{
    // 创建设置对话框
    QDialog* settingsDialog = new QDialog(this);
    settingsDialog->setWindowTitle("系统设置");
    settingsDialog->setModal(true);
    settingsDialog->resize(500, 400);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(settingsDialog);
    
    // 设置标题
    QLabel* titleLabel = new QLabel("系统设置");
    titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 10px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // 创建选项卡
    QTabWidget* tabWidget = new QTabWidget();
    
    // 基本设置选项卡
    QWidget* basicTab = new QWidget();
    QVBoxLayout* basicLayout = new QVBoxLayout(basicTab);
    
    // 界面设置
    QGroupBox* uiGroup = new QGroupBox("界面设置");
    QVBoxLayout* uiLayout = new QVBoxLayout(uiGroup);
    
    QCheckBox* autoSaveCheck = new QCheckBox("自动保存设置");
    autoSaveCheck->setChecked(true);
    uiLayout->addWidget(autoSaveCheck);
    
    QCheckBox* showToolbarCheck = new QCheckBox("显示工具栏");
    showToolbarCheck->setChecked(true);
    uiLayout->addWidget(showToolbarCheck);
    
    QCheckBox* showStatusbarCheck = new QCheckBox("显示状态栏");
    showStatusbarCheck->setChecked(true);
    uiLayout->addWidget(showStatusbarCheck);
    
    basicLayout->addWidget(uiGroup);
    
    // 问答设置
    QGroupBox* qaGroup = new QGroupBox("问答设置");
    QVBoxLayout* qaLayout = new QVBoxLayout(qaGroup);
    
    QCheckBox* autoCompleteCheck = new QCheckBox("启用自动补全");
    autoCompleteCheck->setChecked(true);
    qaLayout->addWidget(autoCompleteCheck);
    
    QCheckBox* showConfidenceCheck = new QCheckBox("显示置信度");
    showConfidenceCheck->setChecked(true);
    qaLayout->addWidget(showConfidenceCheck);
    
    QCheckBox* saveHistoryCheck = new QCheckBox("保存问答历史");
    saveHistoryCheck->setChecked(true);
    qaLayout->addWidget(saveHistoryCheck);
    
    basicLayout->addWidget(qaGroup);
    basicLayout->addStretch();
    
    // 高级设置选项卡
    QWidget* advancedTab = new QWidget();
    QVBoxLayout* advancedLayout = new QVBoxLayout(advancedTab);
    
    // 数据库设置
    QGroupBox* dbGroup = new QGroupBox("数据库设置");
    QVBoxLayout* dbLayout = new QVBoxLayout(dbGroup);
    
    QLabel* dbPathLabel = new QLabel("数据库路径:");
    QLineEdit* dbPathEdit = new QLineEdit();
    dbPathEdit->setText("hrsystem.db");
    dbPathEdit->setReadOnly(true);
    
    QHBoxLayout* dbPathLayout = new QHBoxLayout();
    dbPathLayout->addWidget(dbPathLabel);
    dbPathLayout->addWidget(dbPathEdit);
    dbLayout->addLayout(dbPathLayout);
    
    QPushButton* backupBtn = new QPushButton("备份数据库");
    dbLayout->addWidget(backupBtn);
    
    advancedLayout->addWidget(dbGroup);
    
    // 系统信息
    QGroupBox* infoGroup = new QGroupBox("系统信息");
    QVBoxLayout* infoLayout = new QVBoxLayout(infoGroup);
    
    QLabel* versionLabel = new QLabel("版本: 1.0.0");
    QLabel* buildLabel = new QLabel("构建日期: 2024-12-01");
    QLabel* qtLabel = new QLabel("Qt版本: 6.x");
    
    infoLayout->addWidget(versionLabel);
    infoLayout->addWidget(buildLabel);
    infoLayout->addWidget(qtLabel);
    
    advancedLayout->addWidget(infoGroup);
    advancedLayout->addStretch();
    
    // 添加选项卡
    tabWidget->addTab(basicTab, "基本设置");
    tabWidget->addTab(advancedTab, "高级设置");
    
    mainLayout->addWidget(tabWidget);
    
    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* okButton = new QPushButton("确定");
    okButton->setStyleSheet(
        "QPushButton { background-color: #3498db; color: white; border: none; padding: 8px 16px; "
        "border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #2980b9; }"
    );
    
    QPushButton* cancelButton = new QPushButton("取消");
    cancelButton->setStyleSheet(
        "QPushButton { background-color: #95a5a6; color: white; border: none; padding: 8px 16px; "
        "border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #7f8c8d; }"
    );
    
    QPushButton* applyButton = new QPushButton("应用");
    applyButton->setStyleSheet(
        "QPushButton { background-color: #27ae60; color: white; border: none; padding: 8px 16px; "
        "border-radius: 4px; font-size: 14px; }"
        "QPushButton:hover { background-color: #229954; }"
    );
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(applyButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(okButton, &QPushButton::clicked, settingsDialog, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, settingsDialog, &QDialog::reject);
    connect(applyButton, &QPushButton::clicked, [=]() {
        QMessageBox::information(settingsDialog, "应用设置", "设置已应用！");
    });
    
    settingsDialog->exec();
    settingsDialog->deleteLater();
}

void MainWindow::onHelpClicked()
{
    QString helpText = 
        "HR智能问答系统使用帮助\n\n"
        "1. 智能问答：在问答界面输入您的问题，系统会自动识别问题类型并提供答案\n"
        "2. 快速问题：点击左侧的常见问题可以快速获取答案\n"
        "3. 历史记录：您可以查看之前的问答历史\n"
        "4. 文档关联：答案会关联相关的制度文档，点击可查看详细内容\n\n";
    
    if (currentUser_.isHRAdmin()) {
        helpText += 
            "管理员功能：\n"
            "1. 用户管理：查看、添加、编辑用户信息\n"
            "2. 文档管理：上传、编辑HR制度文档\n"
            "3. 问答管理：管理系统问答库\n"
            "4. 数据统计：查看使用统计和图表\n";
    }
    
    QMessageBox::information(this, "使用帮助", helpText);
}

void MainWindow::onAboutClicked()
{
    QString aboutText = 
        "HR制度智能问答系统 v1.0.0\n\n"
        "基于Qt和C++开发的企业HR制度智能问答系统\n"
        "提供智能问答、文档管理、用户管理等功能\n\n"
        "开发团队：HR System Team\n"
        "技术支持：support@company.com\n\n"
        "Copyright © 2024 Company HR System";
    
    QMessageBox::about(this, "关于", aboutText);
}

void MainWindow::onSystemTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
    case QSystemTrayIcon::DoubleClick:
        onShowMainWindow();
        break;
    default:
        break;
    }
}

void MainWindow::onShowMainWindow()
{
    show();
    raise();
    activateWindow();
}

void MainWindow::onQuitApplication()
{
    int ret = QMessageBox::question(this, "确认退出", 
                                   "确定要退出程序吗？",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        logUserActivity();
        
        // 正常退出时清理崩溃日志文件
        QFile crashLog(crashLogFile_);
        if (crashLog.exists()) {
            crashLog.remove();
            qDebug() << "[正常退出] 已清理崩溃日志文件";
        }
        
        // 记录正常退出标记
        QSettings settings;
        settings.setValue("app/normalExit", true);
        settings.setValue("app/lastExitTime", QDateTime::currentDateTime());
        
        QApplication::quit();
    }
}

void MainWindow::updateStatusBar()
{
    QDateTime now = QDateTime::currentDateTime();
    QString timeStr = now.toString("yyyy-MM-dd hh:mm:ss");
    
    // 更新状态栏右侧的时间显示
    QLabel* timeLabel = qobject_cast<QLabel*>(statusBar_->children().last());
    if (timeLabel) {
        timeLabel->setText(timeStr);
    }
}

void MainWindow::onUserActivity()
{
    lastActivityTime_ = QDateTime::currentDateTime();
}

void MainWindow::logUserActivity()
{
    // 记录用户活动日志
    qDebug() << "用户" << currentUser_.getEmployeeId() << "在" << QDateTime::currentDateTime() << "执行了操作";
}

void MainWindow::checkUserSession()
{
    // 检查用户会话，如果长时间无活动可以提示或自动注销
    QDateTime now = QDateTime::currentDateTime();
    qint64 inactiveSeconds = lastActivityTime_.secsTo(now);
    
    // 如果超过30分钟无活动，显示提示
    if (inactiveSeconds > 1800) {
        int ret = QMessageBox::question(this, "会话超时", 
                                       "您已经30分钟未操作，是否继续使用系统？",
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::Yes);
        
        if (ret == QMessageBox::Yes) {
            onUserActivity(); // 重置活动时间
        } else {
            onLogoutClicked();
        }
    }
}

void MainWindow::createGradientBackground()
{
    setStyleSheet(
        "QMainWindow {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #ECE9E6, stop:1 #FFFFFF);"
        "}"
        
        "QMenuBar {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f8f9fa);"
        "    border-bottom: 1px solid #e1e8ed;"
        "    padding: 8px;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "}"
        
        "QMenuBar::item {"
        "    padding: 8px 15px;"
        "    border-radius: 6px;"
        "    margin-right: 3px;"
        "    color: #2c3e50;"
        "    font-weight: bold;"
        "}"
        
        "QMenuBar::item:selected {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4facfe, stop:1 #00f2fe);"
        "    color: white;"
        "    border: 1px solid #2196f3;"
        "}"
        
        "QMenuBar::item:hover {"
        "    background: rgba(79, 172, 254, 0.15);"
        "    color: #1976d2;"
        "}"
        
        "QMenu {"
        "    background: white;"
        "    border: 1px solid #e1e8ed;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "}"
        
        "QMenu::item {"
        "    padding: 8px 25px 8px 20px;"
        "    border-radius: 4px;"
        "    margin: 2px 5px;"
        "}"
        
        "QMenu::item:selected {"
        "    background: rgba(79, 172, 254, 0.2);"
        "}"
        
        "QToolBar {"
        "    background: white;"
        "    border-bottom: 1px solid #e1e8ed;"
        "    spacing: 10px;"
        "    padding: 5px;"
        "}"
        
        "QToolBar QToolButton {"
        "    background: transparent;"
        "    border-radius: 5px;"
        "    padding: 5px;"
        "}"
        
        "QToolBar QToolButton:hover {"
        "    background: rgba(79, 172, 254, 0.1);"
        "}"
        
        "QToolBar QToolButton:pressed {"
        "    background: rgba(79, 172, 254, 0.2);"
        "}"
        
        "QStatusBar {"
        "    background: white;"
        "    border-top: 1px solid #e1e8ed;"
        "    color: #2c3e50;"
        "}"
    );
}

void MainWindow::setupWindowIcon()
{
    // 设置窗口图标 (暂时注释掉，图标文件缺失)
    // setWindowIcon(QIcon(":/icons/app.ico"));
}

void MainWindow::applyWindowStyle()
{
    // 应用窗口样式
    setStyleSheet(styleSheet() + 
        "QTabWidget::pane {"
        "    border: 1px solid #e1e8ed;"
        "    border-radius: 8px;"
        "    background: white;"
        "    margin-top: -1px;"
        "}"
        "QTabBar::tab {"
        "    padding: 12px 20px;"
        "    margin-right: 2px;"
        "    background: #f8f9fa;"
        "    border: 1px solid #e1e8ed;"
        "    border-bottom: none;"
        "    border-top-left-radius: 8px;"
        "    border-top-right-radius: 8px;"
        "    font-weight: bold;"
        "    color: #2c3e50;"
        "}"
        "QTabBar::tab:selected {"
        "    background: white;"
        "    border-bottom: 2px solid #3498db;"
        "    color: #3498db;"
        "}"
        "QTabBar::tab:hover {"
        "    background: #e3f2fd;"
        "}"
    );
}

// 崩溃保护功能实现
void MainWindow::setupCrashProtection()
{
    qDebug() << "[崩溃保护] 初始化崩溃保护系统";
    
    // 设置崩溃日志文件
    crashLogFile_ = QApplication::applicationDirPath() + "/crash_log.txt";
    
    // 崩溃检测定时器 - 每5秒检查一次组件健康状态
    crashProtectionTimer_ = new QTimer(this);
    connect(crashProtectionTimer_, &QTimer::timeout, this, &MainWindow::onCrashProtectionCheck);
    crashProtectionTimer_->start(15000);  // 增加检查间隔到15秒，减少频繁检查
    
    // 自动保存定时器 - 每30秒自动保存用户状态
    autoSaveTimer_ = new QTimer(this);
    connect(autoSaveTimer_, &QTimer::timeout, this, &MainWindow::onAutoSave);
    autoSaveTimer_->start(30000);
    
    // 检查是否从崩溃中恢复
    restoreFromCrash();
    
    qDebug() << "[崩溃保护] 崩溃保护系统已启动";
}

void MainWindow::onCrashProtectionCheck()
{
    try {
        bool allHealthy = true;
        
        // 检查QA组件健康状态
        if (!isComponentHealthy(qaWidget_)) {
            qWarning() << "[崩溃保护] QA组件异常";
            handleComponentCrash("QA组件");
            allHealthy = false;
        }
        
        // 检查AI助手组件健康状态
        if (!isComponentHealthy(aiAssistantWidget_)) {
            qWarning() << "[崩溃保护] AI助手组件异常";
            handleComponentCrash("AI助手组件");
            allHealthy = false;
        }
        
        // 检查管理面板组件健康状态
        if (currentUser_.isHRAdmin() && !isComponentHealthy(adminPanel_)) {
            qWarning() << "[崩溃保护] 管理面板组件异常";
            handleComponentCrash("管理面板组件");
            allHealthy = false;
        }
        
        // 检查图表组件健康状态
        if (!isComponentHealthy(chartWidget_)) {
            qWarning() << "[崩溃保护] 图表组件异常";
            handleComponentCrash("图表组件");
            allHealthy = false;
        }
        
        if (!allHealthy) {
            crashCounter_++;
            lastCrashTime_ = QDateTime::currentDateTime();
            
            // 如果崩溃频率过高，启动紧急恢复
            if (crashCounter_ > 5) {
                qCritical() << "[崩溃保护] 检测到频繁崩溃，启动紧急恢复";
                onEmergencyRestore();
            }
        } else {
            // 组件都正常，重置崩溃计数
            if (crashCounter_ > 0) {
                crashCounter_ = qMax(0, crashCounter_ - 1);
            }
        }
        
    } catch (const std::exception& e) {
        qCritical() << "[崩溃保护] 崩溃检查过程中发生异常:" << e.what();
        logCrashEvent(QString("崩溃检查异常: %1").arg(e.what()));
    } catch (...) {
        qCritical() << "[崩溃保护] 崩溃检查过程中发生未知异常";
        logCrashEvent("崩溃检查未知异常");
    }
}

void MainWindow::onAutoSave()
{
    try {
        // 保存用户活动状态
        lastActivityTime_ = QDateTime::currentDateTime();
        
        // 保存应用状态到数据库
        if (database_) {
            // 这里可以添加自动保存用户设置、对话历史等
            qDebug() << "[自动保存] 用户状态已保存";
        }
        
    } catch (const std::exception& e) {
        qWarning() << "[自动保存] 自动保存时发生异常:" << e.what();
    } catch (...) {
        qWarning() << "[自动保存] 自动保存时发生未知异常";
    }
}

void MainWindow::handleComponentCrash(const QString& componentName)
{
    qWarning() << "[崩溃保护] 处理组件崩溃:" << componentName;
    
    logCrashEvent(QString("组件崩溃: %1").arg(componentName));
    
    // 尝试重新创建失败的组件
    recreateFailedComponents();
    
    // 只在真正需要时显示通知，避免重复通知
    static QDateTime lastNotificationTime;
    QDateTime now = QDateTime::currentDateTime();
    
    // 如果距离上次通知不到30秒，跳过通知
    if (lastNotificationTime.isValid() && lastNotificationTime.secsTo(now) < 30) {
        qDebug() << "[崩溃保护] 跳过重复通知，距离上次通知不到30秒";
        return;
    }
    
    // 只有在组件确实被重新创建时才显示通知
    bool hasRecreatedComponents = false;
    if (!isComponentHealthy(qaWidget_) || !isComponentHealthy(aiAssistantWidget_) ||
        (currentUser_.isHRAdmin() && (!isComponentHealthy(adminPanel_) || !isComponentHealthy(chartWidget_)))) {
        hasRecreatedComponents = true;
    }
    
    if (hasRecreatedComponents && systemTrayIcon_ && systemTrayIcon_->isVisible()) {
        systemTrayIcon_->showMessage(
            "系统恢复",
            "部分组件已自动恢复",
            QSystemTrayIcon::Information,  // 改为Information级别，减少警告感
            2000  // 缩短显示时间
        );
        lastNotificationTime = now;
        qDebug() << "[崩溃保护] 显示恢复通知";
    }
}

void MainWindow::recreateFailedComponents()
{
    try {
        // 添加恢复计数器，避免过度频繁的恢复
        static int recoveryCount = 0;
        static QDateTime lastRecoveryTime;
        QDateTime now = QDateTime::currentDateTime();
        
        // 如果5分钟内恢复次数超过3次，暂停恢复
        if (lastRecoveryTime.isValid() && lastRecoveryTime.secsTo(now) < 300) {
            recoveryCount++;
            if (recoveryCount > 3) {
                qWarning() << "[崩溃保护] 恢复过于频繁，暂停自动恢复";
                return;
            }
        } else {
            recoveryCount = 0; // 重置计数器
        }
        lastRecoveryTime = now;
        
        qDebug() << "[崩溃保护] 开始重新创建失败的组件，恢复次数:" << recoveryCount;
        
        // 安全地重新创建QA组件
        if (!isComponentHealthy(qaWidget_)) {
            qDebug() << "[崩溃保护] 重新创建QA组件";
            if (qaWidget_) {
                tabWidget_->removeTab(0);
                qaWidget_->deleteLater();
            }
            qaWidget_ = new ModernQAWidget(qaEngine_, database_, currentUser_, this);
            tabWidget_->insertTab(0, qaWidget_, "智能问答");
        }
        
        // 安全地重新创建AI助手组件
        if (!isComponentHealthy(aiAssistantWidget_)) {
            qDebug() << "[崩溃保护] 重新创建AI助手组件";
            if (aiAssistantWidget_) {
                // 找到AI助手的标签页位置
                for (int i = 0; i < tabWidget_->count(); ++i) {
                    if (tabWidget_->widget(i) == aiAssistantWidget_) {
                        tabWidget_->removeTab(i);
                        break;
                    }
                }
                aiAssistantWidget_->deleteLater();
            }
            aiAssistantWidget_ = new AIAssistantWidget(this);
            tabWidget_->addTab(aiAssistantWidget_, "AI助手");
        }
        
        // 安全地重新创建管理面板组件 - 强化权限检查
        if (currentUser_.isHRAdmin() && !isComponentHealthy(adminPanel_)) {
            qDebug() << "[崩溃保护] 重新创建管理面板组件（仅限HR管理员）";
            if (adminPanel_) {
                // 找到管理面板的标签页位置
                for (int i = 0; i < tabWidget_->count(); ++i) {
                    if (tabWidget_->widget(i) == adminPanel_) {
                        tabWidget_->removeTab(i);
                        break;
                    }
                }
                adminPanel_->deleteLater();
            }
            adminPanel_ = new AdminPanel(database_, currentUser_, this);
            tabWidget_->addTab(adminPanel_, "管理后台");
            qDebug() << "[权限控制] ✅ HR管理员面板组件已重新创建";
        } else if (!currentUser_.isHRAdmin() && adminPanel_) {
            qDebug() << "[权限控制] ⚠️ 检测到普通员工拥有管理面板组件，正在移除";
            // 如果普通员工意外拥有管理面板组件，立即移除
            for (int i = 0; i < tabWidget_->count(); ++i) {
                if (tabWidget_->widget(i) == adminPanel_) {
                    tabWidget_->removeTab(i);
                    break;
                }
            }
            adminPanel_->deleteLater();
            adminPanel_ = nullptr;
        }
        
        // 安全地重新创建图表组件 - 添加权限检查
        if (currentUser_.isHRAdmin() && !isComponentHealthy(chartWidget_)) {
            qDebug() << "[崩溃保护] 重新创建图表组件（仅限HR管理员）";
            if (chartWidget_) {
                // 找到图表的标签页位置
                for (int i = 0; i < tabWidget_->count(); ++i) {
                    if (tabWidget_->widget(i) == chartWidget_) {
                        tabWidget_->removeTab(i);
                        break;
                    }
                }
                chartWidget_->deleteLater();
            }
            chartWidget_ = new ChartWidget(database_, currentUser_, this);
            tabWidget_->addTab(chartWidget_, "数据统计");
            qDebug() << "[权限控制] ✅ HR管理员图表组件已重新创建";
        } else if (!currentUser_.isHRAdmin() && chartWidget_) {
            qDebug() << "[权限控制] ⚠️ 检测到普通员工拥有图表组件，正在移除";
            // 如果普通员工意外拥有图表组件，立即移除
            for (int i = 0; i < tabWidget_->count(); ++i) {
                if (tabWidget_->widget(i) == chartWidget_) {
                    tabWidget_->removeTab(i);
                    break;
                }
            }
            chartWidget_->deleteLater();
            chartWidget_ = nullptr;
        }
        
        qDebug() << "[崩溃保护] 组件重新创建完成";
        
    } catch (const std::exception& e) {
        qCritical() << "[崩溃保护] 重新创建组件时发生异常:" << e.what();
        onEmergencyRestore();
    } catch (...) {
        qCritical() << "[崩溃保护] 重新创建组件时发生未知异常";
        onEmergencyRestore();
    }
}

bool MainWindow::isComponentHealthy(QWidget* component)
{
    if (!component) {
        return false;
    }
    
    try {
        // 更严格的健康检查：确保组件确实存在且可用
        // 检查父窗口是否有效
        if (!component->parent()) {
            return false;
        }
        
        // 检查组件是否在正确的容器中
        bool foundInTabWidget = false;
        if (tabWidget_) {
            for (int i = 0; i < tabWidget_->count(); ++i) {
                if (tabWidget_->widget(i) == component) {
                    foundInTabWidget = true;
                    break;
                }
            }
        }
        
        // 组件必须要么在标签页中，要么是独立窗口
        return foundInTabWidget || component->isWindow();
    } catch (...) {
        return false;
    }
}

void MainWindow::restoreFromCrash()
{
    // 检查上次是否正常退出
    QSettings settings;
    bool normalExit = settings.value("app/normalExit", false).toBool();
    QDateTime lastExitTime = settings.value("app/lastExitTime").toDateTime();
    
    if (normalExit) {
        // 上次是正常退出，清除标记
        settings.remove("app/normalExit");
        qDebug() << "[崩溃保护] 上次正常退出，无需恢复";
        
        // 如果有崩溃日志文件（可能是旧的），直接删除
        QFile crashLog(crashLogFile_);
        if (crashLog.exists()) {
            crashLog.remove();
            qDebug() << "[崩溃保护] 清理旧的崩溃日志文件";
        }
        return;
    }
    
    // 检查是否有崩溃日志文件
    QFile crashLog(crashLogFile_);
    if (crashLog.exists()) {
        qDebug() << "[崩溃保护] 检测到崩溃日志，应用可能从崩溃中恢复";
        
        if (crashLog.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream stream(&crashLog);
            QString logContent = stream.readAll();
            crashLog.close();
            
            if (!logContent.isEmpty()) {
                    // 检查崩溃日志的时间，避免显示过旧的崩溃提示
    QFileInfo logInfo(crashLog);
    QDateTime logTime = logInfo.lastModified();
    QDateTime now = QDateTime::currentDateTime();

    // 如果崩溃日志超过2小时前的，认为是过时的
    if (logTime.secsTo(now) < 7200) {  // 2小时 = 7200秒
        // 检查是否真的需要恢复（避免误报）
        QSettings settings;
        int crashCount = settings.value("app/crashCount", 0).toInt();
        
        // 彻底禁用恢复弹窗，避免误报影响用户体验
        if (false) {  // 完全禁用，如果真的需要可以设置为 crashCount > 20
            int ret = QMessageBox::question(this, "应用恢复", 
                "检测到应用可能异常退出，是否恢复上次的工作状态？", 
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            
            if (ret == QMessageBox::Yes) {
                qDebug() << "[崩溃保护] 用户选择恢复工作状态";
                // 这里可以添加状态恢复逻辑
            }
            
            // 重置崩溃计数
            settings.setValue("app/crashCount", 0);
        } else {
            // 静默恢复，不显示弹窗，只记录日志，并重置计数避免累积
            qDebug() << "[崩溃保护] 轻微异常退出（计数:" << crashCount << "），已静默恢复";
            settings.setValue("app/crashCount", 0);
            // 删除崩溃日志文件，避免重复触发
            QFile::remove(crashLogFile_);
        }
    } else {
        qDebug() << "[崩溃保护] 崩溃日志过旧（" << logTime.secsTo(now) << "秒前），跳过提示";
    }
            }
        }
        
        // 清空崩溃日志
        crashLog.remove();
    }
}

void MainWindow::logCrashEvent(const QString& details)
{
    try {
        QFile crashLog(crashLogFile_);
        if (crashLog.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
            QTextStream stream(&crashLog);
            stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") 
                   << " [崩溃] " << details << "\n";
            crashLog.close();
        }
    } catch (...) {
        // 忽略日志写入错误，避免无限递归
    }
}

void MainWindow::onCrashDetected()
{
    qCritical() << "[崩溃保护] 检测到系统崩溃";
    logCrashEvent("系统崩溃检测");
    recreateFailedComponents();
}

void MainWindow::onComponentError()
{
    qWarning() << "[崩溃保护] 组件错误";
    recreateFailedComponents();
}

void MainWindow::onEmergencyRestore()
{
    qCritical() << "[崩溃保护] 检测到紧急恢复信号，但已禁用弹窗";
    
    try {
        // 静默处理恢复，不显示弹窗
        logCrashEvent("紧急恢复信号（已静默处理）");
        
        // 重置计数器，避免累积
        crashCounter_ = 0;
        
        // 确保定时器正常运行
        if (crashProtectionTimer_ && !crashProtectionTimer_->isActive()) {
            crashProtectionTimer_->start(120000); // 延长检查间隔到2分钟
        }
        if (autoSaveTimer_ && !autoSaveTimer_->isActive()) {
            autoSaveTimer_->start(30000);
        }
        if (statusTimer_ && !statusTimer_->isActive()) {
            statusTimer_->start(5000);
        }
        if (activityTimer_ && !activityTimer_->isActive()) {
            activityTimer_->start(60000);
        }
        
        qDebug() << "[崩溃保护] 已静默处理紧急恢复，继续正常运行";
        
        // 清理可能存在的崩溃日志
        QFile::remove(crashLogFile_);
        
    } catch (...) {
        // 最后的手段：强制重启
        qApp->quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    }
}
