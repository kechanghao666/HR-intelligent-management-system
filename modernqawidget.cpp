#include "modernqawidget.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QScrollBar>
#include <QTextBlock>
#include <QDateTime>
#include <QKeyEvent>
#include <QShortcut>
#include <QRegularExpression>
#include <QDebug>
#include <QTimer>
#include <QStringConverter>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <algorithm>

ModernQAWidget::ModernQAWidget(QAEngine* qaEngine, Database* database, const User& currentUser, QWidget *parent)
    : QWidget(parent)
    , qaEngine_(qaEngine)
    , database_(database)
    , currentUser_(currentUser)
    , documentProcessor_(new DocumentProcessor(database, this))
    , mainLayout_(nullptr)
    , stackedWidget_(nullptr)
    , chatWidget_(nullptr)
    , chatLayout_(nullptr)
    , titleBar_(nullptr)
    , titleLayout_(nullptr)
    , titleLabel_(nullptr)
    , statusLabel_(nullptr)
    , sidebarButton_(nullptr)
    , settingsButton_(nullptr)
    , messagesScrollArea_(nullptr)
    , messagesContainer_(nullptr)
    , messagesLayout_(nullptr)
    , inputFrame_(nullptr)
    , inputLayout_(nullptr)
    , inputEdit_(nullptr)
    , sendButton_(nullptr)
    , characterCountLabel_(nullptr)
    , sidebarWidget_(nullptr)
    , sidebarLayout_(nullptr)
    , sidebarFrame_(nullptr)
    , welcomeLabel_(nullptr)
    , avatarWidget_(nullptr)
    , avatarChangeButton_(nullptr)
    , categoryCombo_(nullptr)
    , quickQuestionsGroup_(nullptr)
    , quickQuestionsList_(nullptr)
    , historyGroup_(nullptr)
    , historyList_(nullptr)
    , clearHistoryButton_(nullptr)
    , sidebarAnimation_(nullptr)
    , typingTimer_(nullptr)
    , typingBubble_(nullptr)
    , fadeEffect_(nullptr)
    , sidebarVisible_(true)
    , isTyping_(false)
{
    setupUI();
    setupConnections();
    applyModernStyles();
    loadQuickQuestions();
    loadHistory();
    
    // 设置DocumentProcessor到QAEngine，启用文档处理功能
    if (qaEngine_ && documentProcessor_) {
        qaEngine_->setDocumentProcessor(documentProcessor_);
    }
    
    // 添加欢迎消息
    addMessage(MessageType::SystemMessage, "欢迎使用HR智能问答助手！我是您的专属HR助理，可以为您解答各种人力资源相关问题。");
    
    // 测试：添加示例对话以验证头像显示（已移除，保持界面清爽）
    // addMessage(MessageType::UserMessage, "你好，我想了解一下公司的考勤制度");
    // addMessage(MessageType::BotMessage, "您好！很高兴为您服务。关于公司考勤制度，我来为您详细介绍...");
    
    updateOnlineStatus();
    
    // 默认显示侧边栏
    sidebarButton_->setText("隐藏");
    sidebarButton_->setToolTip("隐藏侧边栏");
}

ModernQAWidget::~ModernQAWidget()
{
    if (typingTimer_) {
        typingTimer_->stop();
    }
}

void ModernQAWidget::setupUI()
{
    // 主布局
    mainLayout_ = new QHBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    
    setupMainChat();
    setupSidebar();
    
    // 默认显示侧边栏
    sidebarWidget_->setVisible(true);
    
    mainLayout_->addWidget(chatWidget_, 1);
    mainLayout_->addWidget(sidebarWidget_, 0);
}

void ModernQAWidget::setupMainChat()
{
    chatWidget_ = new QWidget();
    chatLayout_ = new QVBoxLayout(chatWidget_);
    chatLayout_->setContentsMargins(0, 0, 0, 0);
    chatLayout_->setSpacing(0);
    
    setupTitleBar();
    
    // 消息区域
    messagesScrollArea_ = new QScrollArea();
    messagesScrollArea_->setWidgetResizable(true);
    messagesScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messagesScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    messagesScrollArea_->setFrameShape(QFrame::NoFrame);
    
    messagesContainer_ = new QWidget();
    messagesLayout_ = new QVBoxLayout(messagesContainer_);
    messagesLayout_->setContentsMargins(20, 20, 20, 20);
    messagesLayout_->setSpacing(15);
    messagesLayout_->addStretch();
    
    messagesScrollArea_->setWidget(messagesContainer_);
    chatLayout_->addWidget(messagesScrollArea_, 1);
    
    setupInputArea();
}

void ModernQAWidget::setupTitleBar()
{
    titleBar_ = new QFrame();
    titleBar_->setObjectName("titleBar");
    titleBar_->setFixedHeight(60);
    
    titleLayout_ = new QHBoxLayout(titleBar_);
    titleLayout_->setContentsMargins(20, 10, 20, 10);
    titleLayout_->setSpacing(8);  // 减少间距，让标题紧跟按钮
    
    // 侧边栏按钮 - 自适应中文文字
    sidebarButton_ = new QPushButton("菜单");
    sidebarButton_->setObjectName("sidebarButton");
    sidebarButton_->setMinimumSize(60, 40);
    sidebarButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sidebarButton_->setToolTip("显示/隐藏侧边栏");
    
    // 标题
    titleLabel_ = new QLabel("HR智能助理");
    titleLabel_->setObjectName("titleLabel");
    titleLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #1a1a1a;");
    
    // 状态标签
    statusLabel_ = new QLabel("在线");
    statusLabel_->setObjectName("statusLabel");
    statusLabel_->setStyleSheet("color: #34C759; font-size: 12px; font-weight: bold;");
    
    // 设置按钮 - 自适应中文文字
    settingsButton_ = new QPushButton("设置");
    settingsButton_->setObjectName("settingsButton");
    settingsButton_->setMinimumSize(60, 40);
    settingsButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    settingsButton_->setToolTip("设置");
    
    titleLayout_->addWidget(sidebarButton_);
    titleLayout_->addWidget(titleLabel_);
    titleLayout_->addWidget(statusLabel_);
    titleLayout_->addStretch();
    titleLayout_->addWidget(settingsButton_);
    
    chatLayout_->addWidget(titleBar_);
}

void ModernQAWidget::setupInputArea()
{
    inputFrame_ = new QFrame();
    inputFrame_->setObjectName("inputFrame");
    inputFrame_->setMinimumHeight(80);
    inputFrame_->setMaximumHeight(150);
    
    inputLayout_ = new QHBoxLayout(inputFrame_);
    inputLayout_->setContentsMargins(20, 15, 20, 15);
    inputLayout_->setSpacing(10);
    
    // 附件按钮 - 自适应中文文字
    // 附件功能已移除
    
    // 输入框
    inputEdit_ = new QTextEdit();
    inputEdit_->setObjectName("inputEdit");
    inputEdit_->setPlaceholderText("输入您的问题...");
    inputEdit_->setMaximumHeight(100);
    inputEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    inputEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 发送按钮 - 自适应中文文字
    sendButton_ = new QPushButton("发送");
    sendButton_->setObjectName("sendButton");
    sendButton_->setMinimumSize(80, 40);
    sendButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sendButton_->setEnabled(false);
    
    // 字符计数
    characterCountLabel_ = new QLabel("0/1000");
    characterCountLabel_->setObjectName("characterCount");
    characterCountLabel_->setAlignment(Qt::AlignRight);
    
    QVBoxLayout* inputRightLayout = new QVBoxLayout();
    inputRightLayout->addWidget(sendButton_);
    inputRightLayout->addWidget(characterCountLabel_);
    inputRightLayout->addStretch();
    
    // 附件按钮已移除
    inputLayout_->addWidget(inputEdit_, 1);
    inputLayout_->addLayout(inputRightLayout);
    
    chatLayout_->addWidget(inputFrame_);
}

void ModernQAWidget::setupSidebar()
{
    sidebarWidget_ = new QWidget();
    sidebarWidget_->setFixedWidth(320);
    sidebarWidget_->setObjectName("sidebarWidget");
    
    sidebarLayout_ = new QVBoxLayout(sidebarWidget_);
    sidebarLayout_->setContentsMargins(0, 0, 0, 0);
    sidebarLayout_->setSpacing(0);
    
    sidebarFrame_ = new QFrame();
    sidebarFrame_->setObjectName("sidebarFrame");
    
    QVBoxLayout* frameLayout = new QVBoxLayout(sidebarFrame_);
    frameLayout->setContentsMargins(20, 20, 20, 20);
    frameLayout->setSpacing(15);
    
    // 用户头像和欢迎信息区域
    QWidget* userInfoWidget = new QWidget();
    QVBoxLayout* userInfoLayout = new QVBoxLayout(userInfoWidget);
    userInfoLayout->setSpacing(10);
    userInfoLayout->setContentsMargins(0, 0, 0, 0);
    
    // 头像组件
    avatarWidget_ = new AvatarWidget(currentUser_, userInfoWidget);
    avatarWidget_->setSize(80);  // 较大的头像尺寸
    
    // 头像更换按钮 - 自适应中文文字
    avatarChangeButton_ = new QPushButton("更换头像");
    avatarChangeButton_->setObjectName("avatarChangeButton");
    avatarChangeButton_->setMinimumSize(100, 30);
    avatarChangeButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    // 欢迎标签
    welcomeLabel_ = new QLabel(QString("你好，%1").arg(currentUser_.getName()));
    welcomeLabel_->setObjectName("welcomeLabel");
    welcomeLabel_->setAlignment(Qt::AlignCenter);
    
    // 组装用户信息区域
    userInfoLayout->addWidget(avatarWidget_, 0, Qt::AlignCenter);
    userInfoLayout->addWidget(avatarChangeButton_, 0, Qt::AlignCenter);
    userInfoLayout->addWidget(welcomeLabel_, 0, Qt::AlignCenter);
    
    frameLayout->addWidget(userInfoWidget);
    
    // 分割线
    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    line->setStyleSheet("color: #E5E5EA;");
    frameLayout->addWidget(line);
    
    // 类别选择
    QLabel* categoryLabel = new QLabel("问题分类");
    categoryLabel->setStyleSheet("font-weight: bold; color: #1a1a1a; font-size: 14px;");
    frameLayout->addWidget(categoryLabel);
    
    categoryCombo_ = new QComboBox();
    categoryCombo_->setObjectName("categoryCombo");
    categoryCombo_->addItem("全部类别", "");
    categoryCombo_->addItem("考勤管理", "考勤");
    categoryCombo_->addItem("薪酬管理", "薪酬");
    categoryCombo_->addItem("绩效管理", "绩效");
    categoryCombo_->addItem("入职离职", "入职离职");
    categoryCombo_->addItem("员工福利", "员工福利");
    frameLayout->addWidget(categoryCombo_);
    
    frameLayout->addSpacing(10);
    
    // 常见问题 - 修复超出方框问题
    quickQuestionsGroup_ = new QGroupBox("常见问题");
    quickQuestionsGroup_->setObjectName("quickQuestionsGroup");
    quickQuestionsGroup_->setMinimumHeight(180);
    quickQuestionsGroup_->setMaximumHeight(220);
    QVBoxLayout* quickLayout = new QVBoxLayout(quickQuestionsGroup_);
    quickLayout->setContentsMargins(6, 12, 6, 6);
    quickLayout->setSpacing(4);
    
    quickQuestionsList_ = new QListWidget();
    quickQuestionsList_->setObjectName("quickQuestionsList");
    quickQuestionsList_->setMinimumHeight(130);
    quickQuestionsList_->setMaximumHeight(170);
    // 改善字体和行间距，防止文字超出
    quickQuestionsList_->setFont(QFont("Microsoft YaHei", 8));
    quickQuestionsList_->setWordWrap(true);
    quickQuestionsList_->setTextElideMode(Qt::ElideRight);
    quickQuestionsList_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    quickQuestionsList_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    quickLayout->addWidget(quickQuestionsList_);
    
    frameLayout->addWidget(quickQuestionsGroup_);
    
    // 移除上方带按钮的搜索入口，仅保留下方历史记录内搜索
    
    // 历史记录 - 作为搜索结果列表（保留一个模块）
    historyGroup_ = new QGroupBox("历史记录");
    historyGroup_->setObjectName("historyGroup");
    historyGroup_->setMinimumHeight(280);
    historyGroup_->setMaximumHeight(QWIDGETSIZE_MAX);
    QVBoxLayout* historyLayout = new QVBoxLayout(historyGroup_);
    historyLayout->setContentsMargins(6, 12, 6, 6);
    historyLayout->setSpacing(12);
    
    // 筛选器
    historyFilterCombo_ = new QComboBox();
    historyFilterCombo_->setObjectName("historyFilterCombo");
    historyFilterCombo_->addItem("全部记录", "");
    historyFilterCombo_->addItem("考勤管理", "考勤");
    historyFilterCombo_->addItem("薪酬管理", "薪酬");
    historyFilterCombo_->addItem("绩效管理", "绩效");
    historyFilterCombo_->addItem("入职离职", "入职离职");
    historyFilterCombo_->addItem("员工福利", "员工福利");
    historyFilterCombo_->setMinimumHeight(32);
    historyFilterCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    historyLayout->addWidget(historyFilterCombo_);
    
    // 搜索框 - 重新调整确保文字完整显示
    advancedSearchEdit_ = new QLineEdit();
    advancedSearchEdit_->setObjectName("advancedSearchEdit");
    advancedSearchEdit_->setPlaceholderText("搜索记录");  // 精简文字避免截断
    advancedSearchEdit_->setMinimumHeight(42);
    advancedSearchEdit_->setMaximumHeight(45);
    advancedSearchEdit_->setFont(QFont("Microsoft YaHei", 9));
    advancedSearchEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // 设置内边距确保文字有足够空间
    advancedSearchEdit_->setStyleSheet(
        "QLineEdit { "
        "  padding: 8px 12px; "
        "  border: 1px solid #d1d5db; "
        "  border-radius: 6px; "
        "  background: white; "
        "  font-size: 13px; "
        "} "
        "QLineEdit:focus { "
        "  border: 2px solid #3b82f6; "
        "  outline: none; "
        "}"
    );
    historyLayout->addWidget(advancedSearchEdit_);
    
    // 历史记录列表 - 改善显示
    historyList_ = new QListWidget();
    historyList_->setObjectName("historyList");
    historyList_->setMinimumHeight(120);
    historyList_->setMaximumHeight(140);
    historyList_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    // 改善字体和显示效果
    historyList_->setFont(QFont("Microsoft YaHei", 9));
    historyList_->setWordWrap(true);
    historyList_->setTextElideMode(Qt::ElideNone);
    historyList_->setAlternatingRowColors(true);
    historyLayout->addWidget(historyList_);
    
    frameLayout->addWidget(historyGroup_);
    
    // 添加间距，确保按钮不被遮挡
    frameLayout->addSpacing(12);
    
    // 将清空历史按钮移到历史记录方框外部下方
    clearHistoryButton_ = new QPushButton("清空历史");
    clearHistoryButton_->setObjectName("clearHistoryButton");
    clearHistoryButton_->setMinimumHeight(40);
    clearHistoryButton_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frameLayout->addWidget(clearHistoryButton_);
    
    frameLayout->addStretch();
    
    sidebarLayout_->addWidget(sidebarFrame_);
}

void ModernQAWidget::setupConnections()
{
    // 发送消息
    connect(sendButton_, &QPushButton::clicked, this, &ModernQAWidget::onSendClicked);
    connect(sidebarButton_, &QPushButton::clicked, this, &ModernQAWidget::onSidebarToggled);
    
    // 附件和设置按钮
    // 附件功能已移除
    connect(settingsButton_, &QPushButton::clicked, this, &ModernQAWidget::onSettingsClicked);
    
    // 输入框事件
    connect(inputEdit_, &QTextEdit::textChanged, this, &ModernQAWidget::onInputTextChanged);
    
    // 快速问题和历史记录
    connect(quickQuestionsList_, &QListWidget::itemClicked, this, &ModernQAWidget::onQuickQuestionClicked);
    connect(historyList_, &QListWidget::itemClicked, this, &ModernQAWidget::onHistoryItemClicked);
    connect(clearHistoryButton_, &QPushButton::clicked, this, &ModernQAWidget::clearHistory);
    
    // 简化历史记录功能连接
    connect(historyFilterCombo_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), 
            this, &ModernQAWidget::onFilterHistoryByDate);
    
    // 搜索功能：仅使用历史记录区域的搜索框
    connect(advancedSearchEdit_, &QLineEdit::textChanged, this, &ModernQAWidget::onAdvancedSearchTextChanged);
    
    // 头像相关连接
    connect(avatarWidget_, &AvatarWidget::avatarClicked, [this]() {
        onAvatarChangeClicked();
    });
    connect(avatarChangeButton_, &QPushButton::clicked, [this]() {
        onAvatarChangeClicked();
    });
    
    // AI问答信号连接
    connect(qaEngine_, &QAEngine::aiAnswerReceived, this, &ModernQAWidget::onAIAnswerReceived);

    
    // 回车发送
    QShortcut* sendShortcut = new QShortcut(QKeySequence("Return"), inputEdit_);
    connect(sendShortcut, &QShortcut::activated, this, &ModernQAWidget::onSendClicked);
    
    // Ctrl+Enter换行
    QShortcut* newLineShortcut = new QShortcut(QKeySequence("Ctrl+Return"), inputEdit_);
    connect(newLineShortcut, &QShortcut::activated, [this]() {
        inputEdit_->insertPlainText("\n");
    });
    
    // 不再使用上方搜索按钮
    
    // 打字动画
    typingTimer_ = new QTimer(this);
    typingTimer_->setSingleShot(true);
    connect(typingTimer_, &QTimer::timeout, this, &ModernQAWidget::finishTyping);
}

void ModernQAWidget::applyModernStyles()
{
    setStyleSheet(QString(
        // 主窗口样式
        "ModernQAWidget {"
        "    background: %1;"
        "    font-family: 'SF Pro Display', 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "}"
        
        // 标题栏样式
        "#titleBar {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(255,255,255,0.98), "
        "                stop:0.5 rgba(250,251,252,0.95), "
        "                stop:1 rgba(245,246,247,0.92));"
        "    border-bottom: 1px solid rgba(0,0,0,0.06);"
        "    backdrop-filter: blur(25px);"
        "    box-shadow: 0 1px 3px rgba(0,0,0,0.1), 0 1px 2px rgba(0,0,0,0.06);"
        "}"
        
        // 标题标签样式
        "#titleLabel {"
        "    color: #1F2937;"
        "    font-size: 18px;"
        "    font-weight: bold;"
        "    font-family: 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "    padding: 0px 8px;"
        "}"
        
        "#statusLabel {"
        "    color: #34C759;"
        "    font-size: 12px;"
        "    font-weight: bold;"
        "    padding: 0px 4px;"
        "}"
        
        // 按钮样式
        "%2"
        
        // 输入区域样式
        "%3"
        
        // 侧边栏样式
        "%4"
        
        // 滚动条样式
        "QScrollBar:vertical {"
        "    border: none;"
        "    background: rgba(0,0,0,0.05);"
        "    width: 8px;"
        "    border-radius: 4px;"
        "}"
        "QScrollBar::handle:vertical {"
        "    background: rgba(0,0,0,0.2);"
        "    border-radius: 4px;"
        "    min-height: 20px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "    background: rgba(0,0,0,0.3);"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "    border: none;"
        "    background: none;"
        "}"
    ).arg(
        getGradientBackground(),
        getModernButtonStyle(),
        getInputAreaStyle(),
        getSidebarStyle()
    ));
}

QString ModernQAWidget::getGradientBackground()
{
    return "qlineargradient(x1:0, y1:0, x2:1, y2:1, "
           "stop:0 rgba(243,244,246,1), "
           "stop:0.3 rgba(249,250,251,1), "
           "stop:0.6 rgba(255,255,255,1), "
           "stop:1 rgba(248,250,252,1))";
}

QString ModernQAWidget::getModernButtonStyle()
{
    return 
        "#sidebarButton, #settingsButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(248,250,252,0.95), "
        "                stop:1 rgba(241,245,249,0.9));"
        "    border: 2px solid rgba(59,130,246,0.2);"
        "    border-radius: 12px;"
        "    color: #1F2937;"
        "    font-size: 14px;"
        "    font-weight: 600;"
        "    font-family: 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "    padding: 4px 8px;"
        "    box-shadow: 0 1px 3px rgba(0,0,0,0.1), 0 1px 2px rgba(0,0,0,0.06);"
        "}"
        "#sidebarButton:hover, #settingsButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(59,130,246,0.1), "
        "                stop:1 rgba(37,99,235,0.05));"
        "    border: 2px solid rgba(59,130,246,0.4);"
        "    color: #1E40AF;"
        "    box-shadow: 0 2px 6px rgba(59,130,246,0.2), 0 1px 4px rgba(0,0,0,0.1);"
        "    transform: translateY(-1px);"
        "}"
        "#sidebarButton:pressed, #settingsButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(59,130,246,0.15), "
        "                stop:1 rgba(37,99,235,0.1));"
        "    border: 2px solid rgba(59,130,246,0.5);"
        "    color: #1D4ED8;"
        "    transform: translateY(0px);"
        "    box-shadow: 0 1px 2px rgba(0,0,0,0.15);"
        "}"
        
        "#sendButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 #3b82f6, stop:0.5 #2563eb, stop:1 #1d4ed8);"
        "    border: none;"
        "    border-radius: 20px;"
        "    color: white;"
        "    font-weight: 600;"
        "    font-size: 15px;"
        "    box-shadow: 0 2px 4px rgba(59,130,246,0.4), 0 1px 6px rgba(59,130,246,0.2);"
        "}"
        "#sendButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 #2563eb, stop:0.5 #1d4ed8, stop:1 #1e40af);"
        "    box-shadow: 0 4px 12px rgba(59,130,246,0.4), 0 2px 8px rgba(59,130,246,0.3);"
        "    transform: translateY(-1px);"
        "}"
        "#sendButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 #1d4ed8, stop:0.5 #1e40af, stop:1 #1e3a8a);"
        "    transform: translateY(0px);"
        "    box-shadow: 0 1px 3px rgba(59,130,246,0.4);"
        "}"
        "#sendButton:disabled {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(156,163,175,0.5), stop:1 rgba(107,114,128,0.5));"
        "    color: rgba(255,255,255,0.6);"
        "    transform: none;"
        "    box-shadow: none;"
        "}";
}

QString ModernQAWidget::getInputAreaStyle()
{
    return 
        "#inputFrame {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(255,255,255,0.98), "
        "                stop:0.5 rgba(250,251,252,0.95), "
        "                stop:1 rgba(248,249,250,0.92));"
        "    border-top: 1px solid rgba(0,0,0,0.06);"
        "    backdrop-filter: blur(25px);"
        "    box-shadow: 0 -1px 3px rgba(0,0,0,0.1), 0 -1px 2px rgba(0,0,0,0.06);"
        "}"
        
        "#inputEdit {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(255,255,255,1), "
        "                stop:1 rgba(252,253,254,0.98));"
        "    border: 1px solid rgba(209,213,219,0.6);"
        "    border-radius: 24px;"
        "    padding: 14px 20px;"
        "    font-size: 15px;"
        "    color: #1f2937;"
        "    selection-background-color: #3b82f6;"
        "    selection-color: white;"
        "    box-shadow: 0 1px 3px rgba(0,0,0,0.1), 0 1px 2px rgba(0,0,0,0.06);"
        "}"
        "#inputEdit:focus {"
        "    border: 2px solid #3b82f6;"
        "    background: rgba(255,255,255,1);"
        "    box-shadow: 0 0 0 3px rgba(59,130,246,0.1), 0 1px 3px rgba(0,0,0,0.1);"
        "    outline: none;"
        "}"
        
        "#characterCount {"
        "    color: rgba(0,0,0,0.5);"
        "    font-size: 11px;"
        "    margin-top: 4px;"
        "}";
}

QString ModernQAWidget::getSidebarStyle()
{
    return 
        "#sidebarWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "                stop:0 rgba(255,255,255,0.95), "
        "                stop:1 rgba(248,249,250,0.90));"
        "    border-left: 1px solid rgba(0,0,0,0.08);"
        "    backdrop-filter: blur(20px);"
        "}"
        
        "#welcomeLabel {"
        "    font-size: 16px;"
        "    font-weight: bold;"
        "    color: #1a1a1a;"
        "    padding: 10px;"
        "    background: rgba(0,122,255,0.1);"
        "    border-radius: 12px;"
        "}"
        
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid rgba(0,0,0,0.1);"
        "    border-radius: 12px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "    background: rgba(255,255,255,0.7);"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 4px 8px;"
        "    color: #1a1a1a;"
        "    font-size: 13px;"
        "}"
        
        "#quickQuestionsList, #historyList {"
        "    background: transparent;"
        "    border: none;"
        "    outline: none;"
        "    font-size: 13px;"
        "}"
        "#quickQuestionsList::item, #historyList::item {"
        "    padding: 10px 8px;"
        "    margin: 1px 0px;"
        "    border-radius: 6px;"
        "    background: rgba(255,255,255,0.8);"
        "    border: 1px solid rgba(0,0,0,0.08);"
        "    font-size: 12px;"
        "    line-height: 1.4;"
        "    min-height: 20px;"
        "}"
        "#quickQuestionsList::item:hover, #historyList::item:hover {"
        "    background: rgba(0,122,255,0.1);"
        "    border: 1px solid rgba(0,122,255,0.3);"
        "}"
        "#quickQuestionsList::item:selected, #historyList::item:selected {"
        "    background: rgba(0,122,255,0.2);"
        "    border: 1px solid rgba(0,122,255,0.5);"
        "    color: #1a1a1a;"
        "}"
        
        "#categoryCombo, #searchEdit {"
        "    background: white;"
        "    border: 1px solid rgba(0,0,0,0.1);"
        "    border-radius: 8px;"
        "    padding: 8px 12px;"
        "    font-size: 13px;"
        "    color: #1a1a1a;"
        "}"
        "#categoryCombo:focus, #searchEdit:focus {"
        "    border: 2px solid #007AFF;"
        "}"
        
        "#searchButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);"
        "    border: 1px solid #2980b9;"
        "    border-radius: 8px;"
        "    color: white !important;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    padding: 5px 10px;"
        "}"
        "#searchButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5dade2, stop:1 #3498db);"
        "    color: white !important;"
        "}"
        
        "#clearHistoryButton {"
        "    background: rgba(255,59,48,0.1);"
        "    border: 1px solid rgba(255,59,48,0.3);"
        "    border-radius: 8px;"
        "    color: #FF3B30;"
        "    font-size: 13px;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"
        "#clearHistoryButton:hover {"
        "    background: rgba(255,59,48,0.2);"
        "}"
        
        "#avatarChangeButton {"
        "    background: rgba(0,122,255,0.1);"
        "    border: 1px solid rgba(0,122,255,0.3);"
        "    border-radius: 15px;"
        "    color: #007AFF;"
        "    font-size: 12px;"
        "    padding: 6px 12px;"
        "    font-weight: bold;"
        "}"
        "#avatarChangeButton:hover {"
        "    background: rgba(0,122,255,0.2);"
        "    border: 1px solid rgba(0,122,255,0.5);"
        "}";
}

void ModernQAWidget::onSendClicked()
{
    QString question = inputEdit_->toPlainText().trimmed();
    
    // 严格检查消息内容
    if (question.isEmpty() || question.length() == 0) {
        qWarning() << "发送的消息为空，已忽略";
        inputEdit_->clear();
        updateCharacterCount();
        return;
    }
    
    // 进一步检查：确保不只是空白字符
    QString testString = question;
    testString.replace(QRegularExpression("\\s+"), ""); // 移除所有空白字符
    if (testString.isEmpty() || testString.length() == 0) {
        qWarning() << "发送的消息只包含空白字符，已忽略"; 
        inputEdit_->clear();
        updateCharacterCount();
        return;
    }
    
    qDebug() << "即将发送消息:" << question << "长度:" << question.length();
    
    // 添加用户消息
    addMessage(MessageType::UserMessage, question);
    
    // 清空输入框
    inputEdit_->clear();
    updateCharacterCount();
    
    // 显示打字指示器
    showTypingIndicator();
    
    // 只使用本地数据库问答，不启用AI功能
    QTimer::singleShot(1500, [this, question]() {
        QAResult result = qaEngine_->askQuestion(question);
        result.isAIGenerated = false;
        
        hideTypingIndicator();
        addBotResponse(result);
        
        // 添加到历史记录
        addToHistory(question, result);
        
        emit questionAsked(question, result);
    });
}

void ModernQAWidget::onInputTextChanged()
{
    updateCharacterCount();
    
    QString text = inputEdit_->toPlainText().trimmed();
    sendButton_->setEnabled(!text.isEmpty() && text.length() <= 1000);
}

void ModernQAWidget::onQuickQuestionClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    QString question = item->text().trimmed();
    if (question.isEmpty()) {
        qWarning() << "快速问题为空，忽略";
        return;
    }
    
    qDebug() << "快速问题点击:" << question;
    inputEdit_->setPlainText(question);
    onSendClicked();
}

void ModernQAWidget::onHistoryItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    // 从历史记录中恢复对话
    QString itemText = item->text();
    // 这里可以实现历史记录的详细显示
    QMessageBox::information(this, "历史记录", "点击的历史记录: " + itemText);
}

void ModernQAWidget::onSidebarToggled()
{
    sidebarVisible_ = !sidebarVisible_;
    
    if (!sidebarAnimation_) {
        sidebarAnimation_ = new QPropertyAnimation(sidebarWidget_, "maximumWidth");
        sidebarAnimation_->setDuration(300);
        sidebarAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    }
    
    if (sidebarVisible_) {
        sidebarWidget_->setVisible(true);
        sidebarAnimation_->setStartValue(0);
        sidebarAnimation_->setEndValue(320);
        sidebarButton_->setText("隐藏");
        sidebarButton_->setToolTip("隐藏侧边栏");
    } else {
        sidebarAnimation_->setStartValue(320);
        sidebarAnimation_->setEndValue(0);
        sidebarButton_->setText("菜单");
        sidebarButton_->setToolTip("显示侧边栏");
        
        connect(sidebarAnimation_, &QPropertyAnimation::finished, [this]() {
            if (!sidebarVisible_) {
                sidebarWidget_->setVisible(false);
            }
            sidebarAnimation_->disconnect();
        });
    }
    
    sidebarAnimation_->start();
}

void ModernQAWidget::addMessage(MessageType type, const QString& message)
{
    // 调试信息：记录所有消息创建
    qDebug() << "addMessage调用 - 类型:" << (int)type << "消息内容:'" << message << "' 长度:" << message.length();
    
    // 严格验证消息内容
    QString cleanMessage = message.trimmed();
    
    // 对于用户消息和AI消息，进行严格验证
    if (type == MessageType::UserMessage || type == MessageType::BotMessage) {
        if (cleanMessage.isEmpty() || cleanMessage.length() == 0) {
            qWarning() << "阻止创建空白气泡 - 类型:" << (int)type << "消息为空";
            return;
        }
        
        // 检查是否只包含空白字符
        QString testMessage = cleanMessage;
        testMessage.replace(QRegularExpression("\\s+"), "");
        if (testMessage.isEmpty() || testMessage.length() == 0) {
            qWarning() << "阻止创建空白气泡 - 类型:" << (int)type << "消息只包含空白字符";
            return;
        }
        
        // 确保消息实际有可见内容
        if (testMessage == "" || testMessage.isNull()) {
            qWarning() << "阻止创建空白气泡 - 类型:" << (int)type << "消息内容无效";
            return;
        }
    }
    
    QPixmap avatar;
    
    // 根据消息类型选择头像
    if (type == MessageType::UserMessage) {
        // 使用用户头像
        if (avatarWidget_) {
            avatar = avatarWidget_->getAvatarPixmap();
        }
    } else if (type == MessageType::BotMessage) {
        // 使用AI助手头像
        AvatarManager& manager = AvatarManager::instance();
        avatar = manager.createAssistantAvatar();
    }
    // SystemMessage不需要头像
    
    ChatBubble* bubble = new ChatBubble(type, cleanMessage, avatar, messagesContainer_);
    
    // 插入到倒数第二个位置（在stretch之前）
    int insertIndex = messagesLayout_->count() - 1;
    messagesLayout_->insertWidget(insertIndex, bubble);
    
    messageBubbles_.append(bubble);
    
    // 连接气泡信号
    connect(bubble, &ChatBubble::copyRequested, [this](const QString& text) {
        Q_UNUSED(text)
        statusLabel_->setText("已复制到剪贴板");
        QTimer::singleShot(2000, [this]() {
            updateOnlineStatus();
        });
    });
    
    // 连接相关条款信号
    connect(bubble, &ChatBubble::showRelatedClauses, this, &ModernQAWidget::onShowRelatedClauses);
    
    // 动画效果
    animateNewMessage(bubble);
    
    // 滚动到底部
    QTimer::singleShot(100, this, &ModernQAWidget::scrollToBottom);
}

void ModernQAWidget::addBotResponse(const QAResult& result)
{
    // 检查结果是否为空
    QString formattedAnswer = formatAnswer(result);
    if (formattedAnswer.trimmed().isEmpty()) {
        qWarning() << "AI回答为空，添加默认回答";
        formattedAnswer = "抱歉，我暂时无法回答这个问题。请尝试重新表述您的问题，或者联系管理员获取帮助。";
    }
    
    // 使用AI助手头像
    AvatarManager& manager = AvatarManager::instance();
    QPixmap assistantAvatar = manager.createAssistantAvatar();
    
    ChatBubble* bubble = new ChatBubble(MessageType::BotMessage, formattedAnswer, assistantAvatar, messagesContainer_);
    
    if (!result.confidence.isEmpty()) {
        bubble->setConfidence(result.confidence);
    }
    if (!result.category.isEmpty()) {
        bubble->setCategory(result.category);
    }
    
    int insertIndex = messagesLayout_->count() - 1;
    messagesLayout_->insertWidget(insertIndex, bubble);
    
    messageBubbles_.append(bubble);
    
    connect(bubble, &ChatBubble::copyRequested, [this](const QString& text) {
        Q_UNUSED(text)
        statusLabel_->setText("已复制到剪贴板");
        QTimer::singleShot(2000, [this]() {
            updateOnlineStatus();
        });
    });
    
    // 连接相关条款信号
    connect(bubble, &ChatBubble::showRelatedClauses, this, &ModernQAWidget::onShowRelatedClauses);
    
    animateNewMessage(bubble);
    QTimer::singleShot(100, this, &ModernQAWidget::scrollToBottom);
}

void ModernQAWidget::showTypingIndicator()
{
    if (isTyping_) return;
    
    isTyping_ = true;
    
    // 为打字指示器也使用AI助手头像
    AvatarManager& manager = AvatarManager::instance();
    QPixmap assistantAvatar = manager.createAssistantAvatar();
    
    typingBubble_ = new ChatBubble(MessageType::TypingIndicator, "", assistantAvatar, messagesContainer_);
    
    int insertIndex = messagesLayout_->count() - 1;
    messagesLayout_->insertWidget(insertIndex, typingBubble_);
    
    // 添加到气泡列表中以便统一管理
    messageBubbles_.append(typingBubble_);
    
    typingBubble_->showTypingAnimation();
    animateNewMessage(typingBubble_);
    
    QTimer::singleShot(100, this, &ModernQAWidget::scrollToBottom);
}

void ModernQAWidget::hideTypingIndicator()
{
    if (!isTyping_ || !typingBubble_) return;
    
    isTyping_ = false;
    
    // 停止动画
    typingBubble_->hideTypingAnimation();
    
    // 从布局和气泡列表中移除
    messagesLayout_->removeWidget(typingBubble_);
    
    // 从messageBubbles_列表中移除，如果存在的话
    auto it = std::find(messageBubbles_.begin(), messageBubbles_.end(), typingBubble_);
    if (it != messageBubbles_.end()) {
        messageBubbles_.erase(it);
    }
    
    // 删除气泡对象
    typingBubble_->deleteLater();
    typingBubble_ = nullptr;
}

void ModernQAWidget::scrollToBottom()
{
    QScrollBar* scrollBar = messagesScrollArea_->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void ModernQAWidget::updateCharacterCount()
{
    int length = inputEdit_->toPlainText().length();
    characterCountLabel_->setText(QString("%1/1000").arg(length));
    
    if (length > 1000) {
        characterCountLabel_->setStyleSheet("color: #FF3B30; font-weight: bold;");
    } else if (length > 800) {
        characterCountLabel_->setStyleSheet("color: #FF9500; font-weight: bold;");
    } else {
        characterCountLabel_->setStyleSheet("color: rgba(0,0,0,0.5); font-size: 11px;");
    }
}

void ModernQAWidget::loadQuickQuestions()
{
    QStringList quickQuestions = {
        "公司的工作时间是什么？",
        "如何申请年假？",
        "社保缴费比例是多少？",
        "绩效考核制度是怎样的？",
        "新员工入职流程是什么？",
        "员工福利有哪些？",
        "如何申请调岗？",
        "病假需要什么手续？"
    };
    
    quickQuestionsList_->clear();
    for (const QString& question : quickQuestions) {
        QListWidgetItem* item = new QListWidgetItem(question);
        // 不设置图标，保持简洁
        quickQuestionsList_->addItem(item);
    }
}

void ModernQAWidget::loadHistory()
{
    // 从数据库加载历史记录
    QList<QuestionRecord> records = database_->getQuestionRecords(currentUser_.getEmployeeId());
    
    historyList_->clear();
    currentHistory_.clear();
    
    for (const QuestionRecord& record : records) {
        QAResult result;
        result.answer = record.getAnswer();
        result.category = record.getCategory();
        result.confidence = "高";
        
        currentHistory_.append(result);
        
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(record.getQuestion());
        item->setToolTip(QString("时间: %1\n类别: %2").arg(
            record.getCreateTime().toString("yyyy-MM-dd hh:mm"),
            record.getCategory()
        ));
        historyList_->addItem(item);
    }
}

void ModernQAWidget::addToHistory(const QString& question, const QAResult& result)
{
    // 添加到内存
    currentHistory_.prepend(result);
    
    // 添加到UI
    QListWidgetItem* item = new QListWidgetItem(question);
    item->setToolTip(QString("时间: %1\n类别: %2").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm"),
        result.category
    ));
    historyList_->insertItem(0, item);
    
    // 保存到数据库
    QuestionRecord record(currentUser_.getEmployeeId(), question, result.answer, result.category);
    database_->addQuestionRecord(record);
    
    // 限制历史记录数量
    while (historyList_->count() > 50) {
        delete historyList_->takeItem(historyList_->count() - 1);
        currentHistory_.removeLast();
    }
}

void ModernQAWidget::clearHistory()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认清空", 
        "您确定要清空所有历史记录吗？此操作不可撤销。",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        historyList_->clear();
        currentHistory_.clear();
        
        // 清空数据库记录
        // 注意：这里需要在Database类中实现清空指定用户记录的方法
        addMessage(MessageType::SystemMessage, "历史记录已清空");
    }
}

QString ModernQAWidget::formatAnswer(const QAResult& result)
{
    QString formatted;
    
    // 检查是否有AI回答
    if (result.isAIGenerated && !result.aiAnswer.isEmpty()) {
        // 显示AI增强回答
        formatted = QString("<div style='margin-bottom: 15px;'>"
                           "<h4 style='color: #2980b9; margin: 0 0 8px 0;'>AI智能回答</h4>"
                           "<div style='background: #ecf0f1; padding: 10px; border-radius: 8px; margin-bottom: 10px;'>%1</div>"
                           "</div>")
                    .arg(result.aiAnswer);
        
        // 如果有传统答案，也显示
        if (!result.traditionalAnswer.isEmpty() && 
            result.traditionalAnswer != "抱歉，我暂时无法回答这个问题。请联系HR部门获取更详细的信息，或查看相关制度文档。") {
            formatted += QString("<div style='margin-top: 10px;'>"
                               "<h4 style='color: #27ae60; margin: 0 0 8px 0;'>系统知识库</h4>"
                               "<div style='background: #d5e8d4; padding: 10px; border-radius: 8px;'>%1</div>"
                               "</div>")
                        .arg(result.traditionalAnswer);
        }
    } else {
        // 使用原始回答
        formatted = result.answer;
    }
    
    // 简单的Markdown格式支持
    QRegularExpression boldRegex("\\*\\*(.*?)\\*\\*");
    QRegularExpression italicRegex("\\*(.*?)\\*");
    
    formatted.replace(boldRegex, "<b>\\1</b>");
    formatted.replace(italicRegex, "<i>\\1</i>");
    formatted.replace("\n", "<br>");
    
    return formatted;
}

void ModernQAWidget::animateNewMessage(ChatBubble* bubble)
{
    if (!bubble) return;
    
    bubble->animateIn();
}

void ModernQAWidget::updateOnlineStatus()
{
    statusLabel_->setText("● 在线");
    statusLabel_->setStyleSheet("color: #34C759; font-size: 12px; font-weight: bold;");
}

void ModernQAWidget::askQuestion()
{
    inputEdit_->setFocus();
}

void ModernQAWidget::showQuickQuestions()
{
    if (!sidebarVisible_) {
        onSidebarToggled();
    }
    quickQuestionsList_->setFocus();
}

void ModernQAWidget::showHistory()
{
    if (!sidebarVisible_) {
        onSidebarToggled();
    }
    historyList_->setFocus();
}

void ModernQAWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // 调整消息气泡的大小
    for (ChatBubble* bubble : messageBubbles_) {
        bubble->update();
    }
}

void ModernQAWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    // 聚焦输入框
    inputEdit_->setFocus();
}

void ModernQAWidget::onTypingAnimationFinished()
{
    // 打字动画完成后的处理
    isTyping_ = false;
}

void ModernQAWidget::simulateTyping()
{
    // 模拟打字效果
    if (isTyping_) {
        // 这里可以添加打字动画效果
    }
}

void ModernQAWidget::finishTyping()
{
    // 完成打字效果并清理打字指示器
    if (isTyping_) {
        hideTypingIndicator();
    }
}

void ModernQAWidget::onAvatarChangeClicked()
{
    // 显示头像选择对话框
    AvatarSelectionDialog dialog(currentUser_, this);
    
    if (dialog.exec() == QDialog::Accepted) {
        QPixmap selectedAvatar = dialog.getSelectedAvatar();
        if (!selectedAvatar.isNull()) {
            // 更新头像组件
            avatarWidget_->setAvatarPixmap(selectedAvatar);
            
            // 显示更换成功消息
            statusLabel_->setText("头像已更新");
            QTimer::singleShot(2000, [this]() {
                updateOnlineStatus();
            });
        }
    }
}

// 附件功能已移除

void ModernQAWidget::onSettingsClicked()
{
    SettingsDialog dialog(this);
    
    if (dialog.exec() == QDialog::Accepted) {
        // 设置已保存和应用
        addMessage(MessageType::SystemMessage, "系统设置已更新！部分设置将在重启后生效。");
        
        // 显示状态提示
        statusLabel_->setText("设置已保存");
        QTimer::singleShot(2000, [this]() {
            updateOnlineStatus();
        });
    }
}

void ModernQAWidget::onShowRelatedClauses(const QString& question)
{
    if (!documentProcessor_) {
        QMessageBox::warning(this, "提示", "文档处理服务不可用");
        return;
    }
    
    RelatedClausesDialog dialog(question, documentProcessor_, this);
    dialog.exec();
}

// AI相关槽函数实现
void ModernQAWidget::onAIAnswerReceived(const QString& question, const QAResult& result)
{
    // 隐藏打字指示器
    hideTypingIndicator();
    
    // 显示AI增强的回答
    addBotResponse(result);
    
    // 添加到历史记录
    addToHistory(question, result);
    
    // 发出信号
    emit questionAsked(question, result);
}

// 增强历史记录功能实现
void ModernQAWidget::onAdvancedSearchTextChanged()
{
    QString keyword = advancedSearchEdit_->text().trimmed();
    if (keyword.isEmpty()) {
        // 显示所有历史记录
        for (int i = 0; i < historyList_->count(); ++i) {
            historyList_->item(i)->setHidden(false);
        }
        return;
    }
    
    searchHistory(keyword);
}

void ModernQAWidget::searchHistory(const QString& keyword)
{
    int visibleCount = 0;
    
    for (int i = 0; i < historyList_->count(); ++i) {
        QListWidgetItem* item = historyList_->item(i);
        QString itemText = item->text();
        QString tooltip = item->toolTip();
        
        // 在问题文本和提示中搜索关键词
        bool matches = itemText.contains(keyword, Qt::CaseInsensitive) || 
                      tooltip.contains(keyword, Qt::CaseInsensitive);
        
        item->setHidden(!matches);
        if (matches) visibleCount++;
    }
    
    // 显示搜索结果统计
    statusLabel_->setText(QString("找到 %1 条匹配记录").arg(visibleCount));
    QTimer::singleShot(3000, [this]() {
        updateOnlineStatus();
    });
}

void ModernQAWidget::onFilterHistoryByDate()
{
    QString category = historyFilterCombo_->currentData().toString();
    filterHistoryByCategory(category);
}

void ModernQAWidget::filterHistoryByCategory(const QString& category)
{
    int visibleCount = 0;
    
    for (int i = 0; i < historyList_->count(); ++i) {
        QListWidgetItem* item = historyList_->item(i);
        bool shouldShow = true;
        
        if (!category.isEmpty()) {
            // 从工具提示中提取类别信息
            QString tooltip = item->toolTip();
            shouldShow = tooltip.contains(QString("类别: %1").arg(category));
        }
        
        item->setHidden(!shouldShow);
        if (shouldShow) visibleCount++;
    }
    
    if (category.isEmpty()) {
        statusLabel_->setText("显示全部历史记录");
    } else {
        statusLabel_->setText(QString("显示 %1 类别的 %2 条记录").arg(category).arg(visibleCount));
    }
    
    QTimer::singleShot(3000, [this]() {
        updateOnlineStatus();
    });
}




