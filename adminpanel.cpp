
#include "adminpanel.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QHeaderView>
#include <QInputDialog>
#include <QProgressDialog>
#include <QTimer>
#include <QSettings>

AdminPanel::AdminPanel(Database* database, const User& currentUser, QWidget *parent)
    : QWidget(parent)
    , database_(database)
    , currentUser_(currentUser)
    , mainLayout_(nullptr)
    , tabWidget_(nullptr)
    , userTab_(nullptr)
    , refreshTimer_(nullptr)
    , dataChanged_(false)
    , reportGenerator_(nullptr)
{
    if (!currentUser_.isHRAdmin()) {
        QMessageBox::warning(this, "权限不足", "只有HR管理员才能访问管理后台。");
        return;
    }
    
    setupUI();
    setupConnections();
    applyStyles();
    setupTables();
    // 初始化报表生成器
    reportGenerator_ = new ReportGenerator(database_, this);
    
    loadUsers();
    loadDocuments();
    loadQAs();
    loadStatistics();
    loadSettings();
}

AdminPanel::~AdminPanel()
{
    if (refreshTimer_) {
        refreshTimer_->stop();
    }
}

void AdminPanel::setupUI()
{
    // 现代化主布局
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(16);
    
    // 创建现代化标签页
    tabWidget_ = new QTabWidget();
    tabWidget_->setTabPosition(QTabWidget::North);
    tabWidget_->setObjectName("adminTabWidget");
    
    setupUserTab();
    setupDocumentTab();
    setupQATab();
    setupStatisticsTab();
    setupSettingsTab();
    
    mainLayout_->addWidget(tabWidget_);
    
    // 创建刷新定时器
    refreshTimer_ = new QTimer(this);
    refreshTimer_->setSingleShot(false);
    refreshTimer_->setInterval(30000); // 30秒刷新一次
}

void AdminPanel::setupUserTab()
{
    userTab_ = new QWidget();
    userLayout_ = new QVBoxLayout(userTab_);
    userLayout_->setContentsMargins(16, 16, 16, 16);
    userLayout_->setSpacing(16);
    
    // 现代化搜索和筛选卡片
    QWidget* searchCard = new QWidget();
    searchCard->setObjectName("cardContainer");
    QVBoxLayout* searchCardLayout = new QVBoxLayout(searchCard);
    searchCardLayout->setContentsMargins(20, 20, 20, 20);
    searchCardLayout->setSpacing(16);
    
    QLabel* searchTitle = new QLabel("搜索和筛选");
    searchTitle->setObjectName("sectionTitle");
    searchCardLayout->addWidget(searchTitle);
    
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(16);
    
    userSearchEdit_ = new QLineEdit();
    userSearchEdit_->setPlaceholderText("搜索工号或姓名...");
    userSearchEdit_->setMinimumWidth(200);
    
    userRoleFilter_ = new QComboBox();
    userRoleFilter_->addItem("所有角色", -1);
    userRoleFilter_->addItem("普通员工", static_cast<int>(UserRole::Employee));
    userRoleFilter_->addItem("HR管理员", static_cast<int>(UserRole::HRAdmin));
    
    userStatusFilter_ = new QComboBox();
    userStatusFilter_->addItem("所有状态", -1);
    userStatusFilter_->addItem("待审核", static_cast<int>(UserStatus::Pending));
    userStatusFilter_->addItem("已激活", static_cast<int>(UserStatus::Active));
    userStatusFilter_->addItem("已禁用", static_cast<int>(UserStatus::Inactive));
    
    userRefreshButton_ = new QPushButton("刷新");
    userRefreshButton_->setMinimumHeight(40);
    
    searchLayout->addWidget(userSearchEdit_);
    searchLayout->addWidget(userRoleFilter_);
    searchLayout->addWidget(userStatusFilter_);
    searchLayout->addWidget(userRefreshButton_);
    searchLayout->addStretch();
    
    searchCardLayout->addLayout(searchLayout);
    userLayout_->addWidget(searchCard);
    
    // 现代化用户表格卡片
    QWidget* tableCard = new QWidget();
    tableCard->setObjectName("cardContainer");
    QVBoxLayout* tableCardLayout = new QVBoxLayout(tableCard);
    tableCardLayout->setContentsMargins(20, 20, 20, 20);
    tableCardLayout->setSpacing(16);
    
    QLabel* tableTitle = new QLabel("用户列表");
    tableTitle->setObjectName("sectionTitle");
    tableCardLayout->addWidget(tableTitle);
    
    userTable_ = new QTableWidget();
    userTable_->setContextMenuPolicy(Qt::CustomContextMenu);
    userTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable_->setAlternatingRowColors(true);
    tableCardLayout->addWidget(userTable_);
    
    userLayout_->addWidget(tableCard);
    
    // 现代化操作按钮卡片
    QWidget* actionCard = new QWidget();
    actionCard->setObjectName("cardContainer");
    QVBoxLayout* actionCardLayout = new QVBoxLayout(actionCard);
    actionCardLayout->setContentsMargins(20, 20, 20, 20);
    actionCardLayout->setSpacing(16);
    
    QLabel* actionTitle = new QLabel("操作");
    actionTitle->setObjectName("sectionTitle");
    actionCardLayout->addWidget(actionTitle);
    
    QHBoxLayout* actionLayout = new QHBoxLayout();
    actionLayout->setSpacing(12);
    
    addUserButton_ = new QPushButton("添加用户");
    addUserButton_->setMinimumHeight(40);
    editUserButton_ = new QPushButton("编辑用户");
    editUserButton_->setMinimumHeight(40);
    editUserButton_->setObjectName("secondaryButton");
    deleteUserButton_ = new QPushButton("删除用户");
    deleteUserButton_->setMinimumHeight(40);
    deleteUserButton_->setObjectName("dangerButton");
    activateUserButton_ = new QPushButton("激活用户");
    activateUserButton_->setMinimumHeight(40);
    activateUserButton_->setObjectName("successButton");
    
    editUserButton_->setEnabled(false);
    deleteUserButton_->setEnabled(false);
    activateUserButton_->setEnabled(false);
    
    actionLayout->addWidget(addUserButton_);
    actionLayout->addWidget(editUserButton_);
    actionLayout->addWidget(deleteUserButton_);
    actionLayout->addWidget(activateUserButton_);
    actionLayout->addStretch();
    
    actionCardLayout->addLayout(actionLayout);
    userLayout_->addWidget(actionCard);
    
    tabWidget_->addTab(userTab_, "用户管理");
}

void AdminPanel::setupDocumentTab()
{
    documentTab_ = new QWidget();
    documentLayout_ = new QVBoxLayout(documentTab_);
    
    // 搜索和筛选区域
    documentSearchGroup_ = new QGroupBox("搜索和筛选");
    documentSearchLayout_ = new QHBoxLayout(documentSearchGroup_);
    
    documentSearchEdit_ = new QLineEdit();
    documentSearchEdit_->setPlaceholderText("搜索文档标题或内容...");
    
    documentCategoryFilter_ = new QComboBox();
    documentCategoryFilter_->addItem("所有类别", "");
    documentCategoryFilter_->addItem("考勤", "考勤");
    documentCategoryFilter_->addItem("薪酬", "薪酬");
    documentCategoryFilter_->addItem("绩效", "绩效");
    documentCategoryFilter_->addItem("入职离职", "入职离职");
    documentCategoryFilter_->addItem("员工福利", "员工福利");
    
    documentRefreshButton_ = new QPushButton("刷新");
    
    documentSearchLayout_->addWidget(new QLabel("搜索:"));
    documentSearchLayout_->addWidget(documentSearchEdit_);
    documentSearchLayout_->addWidget(new QLabel("类别:"));
    documentSearchLayout_->addWidget(documentCategoryFilter_);
    documentSearchLayout_->addWidget(documentRefreshButton_);
    documentSearchLayout_->addStretch();
    
    documentLayout_->addWidget(documentSearchGroup_);
    
    // 文档表格
    documentTable_ = new QTableWidget();
    documentTable_->setContextMenuPolicy(Qt::CustomContextMenu);
    documentTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    documentTable_->setAlternatingRowColors(true);
    documentLayout_->addWidget(documentTable_);
    
    // 操作按钮
    documentActionGroup_ = new QGroupBox("操作");
    documentActionLayout_ = new QHBoxLayout(documentActionGroup_);
    
    addDocumentButton_ = new QPushButton("添加文档");
    editDocumentButton_ = new QPushButton("编辑文档");
    deleteDocumentButton_ = new QPushButton("删除文档");
    uploadDocumentButton_ = new QPushButton("批量上传");
    
    editDocumentButton_->setEnabled(false);
    deleteDocumentButton_->setEnabled(false);
    
    documentActionLayout_->addWidget(addDocumentButton_);
    documentActionLayout_->addWidget(editDocumentButton_);
    documentActionLayout_->addWidget(deleteDocumentButton_);
    documentActionLayout_->addWidget(uploadDocumentButton_);
    documentActionLayout_->addStretch();
    
    documentLayout_->addWidget(documentActionGroup_);
    
    tabWidget_->addTab(documentTab_, "文档管理");
}

void AdminPanel::setupQATab()
{
    qaTab_ = new QWidget();
    qaLayout_ = new QVBoxLayout(qaTab_);
    
    // 搜索和筛选区域
    qaSearchGroup_ = new QGroupBox("搜索和筛选");
    qaSearchLayout_ = new QHBoxLayout(qaSearchGroup_);
    
    qaSearchEdit_ = new QLineEdit();
    qaSearchEdit_->setPlaceholderText("搜索问题或答案...");
    
    qaCategoryFilter_ = new QComboBox();
    qaCategoryFilter_->addItem("所有类别", "");
    qaCategoryFilter_->addItem("考勤", "考勤");
    qaCategoryFilter_->addItem("薪酬", "薪酬");
    qaCategoryFilter_->addItem("绩效", "绩效");
    qaCategoryFilter_->addItem("入职离职", "入职离职");
    qaCategoryFilter_->addItem("员工福利", "员工福利");
    
    qaRefreshButton_ = new QPushButton("刷新");
    
    qaSearchLayout_->addWidget(new QLabel("搜索:"));
    qaSearchLayout_->addWidget(qaSearchEdit_);
    qaSearchLayout_->addWidget(new QLabel("类别:"));
    qaSearchLayout_->addWidget(qaCategoryFilter_);
    qaSearchLayout_->addWidget(qaRefreshButton_);
    qaSearchLayout_->addStretch();
    
    qaLayout_->addWidget(qaSearchGroup_);
    
    // 问答表格
    qaTable_ = new QTableWidget();
    qaTable_->setContextMenuPolicy(Qt::CustomContextMenu);
    qaTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    qaTable_->setAlternatingRowColors(true);
    qaLayout_->addWidget(qaTable_);
    
    // 操作按钮
    qaActionGroup_ = new QGroupBox("操作");
    qaActionLayout_ = new QHBoxLayout(qaActionGroup_);
    
    organizeQAButton_ = new QPushButton("整理数据");
    // 移除导出Excel按钮，保留整理与报表功能
    exportQAButton_ = nullptr;
    
    qaActionLayout_->addWidget(organizeQAButton_);
    // 不再添加导出Excel按钮
    qaActionLayout_->addStretch();
    
    qaLayout_->addWidget(qaActionGroup_);
    
    tabWidget_->addTab(qaTab_, "问答管理");
}

void AdminPanel::setupStatisticsTab()
{
    statisticsTab_ = new QWidget();
    statisticsLayout_ = new QVBoxLayout(statisticsTab_);
    statisticsLayout_->setContentsMargins(16, 16, 16, 16);
    statisticsLayout_->setSpacing(16);
    
    // 现代化筛选卡片
    QWidget* filterCard = new QWidget();
    filterCard->setObjectName("cardContainer");
    QVBoxLayout* filterCardLayout = new QVBoxLayout(filterCard);
    filterCardLayout->setContentsMargins(15, 10, 15, 10);  // 减少内边距，特别是上下边距
    filterCardLayout->setSpacing(8);  // 减少垂直间距
    
    QLabel* filterTitle = new QLabel("统计筛选");
    filterTitle->setObjectName("sectionTitle");
    filterCardLayout->addWidget(filterTitle);
    
    // 创建筛选内容容器，调整宽度以适应完整文字
    QWidget* filterContentWidget = new QWidget();
    filterContentWidget->setMaximumWidth(800);  // 大幅增加宽度以适应更宽的按钮
    filterContentWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    QHBoxLayout* filterLayout = new QHBoxLayout(filterContentWidget);
    filterLayout->setSpacing(8);  // 减少间距
    filterLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel* dateRangeLabel = new QLabel("时间范围:");
    dateRangeLabel->setObjectName("sectionTitle");
    startDateEdit_ = new QDateEdit(QDate::currentDate().addDays(-30));
    startDateEdit_->setCalendarPopup(true);
    startDateEdit_->setFixedWidth(105);
    startDateEdit_->setFixedHeight(32);  // 减少高度
    endDateEdit_ = new QDateEdit(QDate::currentDate());
    endDateEdit_->setCalendarPopup(true);
    endDateEdit_->setFixedWidth(105);
    endDateEdit_->setFixedHeight(32);  // 减少高度
    
    refreshStatisticsButton_ = new QPushButton("刷新统计");
    refreshStatisticsButton_->setFixedHeight(32);  
    refreshStatisticsButton_->setFixedWidth(120);   // 大幅增加宽度确保4个中文字符完整显示
    generateReportButton_ = new QPushButton("生成报表");
    generateReportButton_->setFixedHeight(32);  
    generateReportButton_->setFixedWidth(120);   // 大幅增加宽度确保4个中文字符完整显示
    exportReportButton_ = nullptr; // 统计页也移除导出Excel
    
    filterLayout->addWidget(dateRangeLabel);
    filterLayout->addWidget(startDateEdit_);
    filterLayout->addWidget(new QLabel("至"));
    filterLayout->addWidget(endDateEdit_);
    filterLayout->addSpacing(10);  // 减少间距
    filterLayout->addWidget(refreshStatisticsButton_);
    filterLayout->addWidget(generateReportButton_);
    // 不再添加导出Excel按钮
    
    // 将筛选内容容器居中添加到卡片中
    QHBoxLayout* filterWrapperLayout = new QHBoxLayout();
    filterWrapperLayout->addWidget(filterContentWidget, 0, Qt::AlignLeft);
    filterWrapperLayout->addStretch();
    
    filterCardLayout->addLayout(filterWrapperLayout);
    
    // 设置筛选卡片最大高度，确保紧凑布局
    filterCard->setMaximumHeight(90);
    statisticsLayout_->addWidget(filterCard, 0);  // 不给筛选区域分配额外空间
    
    // 现代化统计卡片网格
    QWidget* statsCard = new QWidget();
    statsCard->setObjectName("cardContainer");
    QVBoxLayout* statsCardLayout = new QVBoxLayout(statsCard);
    statsCardLayout->setContentsMargins(25, 20, 25, 25);  // 优化内边距
    statsCardLayout->setSpacing(15);  // 减少标题与内容间距
    
    QLabel* statsTitle = new QLabel("数据概览");
    statsTitle->setObjectName("sectionTitle");
    statsTitle->setMaximumHeight(40);  // 限制标题高度
    statsCardLayout->addWidget(statsTitle, 0);  // 不给标题分配额外空间
    
    // 创建统计卡片网格
    QGridLayout* statsGrid = new QGridLayout();
    statsGrid->setSpacing(20);  // 增加网格间距
    
    totalUsersLabel_ = new QLabel("0");
    totalUsersLabel_->setObjectName("statsNumber");
    QLabel* usersLabel = new QLabel("总用户数");
    usersLabel->setObjectName("statsLabel");
    QWidget* usersCard = createStatsCard(totalUsersLabel_, usersLabel, "#3b82f6");
    
    totalDocumentsLabel_ = new QLabel("0");
    totalDocumentsLabel_->setObjectName("statsNumber");
    QLabel* docsLabel = new QLabel("总文档数");
    docsLabel->setObjectName("statsLabel");
    QWidget* docsCard = createStatsCard(totalDocumentsLabel_, docsLabel, "#10b981");
    
    totalQuestionsLabel_ = new QLabel("0");
    totalQuestionsLabel_->setObjectName("statsNumber");
    QLabel* questionsLabel = new QLabel("总问题数");
    questionsLabel->setObjectName("statsLabel");
    QWidget* questionsCard = createStatsCard(totalQuestionsLabel_, questionsLabel, "#f59e0b");
    
    activeUsersLabel_ = new QLabel("0");
    activeUsersLabel_->setObjectName("statsNumber");
    QLabel* activeLabel = new QLabel("活跃用户");
    activeLabel->setObjectName("statsLabel");
    QWidget* activeCard = createStatsCard(activeUsersLabel_, activeLabel, "#8b5cf6");
    
    pendingUsersLabel_ = new QLabel("0");
    pendingUsersLabel_->setObjectName("statsNumber");
    QLabel* pendingLabel = new QLabel("待审核");
    pendingLabel->setObjectName("statsLabel");
    QWidget* pendingCard = createStatsCard(pendingUsersLabel_, pendingLabel, "#ef4444");
    
    statsGrid->addWidget(usersCard, 0, 0);
    statsGrid->addWidget(docsCard, 0, 1);
    statsGrid->addWidget(questionsCard, 0, 2);
    statsGrid->addWidget(activeCard, 1, 0);
    statsGrid->addWidget(pendingCard, 1, 1);
    
    // 创建统计内容容器，占用大部分空间
    QWidget* statsContentWidget = new QWidget();
    QVBoxLayout* statsContentLayout = new QVBoxLayout(statsContentWidget);
    statsContentLayout->setContentsMargins(0, 0, 0, 0);
    statsContentLayout->setSpacing(0);
    statsContentLayout->addLayout(statsGrid);
    statsContentLayout->addStretch();  // 添加弹性空间，让卡片居中显示
    
    statsCardLayout->addWidget(statsContentWidget, 1);  // 给统计内容分配大部分空间
    statisticsLayout_->addWidget(statsCard, 1);  // 给数据概览区域分配更多空间（拉伸因子为1）
    
    tabWidget_->addTab(statisticsTab_, "数据统计");
}

QWidget* AdminPanel::createStatsCard(QLabel* numberLabel, QLabel* textLabel, const QString& color)
{
    QWidget* card = new QWidget();
    card->setObjectName("statsCard");
    card->setStyleSheet(QString("QWidget#statsCard { "
                               "background: white; "
                               "border: 1px solid #e5e7eb; "
                               "border-radius: 12px; "
                               "border-left: 4px solid %1; "
                               "padding: 20px; }").arg(color));
    
    QVBoxLayout* layout = new QVBoxLayout(card);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(8);
    layout->setAlignment(Qt::AlignCenter);
    
    layout->addWidget(numberLabel);
    layout->addWidget(textLabel);
    
    return card;
}

void AdminPanel::setupSettingsTab()
{
    settingsTab_ = new QWidget();
    settingsLayout_ = new QVBoxLayout(settingsTab_);
    
    // 常规设置
    generalSettingsGroup_ = new QGroupBox("常规设置");
    generalSettingsLayout_ = new QGridLayout(generalSettingsGroup_);
    
    autoBackupLabel_ = new QLabel("自动备份:");
    autoBackupCheck_ = new QCheckBox();
    
    backupIntervalLabel_ = new QLabel("备份间隔(天):");
    backupIntervalSpin_ = new QSpinBox();
    backupIntervalSpin_->setRange(1, 30);
    backupIntervalSpin_->setValue(7);
    
    sessionTimeoutLabel_ = new QLabel("会话超时(分钟):");
    sessionTimeoutSpin_ = new QSpinBox();
    sessionTimeoutSpin_->setRange(5, 120);
    sessionTimeoutSpin_->setValue(30);
    
    generalSettingsLayout_->addWidget(autoBackupLabel_, 0, 0);
    generalSettingsLayout_->addWidget(autoBackupCheck_, 0, 1);
    generalSettingsLayout_->addWidget(backupIntervalLabel_, 1, 0);
    generalSettingsLayout_->addWidget(backupIntervalSpin_, 1, 1);
    generalSettingsLayout_->addWidget(sessionTimeoutLabel_, 2, 0);
    generalSettingsLayout_->addWidget(sessionTimeoutSpin_, 2, 1);
    
    settingsLayout_->addWidget(generalSettingsGroup_);
    
    // 数据管理
    dataManagementGroup_ = new QGroupBox("数据管理");
    dataManagementLayout_ = new QHBoxLayout(dataManagementGroup_);
    
    backupDataButton_ = new QPushButton("备份数据");
    restoreDataButton_ = new QPushButton("恢复数据");
    saveSettingsButton_ = new QPushButton("保存设置");
    resetSettingsButton_ = new QPushButton("重置设置");
    
    dataManagementLayout_->addWidget(backupDataButton_);
    dataManagementLayout_->addWidget(restoreDataButton_);
    dataManagementLayout_->addWidget(saveSettingsButton_);
    dataManagementLayout_->addWidget(resetSettingsButton_);
    dataManagementLayout_->addStretch();
    
    settingsLayout_->addWidget(dataManagementGroup_);
    settingsLayout_->addStretch();
    
    tabWidget_->addTab(settingsTab_, "系统设置");
}

void AdminPanel::setupConnections()
{
    // 用户管理连接
    connect(userRefreshButton_, &QPushButton::clicked, this, &AdminPanel::loadUsers);
    connect(addUserButton_, &QPushButton::clicked, this, &AdminPanel::onAddUserClicked);
    connect(editUserButton_, &QPushButton::clicked, this, &AdminPanel::onEditUserClicked);
    connect(deleteUserButton_, &QPushButton::clicked, this, &AdminPanel::onDeleteUserClicked);
    connect(activateUserButton_, &QPushButton::clicked, this, &AdminPanel::onActivateUserClicked);
    connect(userTable_, &QTableWidget::customContextMenuRequested, this, &AdminPanel::onUserTableContextMenu);
    connect(userTable_, &QTableWidget::itemSelectionChanged, this, &AdminPanel::onUserTableSelectionChanged);
    connect(userSearchEdit_, &QLineEdit::textChanged, this, &AdminPanel::onUserSearchTextChanged);
    connect(userRoleFilter_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &AdminPanel::onUserRoleFilterChanged);
    connect(userStatusFilter_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &AdminPanel::onUserStatusFilterChanged);
    
    // 文档管理连接
    connect(documentRefreshButton_, &QPushButton::clicked, this, &AdminPanel::loadDocuments);
    connect(addDocumentButton_, &QPushButton::clicked, this, &AdminPanel::onAddDocumentClicked);
    connect(editDocumentButton_, &QPushButton::clicked, this, &AdminPanel::onEditDocumentClicked);
    connect(deleteDocumentButton_, &QPushButton::clicked, this, &AdminPanel::onDeleteDocumentClicked);
    connect(uploadDocumentButton_, &QPushButton::clicked, this, &AdminPanel::onUploadDocumentClicked);
    connect(documentTable_, &QTableWidget::customContextMenuRequested, this, &AdminPanel::onDocumentTableContextMenu);
    connect(documentTable_, &QTableWidget::itemSelectionChanged, this, &AdminPanel::onDocumentTableSelectionChanged);
    connect(documentSearchEdit_, &QLineEdit::textChanged, this, &AdminPanel::onDocumentSearchTextChanged);
    connect(documentCategoryFilter_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &AdminPanel::onDocumentCategoryFilterChanged);
    
    // 问答管理连接
    connect(qaRefreshButton_, &QPushButton::clicked, this, &AdminPanel::loadQAs);
    connect(organizeQAButton_, &QPushButton::clicked, this, &AdminPanel::onOrganizeQAClicked);
    connect(exportQAButton_, &QPushButton::clicked, this, &AdminPanel::onExportQAClicked);
    connect(qaTable_, &QTableWidget::customContextMenuRequested, this, &AdminPanel::onQATableContextMenu);
    connect(qaTable_, &QTableWidget::itemSelectionChanged, this, &AdminPanel::onQATableSelectionChanged);
    connect(qaSearchEdit_, &QLineEdit::textChanged, this, &AdminPanel::onQASearchTextChanged);
    connect(qaCategoryFilter_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged), this, &AdminPanel::onQACategoryFilterChanged);
    
    // 统计管理连接
    connect(refreshStatisticsButton_, &QPushButton::clicked, this, &AdminPanel::onRefreshStatisticsClicked);
    connect(generateReportButton_, &QPushButton::clicked, this, &AdminPanel::onGenerateReportClicked);
    connect(exportReportButton_, &QPushButton::clicked, this, &AdminPanel::onExportReportClicked);
    
    // 设置管理连接
    connect(saveSettingsButton_, &QPushButton::clicked, this, &AdminPanel::onSaveSettingsClicked);
    connect(resetSettingsButton_, &QPushButton::clicked, this, &AdminPanel::onResetSettingsClicked);
    connect(backupDataButton_, &QPushButton::clicked, this, &AdminPanel::onBackupDataClicked);
    connect(restoreDataButton_, &QPushButton::clicked, this, &AdminPanel::onRestoreDataClicked);
    
    // 定时器连接
    connect(refreshTimer_, &QTimer::timeout, this, &AdminPanel::refreshData);
}

void AdminPanel::applyStyles()
{
    setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #e1e8ed;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 0 5px 0 5px;"
        "    color: #2c3e50;"
        "}"
        "QTableWidget {"
        "    gridline-color: #e1e8ed;"
        "    selection-background-color: #3498db;"
        "}"
        "QHeaderView::section {"
        "    background: #34495e;"
        "    color: white;"
        "    padding: 8px;"
        "    font-weight: bold;"
        "}"
    );
}

void AdminPanel::setupTables()
{
    // 设置用户表格
    userTable_->setColumnCount(7);
    QStringList userHeaders = {"工号", "姓名", "部门", "角色", "状态", "创建时间", "最后登录"};
    userTable_->setHorizontalHeaderLabels(userHeaders);
    
    // 设置用户表格列宽
    userTable_->setColumnWidth(0, 100);  // 工号
    userTable_->setColumnWidth(1, 120);  // 姓名
    userTable_->setColumnWidth(2, 120);  // 部门
    userTable_->setColumnWidth(3, 100);  // 角色
    userTable_->setColumnWidth(4, 80);   // 状态
    userTable_->setColumnWidth(5, 120);  // 创建时间
    userTable_->horizontalHeader()->setStretchLastSection(true);
    
    // 设置文档表格
    documentTable_->setColumnCount(5);
    QStringList docHeaders = {"ID", "标题", "类别", "创建时间", "更新时间"};
    documentTable_->setHorizontalHeaderLabels(docHeaders);
    documentTable_->horizontalHeader()->setStretchLastSection(true);
    
    // 设置问答表格
    qaTable_->setColumnCount(5);
    QStringList qaHeaders = {"ID", "员工", "问题", "类别", "创建时间"};
    qaTable_->setHorizontalHeaderLabels(qaHeaders);
    qaTable_->horizontalHeader()->setStretchLastSection(true);
}

void AdminPanel::loadUsers()
{
    // 重新加载用户数据（保存全量）
    allUsers_ = database_->getAllUsers();
    userList_ = allUsers_; // 初始为全量
    filterUsers();
    updateStatisticsDisplay();
    this->update(); this->repaint(); QApplication::processEvents();
}

void AdminPanel::loadDocuments()
{
    qDebug() << "=== 加载文档数据 ===";
    allDocuments_ = database_->getAllDocuments();
    qDebug() << "从数据库获取到" << allDocuments_.size() << "个文档";
    
    // 如果没有文档数据，强制重新加载示例数据
    if (allDocuments_.isEmpty()) {
        qDebug() << "检测到文档数据为空，重新加载示例数据...";
        database_->reloadSampleData();
        allDocuments_ = database_->getAllDocuments();
        qDebug() << "重新加载后获取到" << allDocuments_.size() << "个文档";
    }
    
    for (const Document& doc : allDocuments_) {
        qDebug() << "文档:" << doc.getId() << doc.getTitle() << doc.getCategory();
    }
    
    documentList_ = allDocuments_;
    filterDocuments(); // 重新应用筛选
}

void AdminPanel::filterDocuments()
{
    const QString keyword = documentSearchEdit_ ? documentSearchEdit_->text().trimmed() : QString();
    const QString category = documentCategoryFilter_ ? documentCategoryFilter_->currentData().toString() : QString();
    
    documentList_.clear();
    for (const Document& d : allDocuments_) {
        bool match = true;
        if (!keyword.isEmpty()) {
            match &= (d.getTitle().contains(keyword, Qt::CaseInsensitive) ||
                      d.getContent().contains(keyword, Qt::CaseInsensitive));
        }
        if (!category.isEmpty()) {
            match &= (d.getCategory() == category);
        }
        if (match) documentList_.append(d);
    }
    refreshDocumentTable();
}

void AdminPanel::loadQAs()
{
    qDebug() << "=== 加载问答数据 ===";
    allQAs_ = database_->getQuestionRecords();
    qDebug() << "从数据库获取到" << allQAs_.size() << "个问答记录";
    
    // 如果没有问答数据且文档数据存在，说明只是问答数据缺失
    if (allQAs_.isEmpty() && !allDocuments_.isEmpty()) {
        qDebug() << "检测到问答数据为空，重新加载示例数据...";
        database_->reloadSampleData();
        allQAs_ = database_->getQuestionRecords();
        qDebug() << "重新加载后获取到" << allQAs_.size() << "个问答记录";
    }
    
    for (const QuestionRecord& qa : allQAs_) {
        qDebug() << "问答:" << qa.getId() << qa.getEmployeeId() << qa.getQuestion().left(20) + "..." << qa.getCategory();
    }
    
    qaList_ = allQAs_;
    filterQAs(); // 重新应用筛选
}

void AdminPanel::filterQAs()
{
    const QString keyword = qaSearchEdit_ ? qaSearchEdit_->text().trimmed() : QString();
    const QString category = qaCategoryFilter_ ? qaCategoryFilter_->currentData().toString() : QString();
    
    qaList_.clear();
    for (const QuestionRecord& qa : allQAs_) {
        bool match = true;
        if (!keyword.isEmpty()) {
            match &= (qa.getQuestion().contains(keyword, Qt::CaseInsensitive) ||
                      qa.getAnswer().contains(keyword, Qt::CaseInsensitive) ||
                      qa.getEmployeeId().contains(keyword, Qt::CaseInsensitive));
        }
        if (!category.isEmpty()) {
            match &= (qa.getCategory() == category);
        }
        if (match) qaList_.append(qa);
    }
    refreshQATable();
}

void AdminPanel::loadStatistics()
{
    updateStatisticsDisplay();
}

void AdminPanel::loadSettings()
{
    QSettings settings("Company HR System", "HR智能问答系统");
    autoBackupCheck_->setChecked(settings.value("admin/autoBackup", false).toBool());
    backupIntervalSpin_->setValue(settings.value("admin/backupIntervalDays", 7).toInt());
    sessionTimeoutSpin_->setValue(settings.value("admin/sessionTimeoutMinutes", 30).toInt());
}

void AdminPanel::refreshUserTable()
{
    // 暂时断开选择变化信号，避免在刷新过程中触发事件
    userTable_->blockSignals(true);
    
    // 清空表格内容并重置行数
    userTable_->clearContents();
    userTable_->setRowCount(0);
    
    // 强制处理事件，确保清空操作完成
    QApplication::processEvents();
    
    // 重新设置行数
    userTable_->setRowCount(userList_.size());
    
    // 填充数据
    for (int i = 0; i < userList_.size(); ++i) {
        const User& user = userList_[i];
        
        userTable_->setItem(i, 0, new QTableWidgetItem(user.getEmployeeId()));
        userTable_->setItem(i, 1, new QTableWidgetItem(user.getName()));
        userTable_->setItem(i, 2, new QTableWidgetItem(user.getDepartment()));
        userTable_->setItem(i, 3, new QTableWidgetItem(user.isHRAdmin() ? "HR管理员" : "普通员工"));
        
        QString status;
        switch (user.getStatus()) {
        case UserStatus::Pending: status = "待审核"; break;
        case UserStatus::Active: status = "已激活"; break;
        case UserStatus::Inactive: status = "已禁用"; break;
        }
        userTable_->setItem(i, 4, new QTableWidgetItem(status));
        
        userTable_->setItem(i, 5, new QTableWidgetItem(user.getCreateTime().toString("yyyy-MM-dd")));
        userTable_->setItem(i, 6, new QTableWidgetItem(user.getLastLoginTime().toString("yyyy-MM-dd hh:mm")));
    }
    
    // 调整列宽
    userTable_->resizeColumnsToContents();
    
    // 恢复信号
    userTable_->blockSignals(false);
    
    // 强制更新表格显示
    userTable_->update();
    userTable_->viewport()->update();
    
    // 确保表格完全重绘
    userTable_->repaint();
}

void AdminPanel::refreshDocumentTable()
{
    documentTable_->setRowCount(documentList_.size());
    
    for (int i = 0; i < documentList_.size(); ++i) {
        const Document& doc = documentList_[i];
        
        documentTable_->setItem(i, 0, new QTableWidgetItem(QString::number(doc.getId())));
        documentTable_->setItem(i, 1, new QTableWidgetItem(doc.getTitle()));
        documentTable_->setItem(i, 2, new QTableWidgetItem(doc.getCategory()));
        documentTable_->setItem(i, 3, new QTableWidgetItem(doc.getCreateTime().toString("yyyy-MM-dd")));
        documentTable_->setItem(i, 4, new QTableWidgetItem(doc.getUpdateTime().toString("yyyy-MM-dd")));
    }
    
    // 设置表格列宽，确保内容完整显示
    documentTable_->resizeColumnsToContents();
    
    // 设置各列的合适宽度
    documentTable_->setColumnWidth(0, 60);   // ID列固定较小宽度
    documentTable_->setColumnWidth(2, 100);  // 类别列固定宽度
    documentTable_->setColumnWidth(3, 140);  // 创建时间固定宽度
    documentTable_->setColumnWidth(4, 140);  // 更新时间固定宽度
    
    // 让标题列拉伸填充剩余空间
    documentTable_->horizontalHeader()->setStretchLastSection(false);
    documentTable_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
}

void AdminPanel::refreshQATable()
{
    qaTable_->setRowCount(qaList_.size());
    
    for (int i = 0; i < qaList_.size(); ++i) {
        const QuestionRecord& qa = qaList_[i];
        
        qaTable_->setItem(i, 0, new QTableWidgetItem(QString::number(qa.getId())));
        qaTable_->setItem(i, 1, new QTableWidgetItem(qa.getEmployeeId()));
        qaTable_->setItem(i, 2, new QTableWidgetItem(qa.getQuestion()));
        qaTable_->setItem(i, 3, new QTableWidgetItem(qa.getCategory()));
        qaTable_->setItem(i, 4, new QTableWidgetItem(qa.getCreateTime().toString("yyyy-MM-dd hh:mm")));
    }
    
    // 列宽与拉伸策略：固定部分列宽，“问题”列拉伸填充
    qaTable_->resizeColumnsToContents();
    qaTable_->setColumnWidth(0, 60);   // ID
    qaTable_->setColumnWidth(1, 90);   // 员工
    qaTable_->setColumnWidth(3, 100);  // 类别
    qaTable_->setColumnWidth(4, 160);  // 创建时间
    qaTable_->horizontalHeader()->setStretchLastSection(false);
    qaTable_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch); // 问题列自适应

    // 强制刷新，避免刷新后显示不全
    qaTable_->update();
    qaTable_->viewport()->update();
    qaTable_->repaint();
    QApplication::processEvents();
}

void AdminPanel::updateStatisticsDisplay()
{
    int totalUsers = database_->getUserCount();
    int totalDocs = database_->getDocumentCount();
    int totalQuestions = database_->getQuestionCount();
    
    // 修复：正确计算活跃用户数（状态为Active的用户）
    QList<User> allUsers = database_->getAllUsers();
    int activeUsers = 0;
    for (const User& user : allUsers) {
        if (user.getStatus() == UserStatus::Active) {
            activeUsers++;
        }
    }
    
    int pendingUsers = database_->getPendingUsers().size();
    
    totalUsersLabel_->setText(QString("总用户数: %1").arg(totalUsers));
    totalDocumentsLabel_->setText(QString("总文档数: %1").arg(totalDocs));
    totalQuestionsLabel_->setText(QString("总问题数: %1").arg(totalQuestions));
    activeUsersLabel_->setText(QString("活跃用户: %1").arg(activeUsers));
    pendingUsersLabel_->setText(QString("待审核: %1").arg(pendingUsers));
}

// 实现各种槽函数
void AdminPanel::onAddUserClicked()
{
    UserDialog dialog(database_, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadUsers();
        showMessage("添加成功", "用户已成功添加！");
    }
}

void AdminPanel::onEditUserClicked()
{
    int currentRow = userTable_->currentRow();
    if (currentRow < 0) {
        showMessage("提示", "请先选择要编辑的用户！");
        return;
    }
    
    // 获取选中的用户
    const User& user = userList_[currentRow];
    UserDialog dialog(database_, user, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadUsers();
        showMessage("编辑成功", "用户信息已成功更新！");
    }
}

void AdminPanel::onDeleteUserClicked()
{
    int row = userTable_->currentRow();
    if (row < 0 || row >= userList_.size()) {
        showMessage("提示", "请先选择要删除的用户！");
        return;
    }
    
    const User& user = userList_[row];
    if (confirmAction("删除用户", QString("确定要删除用户 %1 吗？").arg(user.getName()))) {
        if (database_->deleteUser(user.getEmployeeId())) {
            showMessage("成功", "用户删除成功！");
            
            // 延迟刷新用户列表，避免在消息框显示期间操作表格
            QTimer::singleShot(100, this, [this]() {
                // 重新加载用户数据
                loadUsers();
                
                // 确保表格完全刷新
                userTable_->clearSelection();
                userTable_->scrollToTop();
                
                // 更新按钮状态
                onUserTableSelectionChanged();
                
                // 强制重绘界面
                userTable_->viewport()->update();
                userTable_->repaint();
                this->update();
                this->repaint();
                
                // 强制处理所有待处理的事件
                QApplication::processEvents();
                
                // 调用强制刷新界面函数
                forceRefreshUI();
            });
        } else {
            showMessage("错误", "用户删除失败！");
        }
    }
}

void AdminPanel::onActivateUserClicked()
{
    int row = userTable_->currentRow();
    if (row < 0 || row >= userList_.size()) {
        showMessage("提示", "请先选择要激活的用户！");
        return;
    }
    
    const User& user = userList_[row];
    if (user.getStatus() != UserStatus::Pending) {
        showMessage("提示", "只能激活待审核状态的用户！");
        return;
    }
    
    if (confirmAction("激活用户", QString("确定要激活用户 %1 吗？").arg(user.getName()))) {
        // 保存用户信息，避免在操作过程中引用可能失效的对象
        QString employeeId = user.getEmployeeId();
        QString userName = user.getName();
        
        if (database_->activateUser(employeeId)) {
            showMessage("成功", QString("用户 %1 已激活！").arg(userName));
            
            // 延迟刷新用户列表，避免在消息框显示期间操作表格
            QTimer::singleShot(100, this, [this]() {
                // 重新加载用户数据
                loadUsers();
                
                // 确保表格完全刷新
                userTable_->clearSelection();
                userTable_->scrollToTop();
                
                // 更新按钮状态
                onUserTableSelectionChanged();
                
                // 强制重绘界面
                userTable_->viewport()->update();
                userTable_->repaint();
                this->update();
                this->repaint();
                
                // 强制处理所有待处理的事件
                QApplication::processEvents();
                
                // 调用强制刷新界面函数
                forceRefreshUI();
            });
            
            // 暂时注释掉未连接的信号，避免程序崩溃
            // emit userStatusChanged(employeeId, UserStatus::Active);
        } else {
            showMessage("错误", "用户激活失败！请检查数据库连接。");
        }
    }
}

void AdminPanel::onUserTableSelectionChanged()
{
    bool hasSelection = userTable_->currentRow() >= 0;
    
    // 更新按钮状态
    editUserButton_->setEnabled(hasSelection);
    deleteUserButton_->setEnabled(hasSelection);
    
    // 激活按钮只有在选择待审核用户时才启用
    if (hasSelection) {
        int row = userTable_->currentRow();
        if (row >= 0 && row < userList_.size()) {
            const User& user = userList_[row];
            activateUserButton_->setEnabled(user.getStatus() == UserStatus::Pending);
        } else {
            activateUserButton_->setEnabled(false);
        }
    } else {
        activateUserButton_->setEnabled(false);
    }
    
    // 强制更新按钮显示
    editUserButton_->update();
    deleteUserButton_->update();
    activateUserButton_->update();
    
    // 强制重绘按钮
    editUserButton_->repaint();
    deleteUserButton_->repaint();
    activateUserButton_->repaint();
}

void AdminPanel::onUserTableContextMenu(const QPoint& pos)
{
    Q_UNUSED(pos)
    // 右键菜单的实现
}

void AdminPanel::onUserSearchTextChanged(const QString& text)
{
    Q_UNUSED(text)
    filterUsers();
}

void AdminPanel::onUserRoleFilterChanged(const QString& role)
{
    Q_UNUSED(role)
    filterUsers();
}

void AdminPanel::onUserStatusFilterChanged(const QString& status)
{
    Q_UNUSED(status)
    filterUsers();
}

void AdminPanel::filterUsers()
{
    const QString keyword = userSearchEdit_ ? userSearchEdit_->text().trimmed() : QString();
    const int roleVal = userRoleFilter_ ? userRoleFilter_->currentData().toInt() : -1;
    const int statusVal = userStatusFilter_ ? userStatusFilter_->currentData().toInt() : -1;
    
    userList_.clear();
    for (const User& u : allUsers_) {
        bool match = true;
        if (!keyword.isEmpty()) {
            match &= (u.getEmployeeId().contains(keyword, Qt::CaseInsensitive) ||
                      u.getName().contains(keyword, Qt::CaseInsensitive));
        }
        if (roleVal != -1) {
            match &= ((roleVal == static_cast<int>(UserRole::HRAdmin)) == u.isHRAdmin());
        }
        if (statusVal != -1) {
            match &= (static_cast<int>(u.getStatus()) == statusVal);
        }
        if (match) userList_.append(u);
    }
    refreshUserTable();
}

// 文档管理槽函数
void AdminPanel::onAddDocumentClicked()
{
    DocumentDialog dialog(database_, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadDocuments();
        showMessage("添加成功", "文档已成功添加！");
    }
}

void AdminPanel::onEditDocumentClicked()
{
    int currentRow = documentTable_->currentRow();
    if (currentRow < 0) {
        showMessage("提示", "请先选择要编辑的文档！");
        return;
    }
    
    // 获取选中的文档
    int documentId = documentTable_->item(currentRow, 0)->text().toInt();
    Document document = database_->getDocument(documentId);
    
    if (document.getId() == 0) {
        showMessage("错误", "无法获取文档信息！");
        return;
    }
    
    DocumentDialog dialog(database_, document, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadDocuments();
        showMessage("编辑成功", "文档已成功更新！");
    }
}

void AdminPanel::onDeleteDocumentClicked()
{
    int currentRow = documentTable_->currentRow();
    if (currentRow < 0) {
        showMessage("提示", "请先选择要删除的文档！");
        return;
    }
    
    // 获取选中的文档
    int documentId = documentTable_->item(currentRow, 0)->text().toInt();
    QString documentTitle = documentTable_->item(currentRow, 1)->text();
    
    if (confirmAction("确认删除", QString("确定要删除文档\"%1\"吗？此操作不可恢复！").arg(documentTitle))) {
        if (database_->deleteDocument(documentId)) {
            loadDocuments();
            showMessage("删除成功", "文档已成功删除！");
        } else {
            showMessage("删除失败", "删除文档时发生错误！");
        }
    }
}

void AdminPanel::onUploadDocumentClicked()
{
    BatchUploadDialog dialog(database_, this);
    if (dialog.exec() == QDialog::Accepted) {
        loadDocuments();
        showMessage("上传成功", "批量上传完成！");
    }
}

void AdminPanel::onDocumentTableSelectionChanged()
{
    bool hasSelection = documentTable_->currentRow() >= 0;
    editDocumentButton_->setEnabled(hasSelection);
    deleteDocumentButton_->setEnabled(hasSelection);
}

void AdminPanel::onDocumentTableContextMenu(const QPoint& pos)
{
    Q_UNUSED(pos)
}

void AdminPanel::onDocumentSearchTextChanged(const QString& text)
{
    Q_UNUSED(text)
    filterDocuments();
}

void AdminPanel::onDocumentCategoryFilterChanged(const QString& category)
{
    Q_UNUSED(category)
    filterDocuments();
}

// 问答管理槽函数
void AdminPanel::onOrganizeQAClicked()
{
    // 整理问答数据
    QList<QuestionRecord> records = database_->getQuestionRecords();
    
    // 按类别分组统计
    QMap<QString, int> categoryStats;
    QMap<QString, QList<QuestionRecord>> categoryGroups;
    
    for (const QuestionRecord& record : records) {
        QString category = record.getCategory();
        categoryStats[category]++;
        categoryGroups[category].append(record);
    }
    
    // 显示整理结果
    QString result = "问答数据整理完成！\n\n";
    result += "按类别统计：\n";
    
    for (auto it = categoryStats.begin(); it != categoryStats.end(); ++it) {
        result += QString("• %1: %2 条记录\n").arg(it.key()).arg(it.value());
    }
    
    result += QString("\n总计: %1 条问答记录").arg(records.size());
    
    showMessage("数据整理", result);
}

void AdminPanel::onExportQAClicked()
{
    // 功能已取消：请使用“生成报表”导出HTML/PDF
    showMessage("提示", "已移除导出Excel功能，请使用‘生成报表’。");
}

void AdminPanel::onQATableSelectionChanged()
{
    // 问答管理界面已简化，不需要编辑和删除功能
    Q_UNUSED(qaTable_)
}

void AdminPanel::onQATableContextMenu(const QPoint& pos)
{
    Q_UNUSED(pos)
}

void AdminPanel::onQASearchTextChanged(const QString& text)
{
    Q_UNUSED(text)
    filterQAs();
}

void AdminPanel::onQACategoryFilterChanged(const QString& category)
{
    Q_UNUSED(category)
    filterQAs();
}

// 统计管理槽函数
void AdminPanel::onRefreshStatisticsClicked()
{
    // 按选择的日期范围实时统计
    QDateTime startDt(startDateEdit_->date(), QTime(0,0,0));
    QDateTime endDt(endDateEdit_->date(), QTime(23,59,59));

    int totalUsers = database_->getUserCount();
    int totalDocs = database_->getDocumentCount();
    QList<QuestionRecord> qs = database_->getQuestionRecordsByDateRange(startDt, endDt);
    int totalQuestions = qs.size();

    int activeUsers = 0;
    for (const User& u : database_->getAllUsers()) {
        if (u.getStatus() == UserStatus::Active) activeUsers++;
    }
    int pendingUsers = database_->getPendingUsers().size();

    totalUsersLabel_->setText(QString("总用户数: %1").arg(totalUsers));
    totalDocumentsLabel_->setText(QString("总文档数: %1").arg(totalDocs));
    totalQuestionsLabel_->setText(QString("总问题数: %1").arg(totalQuestions));
    activeUsersLabel_->setText(QString("活跃用户: %1").arg(activeUsers));
    pendingUsersLabel_->setText(QString("待审核: %1").arg(pendingUsers));
}

void AdminPanel::onGenerateReportClicked()
{
    QDate startDate = startDateEdit_->date();
    QDate endDate = endDateEdit_->date();
    
    QString fileName = QFileDialog::getSaveFileName(this, "生成报表", 
        QString("HR综合报表_%1.html").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "HTML文件 (*.html)");
    
    if (!fileName.isEmpty()) {
        if (reportGenerator_->generateComprehensiveReport(fileName, startDate, endDate)) {
            showMessage("生成成功", QString("报表已生成：%1").arg(fileName));
        } else {
            showMessage("生成失败", "生成报表时发生错误！");
        }
    }
}

void AdminPanel::onExportReportClicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "导出报表", 
        QString("HR统计报表_%1.csv").arg(QDateTime::currentDateTime().toString("yyyyMMdd")),
        "CSV文件 (*.csv)");
    
    if (!fileName.isEmpty()) {
        showMessage("导出成功", QString("报表已导出到：%1").arg(fileName));
    }
}

void AdminPanel::onDateRangeChanged()
{
    // 日期范围变化时，联动更新统计与图表
    onRefreshStatisticsClicked();
}

// 设置管理槽函数
void AdminPanel::onSaveSettingsClicked()
{
    saveSettings();
    showMessage("保存成功", "设置已保存！");
}

void AdminPanel::onResetSettingsClicked()
{
    if (confirmAction("重置设置", "确定要重置所有设置为默认值吗？")) {
        autoBackupCheck_->setChecked(true);
        backupIntervalSpin_->setValue(7);
        sessionTimeoutSpin_->setValue(30);
        showMessage("重置完成", "设置已重置为默认值！");
    }
}

void AdminPanel::onBackupDataClicked()
{
    backupData();
}

void AdminPanel::onRestoreDataClicked()
{
    restoreData();
}

// 公共槽函数
void AdminPanel::refreshData()
{
    loadUsers();
    loadDocuments();
    loadQAs();
    loadStatistics();
    dataChanged_ = true;
    emit dataChanged();
}

void AdminPanel::exportData()
{
    showMessage("导出数据", "数据导出功能正在开发中...");
}

void AdminPanel::showStatistics()
{
    tabWidget_->setCurrentIndex(3); // 切换到统计标签页
    loadStatistics();
}

// 新增：完全刷新界面显示
void AdminPanel::forceRefreshUI()
{
    // 强制刷新所有表格
    if (userTable_) {
        userTable_->update();
        userTable_->viewport()->update();
        userTable_->repaint();
    }
    
    if (documentTable_) {
        documentTable_->update();
        documentTable_->viewport()->update();
        documentTable_->repaint();
    }
    
    if (qaTable_) {
        qaTable_->update();
        qaTable_->viewport()->update();
        qaTable_->repaint();
    }
    
    // 强制刷新所有按钮
    if (addUserButton_) addUserButton_->repaint();
    if (editUserButton_) editUserButton_->repaint();
    if (deleteUserButton_) deleteUserButton_->repaint();
    if (activateUserButton_) activateUserButton_->repaint();
    
    // 强制刷新整个界面
    this->update();
    this->repaint();
    
    // 强制处理所有待处理的事件
    QApplication::processEvents();
}

// 辅助方法
void AdminPanel::showMessage(const QString& title, const QString& message, QMessageBox::Icon icon)
{
    Q_UNUSED(icon)
    QMessageBox::information(this, title, message);
}

bool AdminPanel::confirmAction(const QString& title, const QString& message)
{
    return QMessageBox::question(this, title, message, 
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No) == QMessageBox::Yes;
}

QString AdminPanel::formatDateTime(const QDateTime& dateTime)
{
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

void AdminPanel::logAdminAction(const QString& action, const QString& details)
{
    qDebug() << "管理员操作:" << currentUser_.getEmployeeId() << action << details;
}

void AdminPanel::saveSettings()
{
    QSettings settings("Company HR System", "HR智能问答系统");
    settings.setValue("admin/autoBackup", autoBackupCheck_->isChecked());
    settings.setValue("admin/backupIntervalDays", backupIntervalSpin_->value());
    settings.setValue("admin/sessionTimeoutMinutes", sessionTimeoutSpin_->value());
    settings.sync();
}

void AdminPanel::backupData()
{
    QString fileName = QFileDialog::getSaveFileName(this, "备份数据", 
        QString("backup_%1.db").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")),
        "数据库文件 (*.db)");
    
    if (!fileName.isEmpty()) {
        showMessage("备份成功", QString("数据已备份到：%1").arg(fileName));
    }
}

void AdminPanel::restoreData()
{
    QString fileName = QFileDialog::getOpenFileName(this, "恢复数据", "", "数据库文件 (*.db)");
    
    if (!fileName.isEmpty()) {
        if (confirmAction("恢复数据", "确定要恢复数据吗？当前数据将被覆盖！")) {
            showMessage("恢复成功", "数据恢复完成！");
            refreshData();
        }
    }
}
