#include "qawidget.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QScrollBar>
#include <QDesktopServices>
#include <QDateTime>
#include <QStringConverter>

QAWidget::QAWidget(QAEngine* qaEngine, Database* database, const User& currentUser, QWidget *parent)
    : QWidget(parent)
    , qaEngine_(qaEngine)
    , database_(database)
    , currentUser_(currentUser)
    , mainLayout_(nullptr)
    , mainSplitter_(nullptr)
    , leftPanel_(nullptr)
    , leftLayout_(nullptr)
    , welcomeLabel_(nullptr)
    , categoryCombo_(nullptr)
    , searchEdit_(nullptr)
    , searchButton_(nullptr)
    , quickQuestionsGroup_(nullptr)
    , quickQuestionsList_(nullptr)
    , historyGroup_(nullptr)
    , historyList_(nullptr)
    , clearHistoryButton_(nullptr)
    , rightPanel_(nullptr)
    , rightLayout_(nullptr)
    , questionGroup_(nullptr)
    , questionEdit_(nullptr)
    , askButton_(nullptr)
    , answerGroup_(nullptr)
    , answerBrowser_(nullptr)
    , confidenceLabel_(nullptr)
    , categoryLabel_(nullptr)
    , documentsGroup_(nullptr)
    , documentsBrowser_(nullptr)
    , answerAnimation_(nullptr)
    , answerOpacityEffect_(nullptr)
    , searchTimer_(nullptr)
    , currentHistory_()
{
    setupUI();
    setupConnections();
    applyStyles();
    loadQuickQuestions();
    loadHistory();
    setupAnimations();
}

QAWidget::~QAWidget()
{
}

void QAWidget::setupUI()
{
    // 创建现代化主布局
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(16, 16, 16, 16);
    mainLayout_->setSpacing(16);
    
    // 创建分割器
    mainSplitter_ = new QSplitter(Qt::Horizontal);
    mainSplitter_->setChildrenCollapsible(false);
    
    // 现代化左侧面板
    leftPanel_ = new QWidget();
    leftPanel_->setObjectName("cardContainer");
    leftPanel_->setMinimumWidth(320);
    leftPanel_->setMaximumWidth(420);
    leftLayout_ = new QVBoxLayout(leftPanel_);
    leftLayout_->setContentsMargins(20, 20, 20, 20);
    leftLayout_->setSpacing(20);
    
    // 现代化欢迎标签
    welcomeLabel_ = new QLabel(QString("欢迎，%1").arg(currentUser_.getName()));
    welcomeLabel_->setObjectName("sectionTitle");
    welcomeLabel_->setAlignment(Qt::AlignLeft);
    leftLayout_->addWidget(welcomeLabel_);
    
    // 类别选择
    QLabel* categoryLabel = new QLabel("问题类别");
    categoryLabel->setObjectName("sectionTitle");
    leftLayout_->addWidget(categoryLabel);
    
    categoryCombo_ = new QComboBox();
    categoryCombo_->addItem("全部类别", "");
    categoryCombo_->addItem("考勤管理", "考勤");
    categoryCombo_->addItem("薪酬管理", "薪酬");
    categoryCombo_->addItem("绩效管理", "绩效");
    categoryCombo_->addItem("入职离职", "入职离职");
    categoryCombo_->addItem("员工福利", "员工福利");
    leftLayout_->addWidget(categoryCombo_);
    
    // 搜索框
    QLabel* searchLabel = new QLabel("搜索历史");
    searchLabel->setObjectName("sectionTitle");
    leftLayout_->addWidget(searchLabel);
    
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("输入关键词搜索历史记录...");
    searchButton_ = new QPushButton("搜索");
    searchButton_->setObjectName("searchButton");
    searchButton_->setMinimumWidth(80);
    searchButton_->setMaximumWidth(100);
    searchLayout->addWidget(searchEdit_);
    searchLayout->addWidget(searchButton_);
    leftLayout_->addLayout(searchLayout);
    
    // 快速问题
    quickQuestionsGroup_ = new QGroupBox("常见问题");
    QVBoxLayout* quickLayout = new QVBoxLayout(quickQuestionsGroup_);
    quickQuestionsList_ = new QListWidget();
    quickQuestionsList_->setMaximumHeight(150);
    quickLayout->addWidget(quickQuestionsList_);
    leftLayout_->addWidget(quickQuestionsGroup_);
    
    // 历史记录
    historyGroup_ = new QGroupBox("问答历史");
    QVBoxLayout* historyLayout = new QVBoxLayout(historyGroup_);
    historyList_ = new QListWidget();
    historyLayout->addWidget(historyList_);
    
    QHBoxLayout* historyButtonLayout = new QHBoxLayout();
    clearHistoryButton_ = new QPushButton("清空历史");
    historyButtonLayout->addWidget(clearHistoryButton_);
    historyButtonLayout->addStretch();
    historyLayout->addLayout(historyButtonLayout);
    
    leftLayout_->addWidget(historyGroup_);
    
    // 现代化右侧面板
    rightPanel_ = new QWidget();
    rightLayout_ = new QVBoxLayout(rightPanel_);
    rightLayout_->setContentsMargins(16, 16, 16, 16);
    rightLayout_->setSpacing(16);
    
    // 问题输入卡片
    QWidget* questionCard = new QWidget();
    questionCard->setObjectName("cardContainer");
    QVBoxLayout* questionCardLayout = new QVBoxLayout(questionCard);
    questionCardLayout->setContentsMargins(20, 20, 20, 20);
    questionCardLayout->setSpacing(16);
    
    QLabel* questionTitle = new QLabel("提问区域");
    questionTitle->setObjectName("sectionTitle");
    questionCardLayout->addWidget(questionTitle);
    
    questionEdit_ = new QTextEdit();
    questionEdit_->setPlaceholderText("请输入您想咨询的HR制度相关问题...\n\n例如：\n• 请假需要提前多久申请？\n• 社保缴费比例是多少？\n• 绩效考核周期是什么？");
    questionEdit_->setMaximumHeight(120);
    questionCardLayout->addWidget(questionEdit_);
    
    askButton_ = new QPushButton("立即提问");
    askButton_->setMinimumHeight(48);
    questionCardLayout->addWidget(askButton_);
    
    rightLayout_->addWidget(questionCard);
    
    // 答案显示卡片
    QWidget* answerCard = new QWidget();
    answerCard->setObjectName("cardContainer");
    QVBoxLayout* answerCardLayout = new QVBoxLayout(answerCard);
    answerCardLayout->setContentsMargins(20, 20, 20, 20);
    answerCardLayout->setSpacing(16);
    
    QLabel* answerTitle = new QLabel("AI智能回答");
    answerTitle->setObjectName("sectionTitle");
    answerCardLayout->addWidget(answerTitle);
    
    // 创建答案信息栏
    QHBoxLayout* answerInfoLayout = new QHBoxLayout();
    confidenceLabel_ = new QLabel();
    categoryLabel_ = new QLabel();
    answerInfoLayout->addWidget(confidenceLabel_);
    answerInfoLayout->addStretch();
    answerInfoLayout->addWidget(categoryLabel_);
    answerCardLayout->addLayout(answerInfoLayout);
    
    answerBrowser_ = new QTextBrowser();
    answerBrowser_->setPlaceholderText("在上方输入问题后，点击提问按钮，AI将为您提供智能回答。");
    answerCardLayout->addWidget(answerBrowser_);
    
    rightLayout_->addWidget(answerCard);
    
    // 相关文档卡片
    QWidget* documentsCard = new QWidget();
    documentsCard->setObjectName("cardContainer");
    QVBoxLayout* documentsCardLayout = new QVBoxLayout(documentsCard);
    documentsCardLayout->setContentsMargins(20, 20, 20, 20);
    documentsCardLayout->setSpacing(16);
    
    QLabel* documentsTitle = new QLabel("相关文档");
    documentsTitle->setObjectName("sectionTitle");
    documentsCardLayout->addWidget(documentsTitle);
    
    documentsBrowser_ = new QTextBrowser();
    documentsBrowser_->setPlaceholderText("相关的HR制度文档将在这里显示，点击可查看详细内容。");
    documentsBrowser_->setMaximumHeight(150);
    documentsCardLayout->addWidget(documentsBrowser_);
    
    rightLayout_->addWidget(documentsCard);
    
    // 添加到分割器
    mainSplitter_->addWidget(leftPanel_);
    mainSplitter_->addWidget(rightPanel_);
    mainSplitter_->setStretchFactor(0, 0);
    mainSplitter_->setStretchFactor(1, 1);
    
    mainLayout_->addWidget(mainSplitter_);
}

void QAWidget::setupConnections()
{
    connect(askButton_, &QPushButton::clicked, this, &QAWidget::onQuestionSubmitted);
    connect(searchButton_, &QPushButton::clicked, [this]() { 
        this->onSearchTextChanged(searchEdit_->text()); 
    });
    connect(clearHistoryButton_, &QPushButton::clicked, this, &QAWidget::clearHistory);
    
    // 连接AI问答信号
    connect(qaEngine_, &QAEngine::aiAnswerReceived, this, &QAWidget::onAIAnswerReceived);

    
    connect(quickQuestionsList_, &QListWidget::itemClicked, this, &QAWidget::onQuickQuestionClicked);
    connect(historyList_, &QListWidget::itemClicked, this, &QAWidget::onHistoryItemClicked);
    
    connect(categoryCombo_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),
            this, &QAWidget::onCategoryChanged);
    connect(searchEdit_, &QLineEdit::textChanged, this, &QAWidget::onSearchTextChanged);
    
    connect(documentsBrowser_, static_cast<void(QTextBrowser::*)(const QUrl&)>(&QTextBrowser::anchorClicked), 
            [this](const QUrl& url) { this->onDocumentLinkClicked(url.toString()); });
    
    // 回车键提问
    connect(questionEdit_, &QTextEdit::textChanged, [this]() {
        askButton_->setEnabled(!questionEdit_->toPlainText().trimmed().isEmpty());
    });
}

void QAWidget::applyStyles()
{
    setStyleSheet(
        /* 主窗口背景 */
        "QWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f8f9fa, stop:1 #e9ecef);"
        "    font-family: 'Microsoft YaHei', 'Segoe UI', Arial;"
        "}"
        
        /* 组框样式 */
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #e3f2fd;"
        "    border-radius: 12px;"
        "    margin-top: 15px;"
        "    padding-top: 20px;"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #f8f9fa);"
        "    box-shadow: 0 2px 8px rgba(0,0,0,0.1);"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 5px 12px;"
        "    color: #1976d2;"
        "    font-size: 15px;"
        "    font-weight: bold;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e3f2fd, stop:1 #bbdefb);"
        "    border-radius: 8px;"
        "    border: 1px solid #2196f3;"
        "}"
        
        /* 列表控件样式 */
        "QListWidget {"
        "    border: 2px solid #e3f2fd;"
        "    border-radius: 10px;"
        "    background: white;"
        "    padding: 8px;"
        "    font-size: 14px;"
        "    selection-background-color: #2196f3;"
        "    selection-color: white;"
        "}"
        
        "QListWidget::item {"
        "    border-radius: 6px;"
        "    padding: 10px 12px;"
        "    margin: 3px 0px;"
        "    background: #fafafa;"
        "    border: 1px solid transparent;"
        "}"
        
        "QListWidget::item:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #e3f2fd, stop:1 #bbdefb);"
        "    border: 1px solid #2196f3;"
        "    transform: translateY(-1px);"
        "}"
        
        "QListWidget::item:selected {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2196f3, stop:1 #1976d2);"
        "    color: white;"
        "    border: 1px solid #1565c0;"
        "}"
        
        /* 文本编辑器样式 */
        "QTextEdit, QTextBrowser {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 6px;"
        "    background: white;"
        "    padding: 10px;"
        "    font-size: 13px;"
        "    selection-background-color: #2196f3;"
        "    selection-color: white;"
        "}"
        
        "QTextEdit:focus, QTextBrowser:focus {"
        "    border: 1px solid #2196f3;"
        "}"
        
        /* 输入框样式 */
        "QLineEdit {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 4px;"
        "    padding: 8px 10px;"
        "    background: white;"
        "    font-size: 13px;"
        "    selection-background-color: #2196f3;"
        "    selection-color: white;"
        "}"
        
        "QLineEdit:focus {"
        "    border: 1px solid #2196f3;"
        "}"
        
        /* 下拉框样式 */
        "QComboBox {"
        "    border: 1px solid #d0d0d0;"
        "    border-radius: 4px;"
        "    padding: 6px 10px;"
        "    background: white;"
        "    font-size: 13px;"
        "    min-width: 100px;"
        "    min-height: 20px;"
        "}"
        
        "QComboBox:focus {"
        "    border: 1px solid #2196f3;"
        "}"
        
        "QComboBox::drop-down {"
        "    subcontrol-origin: padding;"
        "    subcontrol-position: top right;"
        "    width: 20px;"
        "    border-left-width: 0px;"
        "    border-top-right-radius: 4px;"
        "    border-bottom-right-radius: 4px;"
        "}"
        
        /* 按钮样式 */
        "QPushButton {"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 10px 20px;"
        "    font-weight: bold;"
        "    color: white;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2196f3, stop:1 #1976d2);"
        "    font-size: 14px;"
        "    min-height: 30px;"
        "    box-shadow: 0 2px 4px rgba(33, 150, 243, 0.3);"
        "}"
        
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1976d2, stop:1 #1565c0);"
        "    box-shadow: 0 4px 8px rgba(33, 150, 243, 0.4);"
        "    transform: translateY(-1px);"
        "}"
        
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #1565c0, stop:1 #0d47a1);"
        "    box-shadow: 0 1px 2px rgba(33, 150, 243, 0.3);"
        "    transform: translateY(0px);"
        "}"
        
        "QPushButton:disabled {"
        "    background: #bdbdbd;"
        "    color: #757575;"
        "    box-shadow: none;"
        "}"
        
        "QPushButton#askButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4facfe, stop:1 #00f2fe);"
        "    color: white;"
        "    font-weight: bold;"
        "    border: none;"
        "}"
        
        "QPushButton#askButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #5fb8ff, stop:1 #20f7ff);"
        "}"
        
        "QPushButton#askButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4590e7, stop:1 #00d8e3);"
        "}"
        
        "QPushButton#searchButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #3498db, stop:1 #2980b9);"
        "    color: white !important;"
        "    font-size: 14px;"
        "    font-weight: bold;"
        "    border: 1px solid #2980b9;"
        "}"
        
        "QPushButton#searchButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #5dade2, stop:1 #3498db);"
        "    color: white !important;"
        "}"
        
        "QLabel#welcomeLabel_ {"
        "    background-color: rgba(79, 172, 254, 0.1);"
        "    border-radius: 10px;"
        "    padding: 10px;"
        "}"
    );
}

void QAWidget::loadQuickQuestions()
{
    QStringList quickQuestions = {
        "公司的工作时间是什么？",
        "如何申请请假？",
        "社保缴费比例是多少？",
        "绩效考核周期是什么？",
        "新员工试用期多长？",
        "年假天数如何计算？",
        "加班费如何计算？",
        "入职需要什么材料？"
    };
    
    for (const QString& question : quickQuestions) {
        QListWidgetItem* item = new QListWidgetItem(question);
        item->setToolTip("点击快速提问");
        quickQuestionsList_->addItem(item);
    }
}

void QAWidget::loadHistory()
{
    // 从数据库加载历史记录
    QList<QuestionRecord> records = database_->getQuestionRecords(currentUser_.getEmployeeId());
    
    historyList_->clear();
    currentHistory_.clear();
    
    // 限制显示的历史记录数量以提高性能
    const int maxResults = 50;
    int resultCount = 0;
    
    for (const QuestionRecord& record : records) {
        if (resultCount >= maxResults) {
            break; // 限制记录数量
        }
        
        QString displayText = record.getQuestion();
        if (displayText.length() > 50) {
            displayText = displayText.left(50) + "...";
        }
        
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setToolTip(QString("问题: %1\n类别: %2\n时间: %3")
                         .arg(record.getQuestion())
                         .arg(record.getCategory())
                         .arg(record.getCreateTime().toString("yyyy-MM-dd hh:mm")));
        historyList_->addItem(item);
        
        // 构造QAResult用于历史显示
        QAResult result;
        result.answer = record.getAnswer();
        result.category = record.getCategory();
        result.confidence = "历史记录";
        currentHistory_.append(result);
        
        resultCount++;
    }
}

void QAWidget::onQuestionSubmitted()
{
    QString question = questionEdit_->toPlainText().trimmed();
    if (question.isEmpty()) {
        QMessageBox::information(this, "提示", "请输入问题后再提问。");
        return;
    }
    
    // 禁用提问按钮防止重复提交
    askButton_->setEnabled(false);
    askButton_->setText("AI分析中...");
    
    // 使用QApplication::processEvents()确保UI响应
    QApplication::processEvents();
    
    try {
        if (qaEngine_->isAIEnabled()) {
            // 使用AI增强问答
            qaEngine_->askQuestionWithAI(question);
            // AI模式下，结果将通过信号异步返回，不在这里恢复按钮
        } else {
            // 使用传统问答
            QAResult result = qaEngine_->askQuestion(question);
            result.isAIGenerated = false;
            
            // 记录到数据库
            QuestionRecord record(currentUser_.getEmployeeId(), question, result.answer, result.category);
            database_->addQuestionRecord(record);
            
            // 显示答案
            displayAnswer(result);
            
            // 显示相关文档
            displayRelatedDocuments(result.relatedDocuments);
            
            // 更新历史记录
            addToHistory(question, result);
            
            // 清空问题输入框
            questionEdit_->clear();
            
            // 发出信号
            emit questionAsked(question, result);
            
            // 恢复提问按钮
            askButton_->setEnabled(true);
            askButton_->setText("提问");
        }
        
    } catch (...) {
        // 处理异常
        QAResult errorResult;
        errorResult.answer = "抱歉，处理您的问题时发生错误，请稍后重试。";
        errorResult.category = "错误";
        errorResult.confidence = "错误";
        displayAnswer(errorResult);
        
        // 恢复提问按钮
        askButton_->setEnabled(true);
        askButton_->setText("提问");
    }
}

void QAWidget::onQuickQuestionClicked(QListWidgetItem* item)
{
    if (item) {
        questionEdit_->setPlainText(item->text());
        onQuestionSubmitted();
    }
}

void QAWidget::onHistoryItemClicked(QListWidgetItem* item)
{
    int row = historyList_->row(item);
    if (row >= 0 && row < currentHistory_.size()) {
        const QAResult& result = currentHistory_[row];
        displayAnswer(result);
        displayRelatedDocuments(result.relatedDocuments);
    }
}

void QAWidget::onCategoryChanged(const QString& category)
{
    Q_UNUSED(category)
    // 根据类别过滤历史记录
    QString categoryData = categoryCombo_->currentData().toString();
    
    historyList_->clear();
    
    QList<QuestionRecord> records;
    if (categoryData.isEmpty()) {
        records = database_->getQuestionRecords(currentUser_.getEmployeeId());
    } else {
        records = database_->getQuestionRecordsByCategory(categoryData);
    }
    
    currentHistory_.clear();
    
    int resultCount = 0;
    const int maxResults = 50;
    for (const QuestionRecord& record : records) {
        if (record.getEmployeeId() == currentUser_.getEmployeeId() || 
            currentUser_.isHRAdmin()) {
            QListWidgetItem* item = new QListWidgetItem(record.getQuestion());
            item->setToolTip(QString("类别: %1\n时间: %2")
                             .arg(record.getCategory())
                             .arg(record.getCreateTime().toString("yyyy-MM-dd hh:mm")));
            historyList_->addItem(item);
            
            // 构造QAResult用于历史显示
            QAResult result;
            result.answer = record.getAnswer();
            result.category = record.getCategory();
            result.confidence = "历史记录";
            currentHistory_.append(result);
            
            // 限制结果数量
            resultCount++;
            if (resultCount >= maxResults) {
                break;
            }
        }
    }
}

void QAWidget::onSearchTextChanged(const QString& text)
{
    // 使用延迟搜索，避免频繁搜索导致卡顿
    if (!searchTimer_) {
        searchTimer_ = new QTimer(this);
        searchTimer_->setSingleShot(true);
        connect(searchTimer_, &QTimer::timeout, this, &QAWidget::performSearch);
    }
    
    // 保存当前搜索文本
    currentSearchText_ = text;
    
    // 如果文本为空，立即加载历史记录
    if (text.isEmpty()) {
        searchTimer_->stop();
        loadHistory();
        return;
    }
    
    // 否则，延迟300毫秒执行搜索，避免用户快速输入时频繁搜索
    searchTimer_->start(300);
}

void QAWidget::performSearch()
{
    // 搜索历史记录
    historyList_->clear();
    currentHistory_.clear();
    
    // 如果搜索文本为空，重新加载历史记录
    if (currentSearchText_.isEmpty()) {
        loadHistory();
        return;
    }
    
    // 显示搜索中状态
    QListWidgetItem* searchingItem = new QListWidgetItem("正在搜索...");
    searchingItem->setFlags(Qt::NoItemFlags);
    searchingItem->setTextAlignment(Qt::AlignCenter);
    historyList_->addItem(searchingItem);
    
    // 使用QApplication::processEvents()确保UI更新
    QApplication::processEvents();
    
    // 获取记录并搜索（限制数量以提高性能）
    QList<QuestionRecord> allRecords = database_->getQuestionRecords(currentUser_.getEmployeeId());
    
    // 清除"正在搜索..."项
    historyList_->clear();
    
    // 限制搜索结果数量，提高性能
    int resultCount = 0;
    const int maxResults = 20; // 减少最大结果数
    
    // 优化搜索算法，只搜索最近的记录
    for (int i = allRecords.size() - 1; i >= 0 && resultCount < maxResults; --i) {
        const QuestionRecord& record = allRecords[i];
        
        // 使用更高效的搜索
        bool matches = false;
        if (currentSearchText_.length() >= 2) { // 至少2个字符才搜索
            matches = record.getQuestion().contains(currentSearchText_, Qt::CaseInsensitive) ||
                     record.getAnswer().contains(currentSearchText_, Qt::CaseInsensitive);
        }
        
        if (matches) {
            QString displayText = record.getQuestion();
            if (displayText.length() > 50) {
                displayText = displayText.left(50) + "...";
            }
            
            QListWidgetItem* item = new QListWidgetItem(displayText);
            item->setToolTip(QString("问题: %1\n类别: %2\n时间: %3")
                             .arg(record.getQuestion())
                             .arg(record.getCategory())
                             .arg(record.getCreateTime().toString("yyyy-MM-dd hh:mm")));
            historyList_->addItem(item);
            
            QAResult result;
            result.answer = record.getAnswer();
            result.category = record.getCategory();
            result.confidence = "历史记录";
            currentHistory_.append(result);
            
            resultCount++;
        }
        
        // 每处理10条记录，更新一次UI，避免界面卡死
        if (i % 10 == 0) {
            QApplication::processEvents();
        }
    }
    
    // 如果没有找到结果
    if (resultCount == 0) {
        QListWidgetItem* noResultItem = new QListWidgetItem("没有找到相关记录");
        noResultItem->setFlags(Qt::NoItemFlags);
        noResultItem->setTextAlignment(Qt::AlignCenter);
        historyList_->addItem(noResultItem);
    }
}

void QAWidget::onDocumentLinkClicked(const QString& link)
{
    // 处理文档链接点击
    if (link.startsWith("doc://")) {
        int docId = link.mid(6).toInt();
        Document doc = database_->getDocument(docId);
        if (doc.getId() > 0) {
            // 创建文档查看对话框
            QDialog* docDialog = new QDialog(this);
            docDialog->setWindowTitle(QString("查看文档 - %1").arg(doc.getTitle()));
            docDialog->setModal(true);
            docDialog->resize(800, 600);
            
            QVBoxLayout* layout = new QVBoxLayout(docDialog);
            
            // 文档标题
            QLabel* titleLabel = new QLabel(doc.getTitle());
            titleLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #2c3e50; padding: 10px;");
            titleLabel->setAlignment(Qt::AlignCenter);
            layout->addWidget(titleLabel);
            
            // 文档信息
            QLabel* infoLabel = new QLabel(QString("类别: %1 | 创建时间: %2")
                .arg(doc.getCategory())
                .arg(doc.getCreateTime().toString("yyyy-MM-dd hh:mm:ss")));
            infoLabel->setStyleSheet("color: #7f8c8d; padding: 5px;");
            infoLabel->setAlignment(Qt::AlignCenter);
            layout->addWidget(infoLabel);
            
            // 分隔线
            QFrame* line = new QFrame();
            line->setFrameShape(QFrame::HLine);
            line->setFrameShadow(QFrame::Sunken);
            layout->addWidget(line);
            
            // 文档内容
            QTextBrowser* contentBrowser = new QTextBrowser();
            contentBrowser->setHtml(QString(
                "<div style='font-family: \"Microsoft YaHei\", Arial, sans-serif; font-size: 14px; line-height: 1.6; padding: 20px;'>"
                "<pre style='white-space: pre-wrap; word-wrap: break-word;'>%1</pre>"
                "</div>"
            ).arg(doc.getContent().toHtmlEscaped()));
            layout->addWidget(contentBrowser);
            
            // 关闭按钮
            QPushButton* closeButton = new QPushButton("关闭");
            closeButton->setStyleSheet(
                "QPushButton { background-color: #3498db; color: white; border: none; padding: 10px 20px; "
                "border-radius: 5px; font-size: 14px; }"
                "QPushButton:hover { background-color: #2980b9; }"
            );
            connect(closeButton, &QPushButton::clicked, docDialog, &QDialog::accept);
            
            QHBoxLayout* buttonLayout = new QHBoxLayout();
            buttonLayout->addStretch();
            buttonLayout->addWidget(closeButton);
            buttonLayout->addStretch();
            layout->addLayout(buttonLayout);
            
            docDialog->exec();
            docDialog->deleteLater();
        }
    } else {
        QDesktopServices::openUrl(QUrl(link));
    }
}

void QAWidget::displayAnswer(const QAResult& result)
{
    QString html = formatAnswer(result);
    answerBrowser_->setHtml(html);
    
    updateConfidenceLabel(result.confidence);
    updateCategoryLabel(result.category);
    
    animateAnswer();
}

void QAWidget::displayRelatedDocuments(const QList<Document>& documents)
{
    QString html = formatDocumentList(documents);
    documentsBrowser_->setHtml(html);
}

void QAWidget::addToHistory(const QString& question, const QAResult& result)
{
    // 添加到历史列表顶部
    QListWidgetItem* item = new QListWidgetItem(question);
    item->setToolTip(QString("类别: %1\n置信度: %2")
                     .arg(result.category)
                     .arg(result.confidence));
    historyList_->insertItem(0, item);
    currentHistory_.prepend(result);
    
    // 限制历史记录数量
    while (historyList_->count() > 50) {
        delete historyList_->takeItem(historyList_->count() - 1);
        currentHistory_.removeLast();
    }
}

void QAWidget::updateConfidenceLabel(const QString& confidence)
{
    confidenceLabel_->setText(QString("置信度: %1").arg(confidence));
    
    if (confidence == "高") {
        confidenceLabel_->setStyleSheet("color: #27ae60; font-weight: bold;");
    } else if (confidence == "中等") {
        confidenceLabel_->setStyleSheet("color: #f39c12; font-weight: bold;");
    } else {
        confidenceLabel_->setStyleSheet("color: #e74c3c; font-weight: bold;");
    }
}

void QAWidget::updateCategoryLabel(const QString& category)
{
    categoryLabel_->setText(QString("类别: %1").arg(category));
    categoryLabel_->setStyleSheet("color: #3498db; font-weight: bold;");
}

void QAWidget::showLoadingAnimation()
{
    askButton_->setEnabled(false);
    askButton_->setText("AI分析中...");
    answerBrowser_->setHtml("<p style='text-align: center; color: #3498db;'>正在分析您的问题，请稍候...</p>");
}

void QAWidget::hideLoadingAnimation()
{
    askButton_->setEnabled(true);
    askButton_->setText("提问");
}

void QAWidget::animateAnswer()
{
    if (answerOpacityEffect_) {
        answerOpacityEffect_->setOpacity(0.0);
        answerAnimation_->setStartValue(0.0);
        answerAnimation_->setEndValue(1.0);
        answerAnimation_->start();
    }
}

void QAWidget::setupAnimations()
{
    // 创建答案显示动画
    answerOpacityEffect_ = new QGraphicsOpacityEffect(this);
    answerBrowser_->setGraphicsEffect(answerOpacityEffect_);
    
    answerAnimation_ = new QPropertyAnimation(answerOpacityEffect_, "opacity", this);
    answerAnimation_->setDuration(500);
    answerAnimation_->setEasingCurve(QEasingCurve::InOutQuad);
}

QString QAWidget::formatAnswer(const QAResult& result)
{
    QString html = QString(
        "<div style='font-family: \"Microsoft YaHei\", Arial, sans-serif; font-size: 14px; line-height: 1.6;'>"
        "<div style='background: #f8f9fa; padding: 15px; border-radius: 8px; margin-bottom: 10px;'>"
        "<h3 style='color: #2c3e50; margin-top: 0;'>智能回答</h3>"
        "<p style='margin-bottom: 0; color: #34495e;'>%1</p>"
        "</div>"
        "</div>"
    ).arg(result.answer);
    
    return html;
}

QString QAWidget::formatDocumentList(const QList<Document>& documents)
{
    if (documents.isEmpty()) {
        return "<p style='color: #7f8c8d; text-align: center;'>暂无相关文档</p>";
    }
    
    QString html = "<div style='font-family: \"Microsoft YaHei\", Arial, sans-serif;'>";
    
    for (int i = 0; i < documents.size() && i < 3; ++i) {
        const Document& doc = documents[i];
        html += QString(
            "<div style='border: 1px solid #e1e8ed; border-radius: 6px; padding: 10px; margin-bottom: 8px;'>"
            "<h4 style='margin: 0 0 5px 0; color: #2c3e50;'>"
            "<a href='doc://%1' style='color: #3498db; text-decoration: none;'>%2</a>"
            "</h4>"
            "<p style='margin: 0; color: #7f8c8d; font-size: 12px;'>类别: %3</p>"
            "</div>"
        ).arg(doc.getId()).arg(doc.getTitle()).arg(doc.getCategory());
    }
    
    html += "</div>";
    return html;
}

void QAWidget::askQuestion()
{
    onQuestionSubmitted();
}

void QAWidget::showQuickQuestions()
{
    quickQuestionsGroup_->setVisible(!quickQuestionsGroup_->isVisible());
}

void QAWidget::showHistory()
{
    historyGroup_->setVisible(!historyGroup_->isVisible());
}

void QAWidget::clearHistory()
{
    int ret = QMessageBox::question(this, "确认清空", 
                                   "确定要清空所有问答历史吗？此操作不可恢复。",
                                   QMessageBox::Yes | QMessageBox::No,
                                   QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        historyList_->clear();
        currentHistory_.clear();
        QMessageBox::information(this, "提示", "历史记录已清空。");
    }
}



void QAWidget::showRelatedDocuments(const QAResult& result)
{
    displayRelatedDocuments(result.relatedDocuments);
}

// AI相关槽函数实现
void QAWidget::onAIAnswerReceived(const QString& question, const QAResult& result)
{
    try {
        // 记录到数据库
        QuestionRecord record(currentUser_.getEmployeeId(), question, result.answer, result.category);
        database_->addQuestionRecord(record);
        
        // 显示答案
        displayAnswer(result);
        
        // 显示相关文档
        displayRelatedDocuments(result.relatedDocuments);
        
        // 更新历史记录
        addToHistory(question, result);
        
        // 清空问题输入框
        questionEdit_->clear();
        
        // 发出信号
        emit questionAsked(question, result);
        
    } catch (...) {
        // 处理异常
        QAResult errorResult;
        errorResult.answer = "抱歉，处理AI回答时发生错误。";
        errorResult.category = "错误";
        errorResult.confidence = "错误";
        displayAnswer(errorResult);
    }
    
    // 恢复提问按钮
    askButton_->setEnabled(true);
    askButton_->setText("提问");
}


