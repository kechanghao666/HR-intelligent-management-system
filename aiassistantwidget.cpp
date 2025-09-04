#include "aiassistantwidget.h"
#include <QDateTime>
#include <QApplication>
#include <QDebug>
#include <QResizeEvent>
#include <QClipboard>
#include <QPainter>

AIAssistantWidget::AIAssistantWidget(QWidget *parent)
    : QWidget(parent)
    , mainLayout_(nullptr)
    , titleBar_(nullptr)
    , titleLayout_(nullptr)
    , titleLabel_(nullptr)
    , statusLabel_(nullptr)
    , clearButton_(nullptr)
    , sidebarToggleButton_(nullptr)
    , modeToggleButton_(nullptr)
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
    , historyGroup_(nullptr)
    , historyList_(nullptr)
    , clearHistoryButton_(nullptr)
    , historySearchEdit_(nullptr)
    , sidebarVisible_(true)
    , xfyunAPI_(nullptr)
    , statusTimer_(nullptr)
    , timeoutTimer_(nullptr)
    , isProcessing_(false)
    , messageCount_(0)
    , currentQuestion_()
    , formFiller_(nullptr)
    , currentForm_(nullptr)
    , formExporter_(nullptr)
    , formPreviewer_(nullptr)
    , formMode_(false)
    , isFormModeEnabled_(true)
    , hasShownWelcome_(false)
{
    setupUI();
    setupSidebar();
    setupConnections();
    applyModernStyles();
    loadHistory();
    showWelcomeMessage();
    
    // 初始化自动填表组件
    formFiller_ = new AutoFormFiller();
    formExporter_ = new FormExporter();
    formPreviewer_ = new FormPreviewer();
    
            // 初始化API - 精简提示信息
        try {
            xfyunAPI_ = new XfyunAPI(this);
            if (xfyunAPI_) {
                connect(xfyunAPI_, &XfyunAPI::answerReceived, this, &AIAssistantWidget::onAIAnswerReceived);
                // 移除初始化提示，让界面更简洁
                // addSystemMessage("AI助手正在初始化，基于星火大模型为您提供智能问答服务");
                // addSystemMessage("请发送一条消息以测试网络连接状态");
            } else {
                qCritical() << "[AI助手] XfyunAPI初始化失败";
                addSystemMessage("AI服务初始化失败");
                statusLabel_->setText("离线");
                statusLabel_->setStyleSheet(
                    "color: #dc2626; background: #fee2e2; "
                    "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
                );
            }
        } catch (const std::exception& e) {
            qCritical() << "[AI助手] 初始化异常:" << e.what();
            xfyunAPI_ = nullptr;
            addSystemMessage("AI服务初始化异常");
            statusLabel_->setText("错误");
            statusLabel_->setStyleSheet(
                "color: #dc2626; background: #fee2e2; "
                "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
            );
        } catch (...) {
            qCritical() << "[AI助手] 初始化未知异常";
            xfyunAPI_ = nullptr;
            addSystemMessage("AI服务初始化失败");
            statusLabel_->setText("错误");
            statusLabel_->setStyleSheet(
                "color: #dc2626; background: #fee2e2; "
                "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
            );
        }
    
    // 设置状态定时器
    statusTimer_ = new QTimer(this);
    connect(statusTimer_, &QTimer::timeout, this, &AIAssistantWidget::updateConnectionStatus);
    statusTimer_->start(3000);
    
    // 设置网络超时定时器
    timeoutTimer_ = new QTimer(this);
    timeoutTimer_->setSingleShot(true);
    connect(timeoutTimer_, &QTimer::timeout, this, &AIAssistantWidget::onNetworkTimeout);
}

AIAssistantWidget::~AIAssistantWidget()
{
    if (statusTimer_) {
        statusTimer_->stop();
    }
    if (timeoutTimer_) {
        timeoutTimer_->stop();
    }
    
    // 清理表单相关资源
    if (currentForm_) {
        delete currentForm_;
        currentForm_ = nullptr;
    }
    if (formFiller_) {
        delete formFiller_;
        formFiller_ = nullptr;
    }
    if (formExporter_) {
        delete formExporter_;
        formExporter_ = nullptr;
    }
    if (formPreviewer_) {
        delete formPreviewer_;
        formPreviewer_ = nullptr;
    }
}

void AIAssistantWidget::setupUI()
{
    // 主布局 - 支持侧边栏的水平布局
    QHBoxLayout* outerLayout = new QHBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);
    
    // 创建主聊天区域
    QWidget* chatWidget = new QWidget();
    mainLayout_ = new QVBoxLayout(chatWidget);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    
    setupTitleBar();
    setupMessagesArea();
    setupInputArea();
    
    outerLayout->addWidget(chatWidget, 1);
}

void AIAssistantWidget::setupTitleBar()
{
    // 标题栏 - 简洁专业设计，参考ChatGPT等一流平台
    titleBar_ = new QFrame();
    titleBar_->setObjectName("titleBar");
    titleBar_->setFixedHeight(50);
    
    titleLayout_ = new QHBoxLayout(titleBar_);
    titleLayout_->setContentsMargins(20, 8, 20, 8);
    titleLayout_->setSpacing(12);
    
    // 标题 - 去除表情符号，使用简洁文字
    titleLabel_ = new QLabel("AI助手");
    titleLabel_->setObjectName("titleLabel");
    titleLabel_->setStyleSheet("font-size: 16px; font-weight: 600; color: #1a1a1a;");
    
    // 状态标签 - 简洁的状态指示
    statusLabel_ = new QLabel("待机");
    statusLabel_->setObjectName("statusLabel");
    
    // 清空按钮 - 自适应中文文字大小
    clearButton_ = new QPushButton("清空");
    clearButton_->setObjectName("clearButton");
    clearButton_->setMinimumSize(50, 28);
    clearButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    // 侧边栏切换按钮
    sidebarToggleButton_ = new QPushButton("历史");
    sidebarToggleButton_->setObjectName("sidebarToggleButton");
    sidebarToggleButton_->setMinimumSize(50, 28);
    sidebarToggleButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    
    // 模式切换按钮
    modeToggleButton_ = new QPushButton("问答模式");
    modeToggleButton_->setObjectName("modeToggleButton");
    modeToggleButton_->setMinimumSize(80, 28);
    modeToggleButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    modeToggleButton_->setToolTip("点击切换AI问答模式和智能填表模式");
    
    titleLayout_->addWidget(titleLabel_);
    titleLayout_->addStretch();
    titleLayout_->addWidget(statusLabel_);
    titleLayout_->addWidget(modeToggleButton_);
    titleLayout_->addWidget(clearButton_);
    titleLayout_->addWidget(sidebarToggleButton_);
    
    mainLayout_->addWidget(titleBar_);
}

void AIAssistantWidget::setupMessagesArea()
{
    // 消息区域 - 参考一流平台的简洁设计
    messagesScrollArea_ = new QScrollArea();
    messagesScrollArea_->setWidgetResizable(true);
    messagesScrollArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messagesScrollArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    messagesScrollArea_->setFrameShape(QFrame::NoFrame);
    messagesScrollArea_->setObjectName("messagesScrollArea");
    
    messagesContainer_ = new QWidget();
    messagesLayout_ = new QVBoxLayout(messagesContainer_);
    messagesLayout_->setContentsMargins(20, 15, 20, 15);
    messagesLayout_->setSpacing(12);
    messagesLayout_->addStretch();
    
    messagesScrollArea_->setWidget(messagesContainer_);
    mainLayout_->addWidget(messagesScrollArea_, 1);
}

void AIAssistantWidget::setupInputArea()
{
    // 输入区域 - 参考一流平台的简洁实用设计
    inputFrame_ = new QFrame();
    inputFrame_->setObjectName("inputFrame");
    inputFrame_->setMinimumHeight(70);
    inputFrame_->setMaximumHeight(120);
    
    inputLayout_ = new QHBoxLayout(inputFrame_);
    inputLayout_->setContentsMargins(20, 12, 20, 12);
    inputLayout_->setSpacing(12);
    
    // 输入框 - 使用QTextEdit支持多行输入
    inputEdit_ = new QTextEdit();
    inputEdit_->setObjectName("inputEdit");
    inputEdit_->setPlaceholderText("输入您的问题...");
    inputEdit_->setMaximumHeight(80);
    inputEdit_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    inputEdit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // 发送按钮 - 自适应中文文字大小
    sendButton_ = new QPushButton("发送");
    sendButton_->setObjectName("sendButton");
    sendButton_->setMinimumSize(80, 36);
    sendButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    sendButton_->setEnabled(false);
    
    // 字符计数 - 简洁显示
    characterCountLabel_ = new QLabel("0/1000");
    characterCountLabel_->setObjectName("characterCount");
    characterCountLabel_->setAlignment(Qt::AlignRight);
    
    // 右侧布局
    QVBoxLayout* inputRightLayout = new QVBoxLayout();
    inputRightLayout->addWidget(sendButton_);
    inputRightLayout->addWidget(characterCountLabel_);
    inputRightLayout->addStretch();
    
    inputLayout_->addWidget(inputEdit_, 1);
    inputLayout_->addLayout(inputRightLayout);
    
    mainLayout_->addWidget(inputFrame_);
}

void AIAssistantWidget::setupSidebar()
{
    // 创建侧边栏
    sidebarWidget_ = new QWidget();
    sidebarWidget_->setFixedWidth(300);
    sidebarWidget_->setObjectName("aiSidebarWidget");
    
    sidebarLayout_ = new QVBoxLayout(sidebarWidget_);
    sidebarLayout_->setContentsMargins(0, 0, 0, 0);
    sidebarLayout_->setSpacing(0);
    
    sidebarFrame_ = new QFrame();
    sidebarFrame_->setObjectName("aiSidebarFrame");
    
    QVBoxLayout* frameLayout = new QVBoxLayout(sidebarFrame_);
    frameLayout->setContentsMargins(15, 15, 15, 15);
    frameLayout->setSpacing(15);
    
    // 历史记录组
    historyGroup_ = new QGroupBox("对话历史");
    historyGroup_->setObjectName("aiHistoryGroup");
    historyGroup_->setMinimumHeight(450);
    QVBoxLayout* historyLayout = new QVBoxLayout(historyGroup_);
    historyLayout->setContentsMargins(12, 20, 12, 12);
    historyLayout->setSpacing(8);
    
    // 搜索框
    historySearchEdit_ = new QLineEdit();
    historySearchEdit_->setObjectName("aiHistorySearchEdit");
    historySearchEdit_->setPlaceholderText("搜索历史对话...");
    historySearchEdit_->setMinimumHeight(32);
    historySearchEdit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    historyLayout->addWidget(historySearchEdit_);
    
    // 历史记录列表
    historyList_ = new QListWidget();
    historyList_->setObjectName("aiHistoryList");
    historyList_->setMinimumHeight(280);
    historyList_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    historyLayout->addWidget(historyList_, 1); // 给列表更多伸展空间
    
    // 清空按钮
    clearHistoryButton_ = new QPushButton("清空历史");
    clearHistoryButton_->setObjectName("aiClearHistoryButton");
    clearHistoryButton_->setMinimumSize(100, 35);
    clearHistoryButton_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    historyLayout->addWidget(clearHistoryButton_);
    
    frameLayout->addWidget(historyGroup_);
    frameLayout->addStretch();
    
    sidebarLayout_->addWidget(sidebarFrame_);
    
    // 将侧边栏添加到主布局
    QHBoxLayout* outerLayout = qobject_cast<QHBoxLayout*>(this->layout());
    if (outerLayout) {
        outerLayout->addWidget(sidebarWidget_, 0);
    }
}

void AIAssistantWidget::setupConnections()
{
    connect(sendButton_, &QPushButton::clicked, this, &AIAssistantWidget::onSendClicked);
    connect(clearButton_, &QPushButton::clicked, this, &AIAssistantWidget::onClearChat);
    connect(inputEdit_, &QTextEdit::textChanged, this, &AIAssistantWidget::onInputTextChanged);
    
    // 侧边栏连接
    connect(sidebarToggleButton_, &QPushButton::clicked, this, &AIAssistantWidget::onSidebarToggled);
    connect(modeToggleButton_, &QPushButton::clicked, this, &AIAssistantWidget::onModeToggled);
    connect(historyList_, &QListWidget::itemClicked, this, &AIAssistantWidget::onHistoryItemClicked);
    connect(clearHistoryButton_, &QPushButton::clicked, this, &AIAssistantWidget::onClearHistoryClicked);
    connect(historySearchEdit_, &QLineEdit::textChanged, this, &AIAssistantWidget::onHistorySearchChanged);
}

void AIAssistantWidget::applyModernStyles()
{
    // 参考QQ、微信、ChatGPT等一流AI问答平台的现代化设计
    setStyleSheet(R"(
        /* 主容器 - 整体背景 */
        AIAssistantWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #f8fafc, stop:1 #f1f5f9);
        }
        
        /* 标题栏 - 现代简约风格 */
        #titleBar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #ffffff, stop:1 #f8fafc);
            border-bottom: 1px solid #e2e8f0;
            border-radius: 0;
        }
        
        #titleLabel {
            font-size: 18px;
            font-weight: 700;
            color: #1e293b;
            font-family: "Microsoft YaHei", "PingFang SC", "Helvetica Neue", Arial, sans-serif;
        }
        
        #statusLabel {
            font-size: 11px;
            color: #059669;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #d1fae5, stop:1 #a7f3d0);
            padding: 4px 10px;
            border-radius: 12px;
            font-weight: 600;
            border: 1px solid #6ee7b7;
        }
        
        #clearButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #f8fafc, stop:1 #e2e8f0);
            color: #64748b;
            border: 1px solid #cbd5e1;
            border-radius: 8px;
            font-weight: 600;
            font-size: 12px;
            padding: 6px 12px;
            min-width: 60px;
        }
        
        #clearButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #e2e8f0, stop:1 #cbd5e1);
            color: #475569;
            border: 1px solid #94a3b8;
        }
        
        #clearButton:pressed {
            background: #cbd5e1;
            transform: translateY(1px);
        }
        
        /* 侧边栏历史按钮 - 现代圆角设计 */
        #sidebarToggleButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #3b82f6, stop:1 #2563eb);
            color: white;
            border: 1px solid #1d4ed8;
            border-radius: 12px;
            font-weight: 600;
            font-size: 12px;
            padding: 6px 12px;
            min-width: 50px;
        }
        
        #sidebarToggleButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #4f46e5, stop:1 #3b82f6);
            border: 1px solid #2563eb;
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(59, 130, 246, 0.3);
        }
        
        #sidebarToggleButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #1e40af, stop:1 #1d4ed8);
            transform: translateY(0px);
            box-shadow: 0 2px 4px rgba(59, 130, 246, 0.2);
        }
        
        /* 模式切换按钮 - 现代圆角设计 */
        #modeToggleButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #059669, stop:1 #047857);
            color: white;
            border: 1px solid #065f46;
            border-radius: 12px;
            font-weight: 600;
            font-size: 11px;
            padding: 6px 12px;
            min-width: 80px;
        }
        
        #modeToggleButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #10b981, stop:1 #059669);
            border: 1px solid #047857;
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(16, 185, 129, 0.3);
        }
        
        #modeToggleButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #047857, stop:1 #065f46);
            transform: translateY(0px);
            box-shadow: 0 2px 4px rgba(16, 185, 129, 0.2);
        }
        
        /* 消息区域 - 仿微信聊天背景 */
        #messagesScrollArea {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, 
                stop:0 #f8fafc, stop:0.5 #f1f5f9, stop:1 #e2e8f0);
            border: none;
        }
        
        /* 侧边栏 - 现代化设计 */
        #sidebarFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0, 
                stop:0 #ffffff, stop:1 #f8fafc);
            border-left: 1px solid #e2e8f0;
        }
        
        /* 输入区域 - 仿ChatGPT风格 */
        #inputFrame {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #ffffff, stop:1 #f8fafc);
            border-top: 1px solid #e2e8f0;
            padding: 16px;
        }
        
        #inputEdit {
            background: #ffffff;
            border: 2px solid #e2e8f0;
            border-radius: 12px;
            padding: 14px 16px;
            font-size: 14px;
            color: #1e293b;
            font-family: "Microsoft YaHei", "PingFang SC", sans-serif;
            selection-background-color: #3b82f6;
            selection-color: white;
            line-height: 1.5;
        }
        
        #inputEdit:focus {
            border: 2px solid #3b82f6;
            outline: none;
            background: #fefefe;
            box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.1);
        }
        
        #inputEdit:hover {
            border: 2px solid #94a3b8;
        }
        
        /* 发送按钮 - 现代化圆润设计 */
        #sendButton {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #4f46e5, stop:1 #3b82f6);
            color: white;
            border: none;
            border-radius: 10px;
            font-weight: 600;
            font-size: 13px;
            padding: 12px 20px;
            min-width: 80px;
            font-family: "Microsoft YaHei", sans-serif;
        }
        
        #sendButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #4338ca, stop:1 #2563eb);
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(59, 130, 246, 0.4);
        }
        
        #sendButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #3730a3, stop:1 #1d4ed8);
            transform: translateY(0px);
            box-shadow: 0 2px 4px rgba(59, 130, 246, 0.3);
        }
        
        #sendButton:disabled {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1, 
                stop:0 #9ca3af, stop:1 #6b7280);
            color: #ffffff;
            transform: none;
            box-shadow: none;
        }
        
        #characterCount {
            color: #64748b;
            font-size: 11px;
            margin-top: 6px;
            font-weight: 500;
        }
        
        /* 滚动条美化 - 仿现代应用 */
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        
        QScrollBar::handle:vertical {
            background: rgba(148, 163, 184, 0.5);
            border-radius: 4px;
            min-height: 20px;
        }
        
        QScrollBar::handle:vertical:hover {
            background: rgba(100, 116, 139, 0.7);
        }
        
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0;
            background: none;
        }
        
        /* 侧边栏组件样式 */
        #historyGroup {
            font-size: 14px;
            font-weight: 600;
            color: #1e293b;
            border: 1px solid #e2e8f0;
            border-radius: 12px;
            margin-top: 10px;
            padding-top: 15px;
            background: rgba(255,255,255,0.8);
        }
        
        #historyGroup::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 4px 8px;
            color: #1e293b;
            font-size: 14px;
            font-weight: 600;
        }
        
        #historyList {
            background: transparent;
            border: none;
            outline: none;
            font-size: 13px;
            color: #374151;
        }
        
        #historyList::item {
            padding: 12px 10px;
            margin: 2px 4px;
            border-radius: 8px;
            background: rgba(255,255,255,0.9);
            border: 1px solid rgba(0,0,0,0.06);
            font-size: 12px;
            line-height: 1.4;
            min-height: 20px;
        }
        
        #historyList::item:hover {
            background: rgba(59,130,246,0.08);
            border: 1px solid rgba(59,130,246,0.2);
            color: #1e40af;
        }
        
        #historyList::item:selected {
            background: rgba(59,130,246,0.15);
            border: 1px solid rgba(59,130,246,0.4);
            color: #1e40af;
        }
        
        #historySearchEdit {
            background: white;
            border: 2px solid #e2e8f0;
            border-radius: 10px;
            padding: 10px 14px;
            font-size: 13px;
            color: #374151;
        }
        
        #historySearchEdit:focus {
            border: 2px solid #3b82f6;
            outline: none;
            box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.1);
        }
        
        #historySearchEdit:hover {
            border: 2px solid #94a3b8;
        }
        
        /* 清空历史按钮 - 现代圆角设计 */
        #aiClearHistoryButton {
            background: rgba(239,68,68,0.1);
            border: 1px solid rgba(239,68,68,0.3);
            border-radius: 10px;
            color: #dc2626;
            font-size: 13px;
            font-weight: 600;
            padding: 10px 16px;
            min-width: 100px;
        }
        
        #aiClearHistoryButton:hover {
            background: rgba(239,68,68,0.15);
            border: 1px solid rgba(239,68,68,0.4);
            transform: translateY(-1px);
            box-shadow: 0 4px 12px rgba(239, 68, 68, 0.2);
        }
        
        #aiClearHistoryButton:pressed {
            background: rgba(239,68,68,0.2);
            transform: translateY(0px);
            box-shadow: 0 2px 4px rgba(239, 68, 68, 0.15);
        }
    )");
}

void AIAssistantWidget::onSendClicked()
{
    QString question = inputEdit_->toPlainText().trimmed();
    if (question.isEmpty() || isProcessing_) return;
    
    // 检查字符长度
    if (question.length() > MAX_CHARS) {
        addSystemMessage(QString("问题长度超出限制（最多%1字符），请简化后重试").arg(MAX_CHARS));
        return;
    }
    
    // 添加用户消息
    addUserMessage(question);
    inputEdit_->clear();
    
    // 存储当前问题用于历史记录
    currentQuestion_ = question;
    
    // 根据当前模式处理用户输入
    if (isFormModeEnabled_) {
        // 表单模式：首先检查是否是表单相关的命令
        if (isFormCommand(question)) {
            handleFormCommand(question);
            return;
        }
        
        // 尝试自动填表功能
        processFormInput(question);
    } else {
        // 问答模式：直接进行AI问答
        enableInput(false);
        isProcessing_ = true;
        statusLabel_->setText("连接中");
        statusLabel_->setStyleSheet(
            "color: #3b82f6; background: #dbeafe; "
            "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
        );
        
        // 启动超时保护
        timeoutTimer_->start(30000);
        
        // 发送到AI - 增强错误处理
        qDebug() << "[AI助手] 发送问题:" << question;
        
        // 安全检查：确保xfyunAPI_存在且网络可用
        if (!xfyunAPI_) {
            qWarning() << "[AI助手] XfyunAPI对象未初始化";
            addSystemMessage("AI服务初始化失败，请重启应用程序");
            onNetworkTimeout();
            return;
        }
        
        try {
            // 尝试发送问题到AI
            xfyunAPI_->askQuestion(question);
        } catch (const std::exception& e) {
            qCritical() << "[AI助手] 发送问题时发生异常:" << e.what();
            addSystemMessage("发送问题时发生错误，请稍后重试");
            onNetworkTimeout();
        } catch (...) {
            qCritical() << "[AI助手] 发送问题时发生未知异常";
            addSystemMessage("发生未知错误，请稍后重试");
            onNetworkTimeout();
        }
    }
}

void AIAssistantWidget::onInputTextChanged()
{
    QString text = inputEdit_->toPlainText();
    int charCount = text.length();
    
    // 更新字符计数
    characterCountLabel_->setText(QString("%1/%2").arg(charCount).arg(MAX_CHARS));
    
    // 控制发送按钮状态
    sendButton_->setEnabled(!text.trimmed().isEmpty() && !isProcessing_ && charCount <= MAX_CHARS);
    
    // 字符数警告
    if (charCount > MAX_CHARS) {
        characterCountLabel_->setStyleSheet("color: #dc2626; font-weight: bold;");
    } else if (charCount > MAX_CHARS * 0.8) {
        characterCountLabel_->setStyleSheet("color: #d97706; font-weight: bold;");
    } else {
        characterCountLabel_->setStyleSheet("color: #6b7280;");
    }
}

void AIAssistantWidget::onAIAnswerReceived(const QString &answer)
{
    qDebug() << "[AI助手] 收到回答:" << answer;
    
    // 停止超时定时器
    timeoutTimer_->stop();
    
    // 恢复状态
    enableInput(true);
    isProcessing_ = false;
    
    // 添加AI回答
    QString finalAnswer;
    if (answer.trimmed().isEmpty()) {
        finalAnswer = "抱歉，我暂时无法回答这个问题，请稍后再试";
        addAIMessage(finalAnswer);
        addSystemMessage("建议检查网络连接或稍后重试");
        // 如果回答为空，显示连接有问题的状态
        statusLabel_->setText("不稳定");
        statusLabel_->setStyleSheet(
            "color: #d97706; background: #fef3c7; "
            "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
        );
    } else if (answer.contains("网络") || answer.contains("错误") || answer.contains("失败")) {
        // 如果回答包含错误信息，显示网络问题状态
        finalAnswer = answer;
        addAIMessage(finalAnswer);
        statusLabel_->setText("网络问题");
        statusLabel_->setStyleSheet(
            "color: #dc2626; background: #fee2e2; "
            "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
        );
    } else {
        // 正常回答，显示就绪状态
        finalAnswer = answer;
        addAIMessage(finalAnswer);
        statusLabel_->setText("已连接");
        statusLabel_->setStyleSheet(
            "color: #059669; background: #d1fae5; "
            "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
        );
    }
    
    // 保存到历史记录
    if (!currentQuestion_.isEmpty()) {
        addToHistory(currentQuestion_, finalAnswer);
        currentQuestion_.clear(); // 清空当前问题
    }
    
    // 聚焦到输入框
    inputEdit_->setFocus();
}

void AIAssistantWidget::onClearChat()
{
    // 保存第一条欢迎消息（如果存在）
    QWidget* welcomeWidget = nullptr;
    if (messagesLayout_->count() > 1) {
        QLayoutItem* firstItem = messagesLayout_->itemAt(0);
        if (firstItem && firstItem->widget()) {
            welcomeWidget = firstItem->widget();
            messagesLayout_->removeItem(firstItem);
        }
    }
    
    // 清空对话布局中的其他所有消息
    QLayoutItem* item;
    while ((item = messagesLayout_->takeAt(0)) != nullptr) {
        if (item->widget() && item->widget() != welcomeWidget) {
            item->widget()->deleteLater();
        }
        delete item;
    }
    
    // 如果有欢迎消息，重新添加到顶部
    if (welcomeWidget) {
        messagesLayout_->insertWidget(0, welcomeWidget);
        messageCount_ = 1; // 如果有欢迎消息，计数从1开始
    } else {
        messageCount_ = 0;
        // 只有在从未显示过欢迎消息时才显示
        if (!hasShownWelcome_) {
            showWelcomeMessage();
        }
    }
    
    // 重新添加弹簧以保持布局
    messagesLayout_->addStretch();
    
    addSystemMessage("对话已清空，AI助手重新准备就绪");
}

void AIAssistantWidget::updateConnectionStatus()
{
    if (!isProcessing_) {
        // 检查API是否可用
        if (xfyunAPI_) {
            statusLabel_->setText("待机");
            statusLabel_->setStyleSheet(
                "color: #6366f1; background: #e0e7ff; "
                "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
            );
        } else {
            statusLabel_->setText("离线");
            statusLabel_->setStyleSheet(
                "color: #dc2626; background: #fee2e2; "
                "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
            );
        }
    }
}

void AIAssistantWidget::onNetworkTimeout()
{
    qDebug() << "[AI助手] 网络请求超时";
    
    // 恢复状态
    enableInput(true);
    isProcessing_ = false;
    statusLabel_->setText("超时");
    statusLabel_->setStyleSheet(
        "color: #dc2626; background: #fee2e2; "
        "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
    );
    
    addAIMessage("网络连接超时，请检查网络连接后重试");
}

void AIAssistantWidget::enableInput(bool enabled)
{
    inputEdit_->setEnabled(enabled);
    sendButton_->setEnabled(enabled && !inputEdit_->toPlainText().trimmed().isEmpty());
    sendButton_->setText(enabled ? "发送" : "发送中");
}

void AIAssistantWidget::addUserMessage(const QString &message)
{
    // 使用现代化ChatBubble组件 - 仿微信/QQ用户消息
    ChatBubble* bubble = new ChatBubble(MessageType::UserMessage, message);
    bubble->setTimestamp(QDateTime::currentDateTime());
    
    // 设置用户头像（可以根据需要自定义）
    QPixmap userAvatar(":/icons/user-avatar.png");
    if (userAvatar.isNull()) {
        // 如果图标不存在，创建一个简单的用户头像
        userAvatar = QPixmap(36, 36);
        userAvatar.fill(QColor(79, 70, 229)); // 蓝色背景
        QPainter painter(&userAvatar);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 14, QFont::Bold));
        painter.drawText(userAvatar.rect(), Qt::AlignCenter, "我");
    }
    bubble->setAvatar(userAvatar);
    
    // 连接相关条款信号（只在问答模式下启用）
    if (!isFormModeEnabled_) {
        connect(bubble, &ChatBubble::showRelatedClauses, this, &AIAssistantWidget::onShowRelatedClauses);
        bubble->setRelatedClausesVisible(true);  // 问答模式下显示相关条款按钮
    } else {
        bubble->setRelatedClausesVisible(false); // 表单模式下隐藏相关条款按钮
    }
    
    // 连接复制功能（所有模式都启用）
    connect(bubble, &ChatBubble::copyRequested, [message](const QString& text) {
        Q_UNUSED(text)
        QApplication::clipboard()->setText(message);
    });
    
    // 添加到布局并显示动画
    messagesLayout_->insertWidget(messagesLayout_->count() - 1, bubble);
    bubble->animateIn();
    scrollToBottom();
    messageCount_++;
}

void AIAssistantWidget::addAIMessage(const QString &message)
{
    // 使用现代化ChatBubble组件 - 仿ChatGPT/Claude AI回复
    ChatBubble* bubble = new ChatBubble(MessageType::BotMessage, message);
    bubble->setTimestamp(QDateTime::currentDateTime());
    bubble->setCategory("AI助手回复");
    
    // 设置AI头像（可以根据需要自定义）
    QPixmap aiAvatar(":/icons/ai-avatar.png");
    if (aiAvatar.isNull()) {
        // 如果图标不存在，创建一个简单的AI头像
        aiAvatar = QPixmap(36, 36);
        aiAvatar.fill(QColor(16, 185, 129)); // 绿色背景
        QPainter painter(&aiAvatar);
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
        painter.drawText(aiAvatar.rect(), Qt::AlignCenter, "AI");
    }
    bubble->setAvatar(aiAvatar);
    
    // 连接复制功能
    connect(bubble, &ChatBubble::copyRequested, [message](const QString& text) {
        Q_UNUSED(text)
        QApplication::clipboard()->setText(message);
        // 可以添加复制成功的提示
    });
    
    // 添加到布局并显示动画
    messagesLayout_->insertWidget(messagesLayout_->count() - 1, bubble);
    bubble->animateIn();
    scrollToBottom();
    messageCount_++;
}

void AIAssistantWidget::addSystemMessage(const QString &message)
{
    // 使用现代化ChatBubble组件 - 系统消息
    ChatBubble* bubble = new ChatBubble(MessageType::SystemMessage, message);
    bubble->setTimestamp(QDateTime::currentDateTime());
    
    // 系统消息不需要头像
    bubble->setAvatar(QPixmap());
    
    // 添加到布局并显示动画
    messagesLayout_->insertWidget(messagesLayout_->count() - 1, bubble);
    bubble->animateIn();
    scrollToBottom();
}

void AIAssistantWidget::scrollToBottom()
{
    QTimer::singleShot(10, [this]() {
        QScrollBar* scrollBar = messagesScrollArea_->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    });
}

void AIAssistantWidget::showWelcomeMessage()
{
    // 只有在未显示过欢迎消息时才显示
    if (hasShownWelcome_) {
        return;
    }
    
    QString welcomeMessage;
    
    if (isFormModeEnabled_) {
        // 智能填表模式欢迎消息
        welcomeMessage = "欢迎使用AI智能填表助手！\n\n"
                        "我可以帮您自动填写各种表单：\n\n"
                        "【支持的表单类型】\n"
                        "• 员工信息表：\"我是张三，工号A001，在技术部工作\"\n"
                        "• 请假申请表：\"我要请假，明天开始请3天病假\"\n\n"
                        "【可用操作】\n"
                        "• 自动导出：\"导出表单\"生成PDF/HTML/CSV文件\n"
                        "• 查看表单：\"查看表单\"或\"预览表单\"\n"
                        "• 清空重置：\"清空表单\"重新开始\n\n"
                        "点击右上角\"问答模式\"按钮可切换到AI问答功能\n\n"
                        "请开始输入表单信息，我会智能识别并为您填写！";
    } else {
        // AI问答模式欢迎消息
        welcomeMessage = "欢迎使用AI智能问答助手！\n\n"
                        "我可以为您解答各种HR相关问题：\n\n"
                        "【专业服务】\n"
                        "• 考勤制度和打卡规定\n"
                        "• 请假流程和假期政策\n"
                        "• 薪酬结构和发放时间\n"
                        "• 培训机会和发展规划\n"
                        "• 员工福利和保险政策\n"
                        "• 各部门联系方式和办事流程\n\n"
                        "点击右上角\"填表模式\"按钮可切换到智能填表功能\n\n"
                        "请输入您想了解的问题，我会为您提供专业解答！";
    }
    
    addFormResponseMessage(welcomeMessage);
    hasShownWelcome_ = true; // 标记已显示欢迎消息
}

void AIAssistantWidget::updateWelcomeMessageForCurrentMode()
{
    // 如果还没有显示过欢迎消息，就不需要更新
    if (!hasShownWelcome_) {
        return;
    }
    
    // 查找第一个消息是否为欢迎消息
    if (messagesLayout_->count() > 1) {
        QLayoutItem* firstItem = messagesLayout_->itemAt(0);
        if (firstItem && firstItem->widget()) {
            ChatBubble* firstBubble = qobject_cast<ChatBubble*>(firstItem->widget());
            if (firstBubble) {
                // 更新欢迎消息内容
                QString welcomeMessage;
                if (isFormModeEnabled_) {
                    welcomeMessage = "欢迎使用AI智能填表助手！\n\n"
                                    "我可以帮您自动填写各种表单：\n\n"
                                    "【支持的表单类型】\n"
                                    "• 员工信息表：\"我是张三，工号A001，在技术部工作\"\n"
                                    "• 请假申请表：\"我要请假，明天开始请3天病假\"\n\n"
                                    "【可用操作】\n"
                                    "• 自动导出：\"导出表单\"生成PDF/HTML/CSV文件\n"
                                    "• 查看表单：\"查看表单\"或\"预览表单\"\n"
                                    "• 清空重置：\"清空表单\"重新开始\n\n"
                                    "点击右上角\"问答模式\"按钮可切换到AI问答功能\n\n"
                                    "请开始输入表单信息，我会智能识别并为您填写！";
                } else {
                    welcomeMessage = "欢迎使用AI智能问答助手！\n\n"
                                    "我可以为您解答各种HR相关问题：\n\n"
                                    "【专业服务】\n"
                                    "• 考勤制度和打卡规定\n"
                                    "• 请假流程和假期政策\n"
                                    "• 薪酬结构和发放时间\n"
                                    "• 培训机会和发展规划\n"
                                    "• 员工福利和保险政策\n"
                                    "• 各部门联系方式和办事流程\n\n"
                                    "点击右上角\"填表模式\"按钮可切换到智能填表功能\n\n"
                                    "请输入您想了解的问题，我会为您提供专业解答！";
                }
                firstBubble->setMessage(welcomeMessage);
            }
        }
    }
}

void AIAssistantWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    // 确保消息区域正确滚动
    QTimer::singleShot(50, this, &AIAssistantWidget::scrollToBottom);
}

// 侧边栏历史记录功能实现
void AIAssistantWidget::onSidebarToggled()
{
    sidebarVisible_ = !sidebarVisible_;
    sidebarWidget_->setVisible(sidebarVisible_);
    
    if (sidebarVisible_) {
        sidebarToggleButton_->setText("隐藏");
    } else {
        sidebarToggleButton_->setText("历史");
    }
}

void AIAssistantWidget::onHistoryItemClicked(QListWidgetItem* item)
{
    if (!item) return;
    
    QString question = item->text();
    inputEdit_->setPlainText(question);
    inputEdit_->setFocus();
}

void AIAssistantWidget::onClearHistoryClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认清空", 
        "您确定要清空所有历史记录吗？",
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        historyList_->clear();
        saveHistory();
    }
}

void AIAssistantWidget::onHistorySearchChanged()
{
    QString keyword = historySearchEdit_->text().trimmed();
    filterHistory(keyword);
}

void AIAssistantWidget::addToHistory(const QString &question, const QString &answer)
{
    // 添加到历史记录列表
    QListWidgetItem* item = new QListWidgetItem(question);
    item->setToolTip(QString("问题: %1\n回答: %2\n时间: %3")
                    .arg(question)
                    .arg(answer.left(100) + (answer.length() > 100 ? "..." : ""))
                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm")));
    
    historyList_->insertItem(0, item);
    
    // 限制历史记录数量
    while (historyList_->count() > 50) {
        delete historyList_->takeItem(historyList_->count() - 1);
    }
    
    saveHistory();
}

void AIAssistantWidget::loadHistory()
{
    // 这里可以从文件或数据库加载历史记录
    // 简化实现：暂时不持久化
}

void AIAssistantWidget::saveHistory()
{
    // 这里可以保存历史记录到文件或数据库
    // 简化实现：暂时不持久化
}

void AIAssistantWidget::filterHistory(const QString &keyword)
{
    for (int i = 0; i < historyList_->count(); ++i) {
        QListWidgetItem* item = historyList_->item(i);
        bool matches = keyword.isEmpty() || 
                      item->text().contains(keyword, Qt::CaseInsensitive) ||
                      item->toolTip().contains(keyword, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

// ===== 自动填表功能实现 =====

void AIAssistantWidget::processFormInput(const QString &input)
{
    if (!formFiller_) {
        // 在填表模式下如果formFiller_不可用，显示错误消息而不回退到问答模式
        addSystemMessage("智能填表功能暂时不可用，请稍后重试");
        enableInput(true);
        isProcessing_ = false;
        return;
    }
    
    QString responseMessage;
    FormTemplate* newForm = formFiller_->processUserInput(input, responseMessage);
    
    if (newForm) {
        // 更新当前表单
        if (currentForm_) {
            delete currentForm_;
        }
        currentForm_ = newForm;
        formMode_ = true;
        
        // 调试信息：检查新表单的数据
        qDebug() << "[表单创建] 新表单名称:" << currentForm_->getName();
        auto allValues = currentForm_->getAllValues();
        qDebug() << "[表单创建] 表单数据数量:" << allValues.size();
        for (auto it = allValues.begin(); it != allValues.end(); ++it) {
            qDebug() << "[表单创建] 字段" << it.key() << "=" << it.value().toString();
        }
        
        // 额外测试：立即尝试生成HTML来验证数据
        QString testHtml = currentForm_->toHtml();
        qDebug() << "[表单创建] 测试HTML生成长度:" << testHtml.length();
        
        // 显示响应消息
        addFormResponseMessage(responseMessage);
        
        // 更新状态
        statusLabel_->setText("表单模式");
        statusLabel_->setStyleSheet(
            "color: #059669; background: #d1fae5; "
            "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
        );
        
        // 保存到历史记录
        addToHistory(currentQuestion_, responseMessage);
        currentQuestion_.clear();
        
    } else if (hasActiveForm()) {
        // 如果有活动表单，尝试更新它
        NLPProcessor processor;
        QList<ExtractionResult> extractions = processor.extractFieldValues(input, currentForm_);
        
        if (!extractions.isEmpty()) {
            // 更新表单字段
            qDebug() << "[表单更新] 提取到" << extractions.size() << "个字段";
            for (const ExtractionResult& extraction : extractions) {
                qDebug() << "[表单更新] 设置字段:" << extraction.fieldId << "=" << extraction.value.toString();
                currentForm_->setFieldValue(extraction.fieldId, extraction.value);
            }
            
            // 生成更新响应
            FormRecognitionResult recognition;
            recognition.formName = currentForm_->getName();
            recognition.confidence = 0.8;
            responseMessage = formFiller_->generateResponseMessage(recognition, currentForm_);
            
            addFormResponseMessage("表单信息已更新！\n\n" + responseMessage);
            
            // 保存到历史记录
            addToHistory(currentQuestion_, "表单信息已更新");
            currentQuestion_.clear();
        } else {
            // 没有提取到新信息，在填表模式下提示用户
            addFormResponseMessage("抱歉，我无法从您的输入中提取到有用的表单信息。请尝试更详细地描述您需要填写的内容，例如：\n\n• \"我是张三，工号A001，在技术部工作\"\n• \"我要请假，明天开始请3天病假\"");
            enableInput(true);
            isProcessing_ = false;
        }
    } else {
        // 没有表单，在填表模式下提示用户创建新表单
        addFormResponseMessage("请描述您需要填写的表单信息，我会智能识别并帮您创建相应的表单。\n\n【支持的表单类型】\n• 员工信息表：\"我是张三，工号A001，在技术部工作\"\n• 请假申请表：\"我要请假，明天开始请3天病假\"");
        enableInput(true);
        isProcessing_ = false;
    }
}

bool AIAssistantWidget::isFormCommand(const QString &input)
{
    QString lowerInput = input.toLower();
    QStringList formCommands = {
        "导出表单", "导出pdf", "导出html", "导出csv", "导出txt",
        "查看表单", "显示表单", "预览表单", "表单内容",
        "清空表单", "删除表单", "重置表单", "新建表单"
    };
    
    for (const QString& command : formCommands) {
        if (lowerInput.contains(command)) {
            return true;
        }
    }
    
    return false;
}

void AIAssistantWidget::handleFormCommand(const QString &command)
{
    QString lowerCommand = command.toLower();
    
    if (lowerCommand.contains("导出")) {
        if (!hasActiveForm()) {
            addSystemMessage("当前没有活动的表单可以导出。请先创建一个表单。");
            return;
        }
        
        ExportFormat format = ExportFormat::PDF; // 默认PDF
        if (lowerCommand.contains("html")) format = ExportFormat::HTML;
        else if (lowerCommand.contains("csv")) format = ExportFormat::CSV;
        else if (lowerCommand.contains("txt")) format = ExportFormat::TXT;
        
        exportCurrentForm(format);
    }
    else if (lowerCommand.contains("查看") || lowerCommand.contains("显示") || lowerCommand.contains("预览")) {
        if (!hasActiveForm()) {
            addSystemMessage("当前没有活动的表单可以查看。请先创建一个表单。");
            return;
        }
        
        showFormPreview(currentForm_);
    }
    else if (lowerCommand.contains("清空") || lowerCommand.contains("删除") || lowerCommand.contains("重置")) {
        if (hasActiveForm()) {
            clearActiveForm();
            addSystemMessage("表单已清空，您可以重新开始填写新表单。");
        } else {
            addSystemMessage("当前没有活动的表单。您可以直接开始描述需要填写的表单信息。");
        }
    }
    else if (lowerCommand.contains("新建")) {
        clearActiveForm();
        addSystemMessage("已准备创建新表单。请描述您需要填写的表单信息，例如：\n\n"
                        "• 员工信息：\"我是新员工张三，工号A001，在技术部工作\"\n"
                        "• 请假申请：\"我要请假，从明天开始请3天病假\"");
    }
    
    // 保存到历史记录
    addToHistory(currentQuestion_, "表单操作已执行");
    currentQuestion_.clear();
}

void AIAssistantWidget::showFormPreview(FormTemplate* form)
{
    if (!form || !formPreviewer_) {
        addSystemMessage("无法预览表单");
        return;
    }
    
    QString previewText = formPreviewer_->generatePreviewText(form);
    QString message = QString("当前表单预览：\n\n%1\n\n"
                             "可用操作：\n"
                             "• 继续输入信息补充表单\n"
                             "• 输入\"导出表单\"生成文件\n"
                             "• 输入\"清空表单\"重新开始").arg(previewText);
    
    addFormResponseMessage(message);
}

void AIAssistantWidget::exportCurrentForm(ExportFormat format)
{
    if (!hasActiveForm() || !formExporter_) {
        addSystemMessage("导出失败：没有可用的表单");
        return;
    }
    
    // 添加调试信息，检查表单数据
    qDebug() << "[导出] 当前表单名称:" << currentForm_->getName();
    auto allValues = currentForm_->getAllValues();
    qDebug() << "[导出] 表单数据数量:" << allValues.size();
    for (auto it = allValues.begin(); it != allValues.end(); ++it) {
        qDebug() << "[导出] 字段" << it.key() << "=" << it.value().toString();
    }
    
    ExportResult result = formExporter_->exportForm(currentForm_, format);
    
    if (result.success) {
        QString formatName = FormExporter::getFormatName(format);
        QString message = QString("表单导出成功！\n\n"
                                 "文件格式：%1\n"
                                 "文件大小：%2\n"
                                 "保存位置：%3\n\n"
                                 "文件已自动保存到桌面，您可以直接打开查看。")
                         .arg(formatName, result.fileSize, result.filePath);
        
        addFormResponseMessage(message);
        
        // 保存到历史记录
        addToHistory("导出表单", QString("表单已导出为%1格式").arg(formatName));
    } else {
        QString message = QString("导出失败\n\n错误信息：%1\n\n"
                                 "请稍后重试或联系技术支持。").arg(result.errorMessage);
        addSystemMessage(message);
    }
}

void AIAssistantWidget::addFormResponseMessage(const QString &message)
{
    // 使用AI消息气泡显示表单响应
    ChatBubble* bubble = new ChatBubble(MessageType::BotMessage, message);
    bubble->setTimestamp(QDateTime::currentDateTime());
    bubble->setCategory(isFormModeEnabled_ ? "表单助手" : "AI助手回复");
    
    // 创建AI头像
    QPixmap aiAvatar(36, 36);
    aiAvatar.fill(QColor(16, 185, 129)); // 绿色背景
    QPainter painter(&aiAvatar);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Microsoft YaHei", 12, QFont::Bold));
    painter.drawText(aiAvatar.rect(), Qt::AlignCenter, "AI");
    bubble->setAvatar(aiAvatar);
    
    // 连接复制信号
    connect(bubble, &ChatBubble::copyRequested, [message](const QString& text) {
        Q_UNUSED(text)
        QApplication::clipboard()->setText(message);
    });
    
    // 添加到布局并显示动画
    messagesLayout_->insertWidget(messagesLayout_->count() - 1, bubble);
    bubble->animateIn();
    scrollToBottom();
    messageCount_++;
}

bool AIAssistantWidget::hasActiveForm() const
{
    return currentForm_ != nullptr && formMode_;
}

void AIAssistantWidget::clearActiveForm()
{
    if (currentForm_) {
        delete currentForm_;
        currentForm_ = nullptr;
    }
    formMode_ = false;
    
    // 重置状态
    statusLabel_->setText("就绪");
    statusLabel_->setStyleSheet(
        "color: #059669; background: #d1fae5; "
        "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
    );
}

QString AIAssistantWidget::getFormStatusMessage()
{
    if (!hasActiveForm()) {
        return "当前没有活动的表单";
    }
    
    QString status = QString("当前表单：%1").arg(currentForm_->getName());
    QMap<QString, QVariant> values = currentForm_->getAllValues();
    QList<FormField> fields = currentForm_->getFields();
    
    int filledCount = 0;
    int requiredCount = 0;
    
    for (const FormField& field : fields) {
        if (field.required) {
            requiredCount++;
            if (values.contains(field.id) && !values[field.id].toString().isEmpty()) {
                filledCount++;
            }
        }
    }
    
    status += QString(" (%1/%2 必填项已完成)").arg(filledCount).arg(requiredCount);
    return status;
}

// 槽函数实现（预留接口）
void AIAssistantWidget::onAutoFormRequest(const QString& userInput)
{
    processFormInput(userInput);
}

void AIAssistantWidget::onExportFormRequest(const QString& format)
{
    ExportFormat exportFormat = ExportFormat::PDF;
    QString lowerFormat = format.toLower();
    
    if (lowerFormat.contains("html")) exportFormat = ExportFormat::HTML;
    else if (lowerFormat.contains("csv")) exportFormat = ExportFormat::CSV;
    else if (lowerFormat.contains("txt")) exportFormat = ExportFormat::TXT;
    
    exportCurrentForm(exportFormat);
}

void AIAssistantWidget::onViewFormRequest()
{
    if (hasActiveForm()) {
        showFormPreview(currentForm_);
    } else {
        addSystemMessage("当前没有活动的表单可以查看");
    }
}

void AIAssistantWidget::onClearFormRequest()
{
    clearActiveForm();
    addSystemMessage("表单已清空");
}

void AIAssistantWidget::onModeToggled()
{
    isFormModeEnabled_ = !isFormModeEnabled_;
    
    // 更新按钮文本和状态
    if (isFormModeEnabled_) {
        modeToggleButton_->setText("问答模式");
        modeToggleButton_->setToolTip("点击切换到AI问答模式");
        statusLabel_->setText("填表模式");
        statusLabel_->setStyleSheet(
            "color: #059669; background: #d1fae5; "
            "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
        );
    } else {
        modeToggleButton_->setText("填表模式");
        modeToggleButton_->setToolTip("点击切换到智能填表模式");
        clearActiveForm(); // 切换到问答模式时清空表单
        statusLabel_->setText("问答模式");
        statusLabel_->setStyleSheet(
            "color: #3b82f6; background: #dbeafe; "
            "padding: 3px 8px; border-radius: 8px; font-weight: 500;"
        );
    }
    
    // 更新现有的欢迎消息内容而不是重新显示
    updateWelcomeMessageForCurrentMode();
    
    // 只显示模式切换提示，不重新显示欢迎语
    addSystemMessage(QString("已切换到%1").arg(isFormModeEnabled_ ? "智能填表模式" : "AI问答模式"));
}

QString AIAssistantWidget::generateRelatedTerms(const QString& message)
{
    // 根据消息内容匹配相关的HR政策条款
    QStringList terms;
    QString lowerMessage = message.toLower();
    
    if (lowerMessage.contains("薪酬") || lowerMessage.contains("工资") || lowerMessage.contains("薪资")) {
        terms << "《员工薪酬管理制度》第3条：基本工资+绩效奖金+各项津贴";
        terms << "《薪酬发放规定》：每月15日发放，遇节假日提前";
        terms << "《绩效考核办法》：根据KPI考核结果确定绩效奖金";
    }
    
    if (lowerMessage.contains("请假") || lowerMessage.contains("假期") || lowerMessage.contains("休假")) {
        terms << "《员工请假管理办法》：提前3天申请，需主管审批";
        terms << "《年假制度》：连续工作满1年可享受带薪年假";
        terms << "《病假规定》：需提供医院证明，工资按比例发放";
    }
    
    if (lowerMessage.contains("考勤") || lowerMessage.contains("打卡") || lowerMessage.contains("上班")) {
        terms << "《考勤管理制度》：正常工作时间为9:00-18:00";
        terms << "《打卡规定》：须在规定时间内完成打卡，迟到早退有相应扣罚";
        terms << "《弹性工作制》：核心时间段必须在岗，其他时间可灵活安排";
    }
    
    if (lowerMessage.contains("培训") || lowerMessage.contains("学习") || lowerMessage.contains("发展")) {
        terms << "《员工培训发展制度》：公司提供多元化培训机会";
        terms << "《学习津贴政策》：支持员工参加外部培训课程";
        terms << "《职业发展规划》：每年进行职业发展评估与规划";
    }
    
    if (lowerMessage.contains("福利") || lowerMessage.contains("保险") || lowerMessage.contains("社保")) {
        terms << "《员工福利制度》：五险一金+商业保险+年度体检";
        terms << "《补充医疗保险》：覆盖员工及直系亲属";
        terms << "《企业年金计划》：为员工退休生活提供保障";
    }
    
    if (terms.isEmpty()) {
        // 默认显示通用政策
        terms << "《员工手册》：包含完整的公司制度和政策";
        terms << "《人事管理制度》：员工入职、在职、离职全流程规定";
        terms << "如需更详细咨询，请联系HR部门：内线8001";
    }
    
    return terms.join("\n");
}

void AIAssistantWidget::onShowRelatedClauses(const QString& question)
{
    // 生成相关条款内容
    QString relatedTerms = generateRelatedTerms(question);
    
    // 创建一个简单的对话框显示相关条款
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("相关条款");
    dialog->setFixedSize(600, 450);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    // 标题
    QLabel* titleLabel = new QLabel(QString("问题：%1").arg(question));
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px; color: #2c3e50; margin: 10px;");
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);
    
    // 内容区域
    QTextBrowser* contentBrowser = new QTextBrowser();
    contentBrowser->setHtml(QString(
        "<div style='font-family: Microsoft YaHei; line-height: 1.6;'>"
        "<h3 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 8px;'>相关条款</h3>"
        "%1"
        "</div>"
    ).arg(relatedTerms.replace("\n", "<br/>")));
    contentBrowser->setStyleSheet(
        "QTextBrowser {"
        "    background: white;"
        "    border: 1px solid #ddd;"
        "    border-radius: 8px;"
        "    padding: 15px;"
        "    font-size: 13px;"
        "}"
    );
    layout->addWidget(contentBrowser);
    
    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    QPushButton* copyButton = new QPushButton("复制条款");
    copyButton->setStyleSheet(
        "QPushButton {"
        "    background: #3498db;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #2980b9;"
        "}"
    );
    connect(copyButton, &QPushButton::clicked, [relatedTerms]() {
        QApplication::clipboard()->setText(relatedTerms);
    });
    
    QPushButton* closeButton = new QPushButton("关闭");
    closeButton->setStyleSheet(
        "QPushButton {"
        "    background: #95a5a6;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 6px;"
        "    padding: 8px 16px;"
        "    font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "    background: #7f8c8d;"
        "}"
    );
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::close);
    
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);
    
    dialog->exec();
    dialog->deleteLater();
}