#include "relatedclausesdialog.h"
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QHeaderView>
#include <QTimer>
#include <QDesktopServices>
#include <QUrl>

RelatedClausesDialog::RelatedClausesDialog(const QString& question, DocumentProcessor* processor, QWidget *parent)
    : QDialog(parent)
    , question_(question)
    , processor_(processor)
{
    setWindowTitle("相关条款");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(900, 600);
    
    setupUI();
    searchRelatedClauses();
}

RelatedClausesDialog::~RelatedClausesDialog()
{
}

void RelatedClausesDialog::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(15);
    
    // 问题标题
    QLabel* questionLabel = new QLabel(QString("问题: %1").arg(question_));
    questionLabel->setStyleSheet(
        "font-size: 16px; font-weight: bold; color: #2c3e50; "
        "padding: 10px; background: #ecf0f1; border-radius: 5px;"
    );
    questionLabel->setWordWrap(true);
    mainLayout_->addWidget(questionLabel);
    
    // 分割器
    splitter_ = new QSplitter(Qt::Horizontal);
    mainLayout_->addWidget(splitter_);
    
    // 左侧条款列表
    setupClauseList();
    
    // 右侧详细内容
    setupContentView();
    
    // 按钮区域
    setupButtons();
    
    // 设置分割器比例
    splitter_->setStretchFactor(0, 1);
    splitter_->setStretchFactor(1, 2);
    
    // 应用样式
    setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 5px;"
        "    margin: 5px 0px;"
        "    padding-top: 15px;"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 4px 8px;"
        "    color: #2c3e50;"
        "}"
        "QListWidget {"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 3px;"
        "    background: white;"
        "    selection-background-color: #3498db;"
        "    outline: none;"
        "}"
        "QListWidget::item {"
        "    padding: 10px;"
        "    border-bottom: 1px solid #ecf0f1;"
        "}"
        "QListWidget::item:selected {"
        "    background: #3498db;"
        "    color: white;"
        "}"
        "QTextBrowser {"
        "    border: 1px solid #bdc3c7;"
        "    border-radius: 3px;"
        "    background: white;"
        "    font-size: 13px;"
        "    line-height: 1.5;"
        "}"
        "QPushButton {"
        "    padding: 8px 16px;"
        "    border: 1px solid #3498db;"
        "    border-radius: 4px;"
        "    background: #3498db;"
        "    color: white;"
        "    font-weight: 500;"
        "}"
        "QPushButton:hover {"
        "    background: #2980b9;"
        "    border-color: #2980b9;"
        "}"
        "QPushButton:pressed {"
        "    background: #21618c;"
        "}"
        "QPushButton:disabled {"
        "    background: #95a5a6;"
        "    border-color: #95a5a6;"
        "    color: #ecf0f1;"
        "}"
    );
}

void RelatedClausesDialog::setupClauseList()
{
    clauseListGroup_ = new QGroupBox("相关条款列表");
    QVBoxLayout* listLayout = new QVBoxLayout(clauseListGroup_);
    
    // 搜索状态
    statusLabel_ = new QLabel("正在搜索相关条款...");
    statusLabel_->setStyleSheet("color: #7f8c8d; font-size: 12px; padding: 5px;");
    listLayout->addWidget(statusLabel_);
    
    // 搜索进度
    searchProgress_ = new QProgressBar();
    searchProgress_->setRange(0, 0); // 不确定进度
    searchProgress_->setTextVisible(false);
    searchProgress_->setMaximumHeight(4);
    listLayout->addWidget(searchProgress_);
    
    // 条款列表
    clauseList_ = new QListWidget();
    clauseList_->setAlternatingRowColors(true);
    connect(clauseList_, &QListWidget::itemSelectionChanged, 
            this, &RelatedClausesDialog::onClauseSelected);
    listLayout->addWidget(clauseList_);
    
    splitter_->addWidget(clauseListGroup_);
}

void RelatedClausesDialog::setupContentView()
{
    contentGroup_ = new QGroupBox("条款详细内容");
    QVBoxLayout* contentLayout = new QVBoxLayout(contentGroup_);
    
    // 内容浏览器
    contentBrowser_ = new QTextBrowser();
    contentBrowser_->setPlainText("请从左侧列表选择一个条款查看详细内容。");
    contentLayout->addWidget(contentBrowser_);
    
    // 信息标签
    QHBoxLayout* infoLayout = new QHBoxLayout();
    
    sourceLabel_ = new QLabel("来源: 未选择");
    sourceLabel_->setStyleSheet("color: #7f8c8d; font-size: 11px;");
    infoLayout->addWidget(sourceLabel_);
    
    infoLayout->addStretch();
    
    relevanceLabel_ = new QLabel("相关度: --");
    relevanceLabel_->setStyleSheet("color: #7f8c8d; font-size: 11px;");
    infoLayout->addWidget(relevanceLabel_);
    
    contentLayout->addLayout(infoLayout);
    
    splitter_->addWidget(contentGroup_);
}

void RelatedClausesDialog::setupButtons()
{
    buttonLayout_ = new QHBoxLayout();
    
    viewFullButton_ = new QPushButton("查看完整文档");
    viewFullButton_->setEnabled(false);
    connect(viewFullButton_, &QPushButton::clicked, 
            this, &RelatedClausesDialog::onViewFullDocument);
    buttonLayout_->addWidget(viewFullButton_);
    
    copyButton_ = new QPushButton("复制条款内容");
    copyButton_->setEnabled(false);
    connect(copyButton_, &QPushButton::clicked, 
            this, &RelatedClausesDialog::onCopyClause);
    buttonLayout_->addWidget(copyButton_);
    
    buttonLayout_->addStretch();
    
    closeButton_ = new QPushButton("关闭");
    closeButton_->setStyleSheet(
        "QPushButton {"
        "    background: #95a5a6;"
        "    border-color: #95a5a6;"
        "}"
        "QPushButton:hover {"
        "    background: #7f8c8d;"
        "    border-color: #7f8c8d;"
        "}"
    );
    connect(closeButton_, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout_->addWidget(closeButton_);
    
    mainLayout_->addLayout(buttonLayout_);
}

void RelatedClausesDialog::searchRelatedClauses()
{
    if (!processor_) {
        statusLabel_->setText("搜索服务不可用");
        searchProgress_->hide();
        return;
    }
    
    // 模拟搜索过程
    QTimer::singleShot(1000, this, &RelatedClausesDialog::onSearchCompleted);
}

void RelatedClausesDialog::onSearchCompleted()
{
    searchProgress_->hide();
    
    // 模拟生成一些相关条款
    clauses_.clear();
    
    // 根据问题内容生成模拟的相关条款
    if (question_.contains("工作时间") || question_.contains("上班") || question_.contains("下班")) {
        RelatedClause clause1;
        clause1.id = "work_time_001";
        clause1.title = "公司工作时间规定";
        clause1.content = "根据《员工手册》第3.1条规定：\n\n"
                         "1. 标准工作时间为每周一至周五，上午9:00-12:00，下午13:30-17:30\n"
                         "2. 弹性工作时间为上午8:30-10:00可以弹性上班\n"
                         "3. 加班需要提前申请并获得部门主管批准\n"
                         "4. 法定节假日按国家规定执行";
        clause1.source = "员工手册 v2.1";
        clause1.relevanceScore = 0.95;
        clause1.category = "工作制度";
        clause1.lastModified = QDateTime::currentDateTime().addDays(-30);
        clauses_.append(clause1);
        
        RelatedClause clause2;
        clause2.id = "work_time_002";
        clause2.title = "考勤管理制度";
        clause2.content = "考勤管理相关规定：\n\n"
                         "1. 员工需通过考勤系统打卡记录出勤\n"
                         "2. 迟到早退按相关规定处理\n"
                         "3. 病假、事假需要提前申请\n"
                         "4. 连续旷工3天以上按自动离职处理";
        clause2.source = "考勤管理制度 v1.3";
        clause2.relevanceScore = 0.87;
        clause2.category = "考勤制度";
        clause2.lastModified = QDateTime::currentDateTime().addDays(-15);
        clauses_.append(clause2);
    }
    
    if (question_.contains("年假") || question_.contains("假期") || question_.contains("休假")) {
        RelatedClause clause3;
        clause3.id = "leave_001";
        clause3.title = "年假申请管理办法";
        clause3.content = "年假申请相关规定：\n\n"
                         "1. 工作满1年的员工享有5天年假\n"
                         "2. 工作满5年的员工享有10天年假\n"
                         "3. 工作满10年的员工享有15天年假\n"
                         "4. 年假需提前一周申请，特殊情况可紧急申请";
        clause3.source = "福利管理制度 v2.0";
        clause3.relevanceScore = 0.92;
        clause3.category = "福利制度";
        clause3.lastModified = QDateTime::currentDateTime().addDays(-60);
        clauses_.append(clause3);
    }
    
    if (clauses_.isEmpty()) {
        // 通用条款
        RelatedClause defaultClause;
        defaultClause.id = "general_001";
        defaultClause.title = "员工权利与义务";
        defaultClause.content = "根据公司章程规定：\n\n"
                               "员工权利：\n"
                               "1. 获得劳动报酬的权利\n"
                               "2. 获得劳动保护的权利\n"
                               "3. 参与民主管理的权利\n\n"
                               "员工义务：\n"
                               "1. 遵守公司规章制度\n"
                               "2. 保守公司商业秘密\n"
                               "3. 维护公司利益和形象";
        defaultClause.source = "公司章程";
        defaultClause.relevanceScore = 0.65;
        defaultClause.category = "基本制度";
        defaultClause.lastModified = QDateTime::currentDateTime().addDays(-90);
        clauses_.append(defaultClause);
    }
    
    displayClauses(clauses_);
}

void RelatedClausesDialog::displayClauses(const QList<RelatedClause>& clauses)
{
    clauseList_->clear();
    
    if (clauses.isEmpty()) {
        statusLabel_->setText("未找到相关条款");
        return;
    }
    
    statusLabel_->setText(QString("找到 %1 个相关条款").arg(clauses.size()));
    
    for (const RelatedClause& clause : clauses) {
        QListWidgetItem* item = new QListWidgetItem();
        
        QString itemText = QString("%1\n来源: %2\n相关度: %3%")
                          .arg(clause.title)
                          .arg(clause.source)
                          .arg(QString::number(clause.relevanceScore * 100, 'f', 0));
        
        item->setText(itemText);
        item->setData(Qt::UserRole, clause.id);
        
        // 根据相关度设置颜色
        if (clause.relevanceScore >= 0.9) {
            item->setBackground(QBrush(QColor(231, 76, 60, 30))); // 高相关度-红色
        } else if (clause.relevanceScore >= 0.8) {
            item->setBackground(QBrush(QColor(230, 126, 34, 30))); // 中等相关度-橙色
        } else {
            item->setBackground(QBrush(QColor(149, 165, 166, 30))); // 低相关度-灰色
        }
        
        clauseList_->addItem(item);
    }
}

void RelatedClausesDialog::onClauseSelected()
{
    QListWidgetItem* currentItem = clauseList_->currentItem();
    if (!currentItem) {
        return;
    }
    
    QString clauseId = currentItem->data(Qt::UserRole).toString();
    
    // 查找对应的条款
    for (const RelatedClause& clause : clauses_) {
        if (clause.id == clauseId) {
            currentClause_ = clause;
            formatClauseContent(clause);
            
            // 启用按钮
            viewFullButton_->setEnabled(true);
            copyButton_->setEnabled(true);
            break;
        }
    }
}

void RelatedClausesDialog::formatClauseContent(const RelatedClause& clause)
{
    QString content = clause.content;
    content.replace("\n", "<br>");
    
    QString html = QString(
        "<h3 style='color: #2c3e50; margin-bottom: 15px;'>%1</h3>"
        "<div style='background: #f8f9fa; padding: 15px; border-left: 4px solid #3498db; margin: 10px 0;'>"
        "%2"
        "</div>"
        "<p style='margin-top: 20px; font-size: 12px; color: #7f8c8d;'>"
        "<strong>分类：</strong>%3 | "
        "<strong>最后更新：</strong>%4"
        "</p>"
    ).arg(clause.title)
     .arg(content)
     .arg(clause.category)
     .arg(clause.lastModified.toString("yyyy-MM-dd"));
    
    contentBrowser_->setHtml(html);
    
    // 更新信息标签
    sourceLabel_->setText(QString("来源: %1").arg(clause.source));
    relevanceLabel_->setText(QString("相关度: %1%").arg(QString::number(clause.relevanceScore * 100, 'f', 0)));
}

void RelatedClausesDialog::onViewFullDocument()
{
    if (currentClause_.id.isEmpty()) return;
    
    QMessageBox::information(this, "查看完整文档", 
        QString("将在浏览器中打开完整文档：\n%1\n\n文档ID：%2")
        .arg(currentClause_.source)
        .arg(currentClause_.id));
}

void RelatedClausesDialog::onCopyClause()
{
    if (currentClause_.id.isEmpty()) return;
    
    QString copyText = QString("%1\n\n%2\n\n来源：%3\n相关度：%4%")
                      .arg(currentClause_.title)
                      .arg(currentClause_.content)
                      .arg(currentClause_.source)
                      .arg(QString::number(currentClause_.relevanceScore * 100, 'f', 0));
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(copyText);
    
    QMessageBox::information(this, "复制成功", "条款内容已复制到剪贴板！");
}
