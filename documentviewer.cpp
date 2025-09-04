#include "documentviewer.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include <QScrollBar>
#include <QMimeData>
#include <QDesktopServices>
#include <QStringConverter>
#include <QRegularExpression>
#include <QClipboard>

DocumentViewer::DocumentViewer(Database* database, QWidget *parent)
    : QWidget(parent)
    , database_(database)
    , currentDocument_()
    , mainLayout_(nullptr)
    , mainSplitter_(nullptr)
    , outlinePanel_(nullptr)
    , outlineLayout_(nullptr)
    , outlineTitleLabel_(nullptr)
    , outlineTree_(nullptr)
    , expandAllButton_(nullptr)
    , collapseAllButton_(nullptr)
    , documentPanel_(nullptr)
    , documentLayout_(nullptr)
    , toolbarLayout_(nullptr)
    , searchLabel_(nullptr)
    , searchEdit_(nullptr)
    , searchButton_(nullptr)
    , nextButton_(nullptr)
    , previousButton_(nullptr)
    , zoomLabel_(nullptr)
    , zoomInButton_(nullptr)
    , zoomOutButton_(nullptr)
    , zoomResetButton_(nullptr)
    , printButton_(nullptr)
    , exportButton_(nullptr)
    , documentBrowser_(nullptr)
    , documentInfoLabel_(nullptr)
    , searchPositions_()
    , currentSearchIndex_(-1)
    , zoomLevel_(1.0)
    , documentSections_()
    , sectionPositions_()
{
    setupUI();
    setupConnections();
    applyStyles();
    setupTextFormats();
}

DocumentViewer::~DocumentViewer()
{
}

void DocumentViewer::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(10, 10, 10, 10);
    mainLayout_->setSpacing(10);
    
    // 创建分割器
    mainSplitter_ = new QSplitter(Qt::Horizontal);
    mainSplitter_->setChildrenCollapsible(false);
    
    setupOutline();
    setupDocumentBrowser();
    
    // 添加到分割器
    mainSplitter_->addWidget(outlinePanel_);
    mainSplitter_->addWidget(documentPanel_);
    mainSplitter_->setStretchFactor(0, 0);
    mainSplitter_->setStretchFactor(1, 1);
    mainSplitter_->setSizes({250, 750});
    
    mainLayout_->addWidget(mainSplitter_);
}

void DocumentViewer::setupOutline()
{
    // 左侧大纲面板
    outlinePanel_ = new QWidget();
    outlinePanel_->setMinimumWidth(200);
    outlinePanel_->setMaximumWidth(350);
    outlineLayout_ = new QVBoxLayout(outlinePanel_);
    outlineLayout_->setSpacing(10);
    
    // 大纲标题
    outlineTitleLabel_ = new QLabel("文档大纲");
    outlineTitleLabel_->setStyleSheet("font-size: 14px; font-weight: bold; color: #2c3e50; padding: 5px;");
    outlineLayout_->addWidget(outlineTitleLabel_);
    
    // 大纲树
    outlineTree_ = new QTreeWidget();
    outlineTree_->setHeaderHidden(true);
    outlineTree_->setRootIsDecorated(true);
    outlineTree_->setAlternatingRowColors(true);
    outlineTree_->setContextMenuPolicy(Qt::CustomContextMenu);
    outlineLayout_->addWidget(outlineTree_);
    
    // 大纲操作按钮
    QHBoxLayout* outlineButtonLayout = new QHBoxLayout();
    expandAllButton_ = new QPushButton("展开全部");
    collapseAllButton_ = new QPushButton("折叠全部");
    expandAllButton_->setMaximumWidth(80);
    collapseAllButton_->setMaximumWidth(80);
    
    outlineButtonLayout->addWidget(expandAllButton_);
    outlineButtonLayout->addWidget(collapseAllButton_);
    outlineLayout_->addLayout(outlineButtonLayout);
}

void DocumentViewer::setupDocumentBrowser()
{
    // 右侧文档显示区域
    documentPanel_ = new QWidget();
    documentLayout_ = new QVBoxLayout(documentPanel_);
    documentLayout_->setSpacing(10);
    
    setupToolbar();
    
    // 文档浏览器
    documentBrowser_ = new QTextBrowser();
    documentBrowser_->setOpenExternalLinks(false);
    documentBrowser_->setOpenLinks(false);
    documentBrowser_->setReadOnly(true);
    documentBrowser_->setPlaceholderText("请选择要查看的文档...");
    documentLayout_->addWidget(documentBrowser_);
    
    // 文档信息栏
    documentInfoLabel_ = new QLabel();
    documentInfoLabel_->setStyleSheet("background: #ecf0f1; padding: 5px; border-radius: 3px; font-size: 12px; color: #7f8c8d;");
    documentInfoLabel_->setVisible(false);
    documentLayout_->addWidget(documentInfoLabel_);
}

void DocumentViewer::setupToolbar()
{
    // 工具栏
    toolbarLayout_ = new QHBoxLayout();
    
    // 搜索组件
    searchLabel_ = new QLabel("搜索:");
    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText("输入关键词搜索文档内容...");
    searchEdit_->setMaximumWidth(200);
    searchButton_ = new QPushButton("搜索");
    searchButton_->setObjectName("searchButton");
    nextButton_ = new QPushButton("下一个");
    previousButton_ = new QPushButton("上一个");
    
    searchButton_->setMaximumWidth(60);
    nextButton_->setMaximumWidth(60);
    previousButton_->setMaximumWidth(60);
    nextButton_->setEnabled(false);
    previousButton_->setEnabled(false);
    
    // 缩放组件
    zoomLabel_ = new QLabel("缩放:");
    zoomInButton_ = new QPushButton("放大");
    zoomOutButton_ = new QPushButton("缩小");
    zoomResetButton_ = new QPushButton("重置");
    
    zoomInButton_->setMaximumWidth(50);
    zoomOutButton_->setMaximumWidth(50);
    zoomResetButton_->setMaximumWidth(50);
    
    // 操作按钮
    printButton_ = new QPushButton("打印");
    exportButton_ = new QPushButton("导出");
    
    printButton_->setMaximumWidth(50);
    exportButton_->setMaximumWidth(50);
    
    // 添加到工具栏布局
    toolbarLayout_->addWidget(searchLabel_);
    toolbarLayout_->addWidget(searchEdit_);
    toolbarLayout_->addWidget(searchButton_);
    toolbarLayout_->addWidget(previousButton_);
    toolbarLayout_->addWidget(nextButton_);
    toolbarLayout_->addWidget(new QLabel("|")); // 分隔符
    toolbarLayout_->addWidget(zoomLabel_);
    toolbarLayout_->addWidget(zoomOutButton_);
    toolbarLayout_->addWidget(zoomResetButton_);
    toolbarLayout_->addWidget(zoomInButton_);
    toolbarLayout_->addWidget(new QLabel("|")); // 分隔符
    toolbarLayout_->addWidget(printButton_);
    toolbarLayout_->addWidget(exportButton_);
    toolbarLayout_->addStretch();
    
    documentLayout_->addLayout(toolbarLayout_);
}

void DocumentViewer::setupConnections()
{
    // 搜索相关连接
    connect(searchButton_, &QPushButton::clicked, this, &DocumentViewer::onSearchButtonClicked);
    connect(nextButton_, &QPushButton::clicked, this, &DocumentViewer::onNextButtonClicked);
    connect(previousButton_, &QPushButton::clicked, this, &DocumentViewer::onPreviousButtonClicked);
    connect(searchEdit_, &QLineEdit::textChanged, this, &DocumentViewer::onSearchTextChanged);
    connect(searchEdit_, &QLineEdit::returnPressed, this, &DocumentViewer::onSearchButtonClicked);
    
    // 缩放相关连接
    connect(zoomInButton_, &QPushButton::clicked, this, &DocumentViewer::onZoomInClicked);
    connect(zoomOutButton_, &QPushButton::clicked, this, &DocumentViewer::onZoomOutClicked);
    connect(zoomResetButton_, &QPushButton::clicked, this, &DocumentViewer::onZoomResetClicked);
    
    // 操作按钮连接
    connect(printButton_, &QPushButton::clicked, this, &DocumentViewer::onPrintClicked);
    connect(exportButton_, &QPushButton::clicked, this, &DocumentViewer::onExportClicked);
    
    // 大纲相关连接
    connect(outlineTree_, &QTreeWidget::itemClicked, this, &DocumentViewer::onOutlineItemClicked);
    connect(outlineTree_, &QTreeWidget::customContextMenuRequested, this, &DocumentViewer::onOutlineContextMenu);
    connect(expandAllButton_, &QPushButton::clicked, outlineTree_, &QTreeWidget::expandAll);
    connect(collapseAllButton_, &QPushButton::clicked, outlineTree_, &QTreeWidget::collapseAll);
    
    // 文档浏览器连接
    connect(documentBrowser_, &QTextBrowser::anchorClicked, this, &DocumentViewer::onDocumentLinkClicked);
    connect(documentBrowser_, &QTextBrowser::selectionChanged, this, &DocumentViewer::onTextSelectionChanged);
}

void DocumentViewer::applyStyles()
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
        "QTreeWidget {"
        "    border: 1px solid #e1e8ed;"
        "    border-radius: 5px;"
        "    background: white;"
        "}"
        "QTreeWidget::item {"
        "    padding: 3px;"
        "    border-radius: 3px;"
        "}"
        "QTreeWidget::item:hover {"
        "    background: #e3f2fd;"
        "}"
        "QTreeWidget::item:selected {"
        "    background: #3498db;"
        "    color: white;"
        "}"
        "QTextBrowser {"
        "    border: 1px solid #e1e8ed;"
        "    border-radius: 5px;"
        "    background: white;"
        "    padding: 10px;"
        "}"
    );
}

void DocumentViewer::setupTextFormats()
{
    // 设置文本格式
    highlightFormat_.setBackground(QColor(255, 255, 0, 100)); // 黄色高亮
    searchHighlightFormat_.setBackground(QColor(255, 165, 0, 150)); // 橙色搜索高亮
    
    originalFont_ = documentBrowser_->font();
}

void DocumentViewer::loadDocument(const Document& document)
{
    currentDocument_ = document;
    loadDocumentContent();
    parseDocumentStructure();
    buildOutlineTree();
    updateDocumentInfo();
    
    emit documentLoaded(document);
}

void DocumentViewer::loadDocumentById(int documentId)
{
    Document document = database_->getDocument(documentId);
    if (document.getId() > 0) {
        loadDocument(document);
    } else {
        showMessage("错误", "无法加载文档，文档不存在。", QMessageBox::Warning);
    }
}

void DocumentViewer::loadDocumentContent()
{
    if (currentDocument_.getId() <= 0) {
        documentBrowser_->clear();
        return;
    }
    
    QString formattedContent = formatDocumentContent();
    documentBrowser_->setHtml(formattedContent);
    applyZoom();
}

void DocumentViewer::parseDocumentStructure()
{
    documentSections_.clear();
    sectionPositions_.clear();
    
    if (currentDocument_.getId() <= 0) {
        return;
    }
    
    QString content = currentDocument_.getContent();
    QStringList lines = content.split('\n');
    
    int position = 0;
    for (int i = 0; i < lines.size(); ++i) {
        QString line = lines[i].trimmed();
        
        // 检测章节标题（以"第"开头或包含"章"、"条"等）
        if (line.contains(QRegularExpression("^第.+章")) ||
            line.contains(QRegularExpression("^第.+条")) ||
            line.contains(QRegularExpression("^\\d+\\."))) {
            
            documentSections_.append(qMakePair(line, position));
            sectionPositions_[line] = position;
        }
        
        position += lines[i].length() + 1; // +1 for newline
    }
}

void DocumentViewer::buildOutlineTree()
{
    outlineTree_->clear();
    
    if (documentSections_.isEmpty()) {
        return;
    }
    
    QTreeWidgetItem* rootItem = nullptr;
    QTreeWidgetItem* chapterItem = nullptr;
    
    Q_UNUSED(rootItem) // 暂时未使用，为未来功能预留
    
    for (const auto& section : documentSections_) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, section.first);
        item->setData(0, Qt::UserRole, section.second);
        
        if (section.first.contains("章")) {
            // 章节
            chapterItem = item;
            outlineTree_->addTopLevelItem(item);
        } else if (section.first.contains("条") && chapterItem) {
            // 条款
            chapterItem->addChild(item);
        } else {
            // 其他项目
            outlineTree_->addTopLevelItem(item);
        }
    }
    
    outlineTree_->expandAll();
}

void DocumentViewer::searchInDocument(const QString& keyword)
{
    searchEdit_->setText(keyword);
    onSearchButtonClicked();
}

void DocumentViewer::highlightSection(int section)
{
    if (section >= 0 && section < documentSections_.size()) {
        highlightSectionInText(section);
        emit sectionHighlighted(section);
    }
}

void DocumentViewer::onSearchTextChanged(const QString& text)
{
    if (text.isEmpty()) {
        clearSearchHighlights();
        updateSearchButtons();
    }
}

void DocumentViewer::onSearchButtonClicked()
{
    QString searchText = searchEdit_->text().trimmed();
    if (searchText.isEmpty()) {
        return;
    }
    
    clearSearchHighlights();
    highlightSearchResults();
    
    if (!searchPositions_.isEmpty()) {
        currentSearchIndex_ = 0;
        scrollToPosition(searchPositions_[0]);
        emit searchResultFound(searchText, searchPositions_[0]);
    }
    
    updateSearchButtons();
}

void DocumentViewer::onNextButtonClicked()
{
    findNextSearchResult();
}

void DocumentViewer::onPreviousButtonClicked()
{
    findPreviousSearchResult();
}

void DocumentViewer::onZoomInClicked()
{
    zoomLevel_ = qMin(zoomLevel_ * 1.2, 3.0);
    applyZoom();
}

void DocumentViewer::onZoomOutClicked()
{
    zoomLevel_ = qMax(zoomLevel_ / 1.2, 0.5);
    applyZoom();
}

void DocumentViewer::onZoomResetClicked()
{
    zoomLevel_ = 1.0;
    applyZoom();
}

void DocumentViewer::onPrintClicked()
{
    printDocument();
}

void DocumentViewer::onExportClicked()
{
    exportDocument();
}

void DocumentViewer::onOutlineItemClicked(QTreeWidgetItem* item, int column)
{
    Q_UNUSED(column)
    
    if (item) {
        bool ok;
        int position = item->data(0, Qt::UserRole).toInt(&ok);
        if (ok) {
            scrollToPosition(position);
            highlightSectionInText(-1); // 清除之前的高亮
        }
    }
}

void DocumentViewer::onOutlineContextMenu(const QPoint& pos)
{
    QTreeWidgetItem* item = outlineTree_->itemAt(pos);
    if (!item) {
        return;
    }
    
    QMenu contextMenu(this);
    
    QAction* jumpAction = contextMenu.addAction("跳转到此处");
    QAction* copyAction = contextMenu.addAction("复制标题");
    
    QAction* selectedAction = contextMenu.exec(outlineTree_->mapToGlobal(pos));
    
    if (selectedAction == jumpAction) {
        onOutlineItemClicked(item, 0);
    } else if (selectedAction == copyAction) {
        QApplication::clipboard()->setText(item->text(0));
    }
}

void DocumentViewer::onDocumentLinkClicked(const QUrl& url)
{
    QString link = url.toString();
    
    if (link.startsWith("#section_")) {
        // 内部链接，跳转到指定章节
        QString sectionName = link.mid(9);
        if (sectionPositions_.contains(sectionName)) {
            scrollToPosition(sectionPositions_[sectionName]);
        }
    } else {
        // 外部链接
        QDesktopServices::openUrl(url);
    }
}

void DocumentViewer::onTextSelectionChanged()
{
    // 处理文本选择变化
    QString selectedText = documentBrowser_->textCursor().selectedText();
    if (!selectedText.isEmpty()) {
        // 可以在这里添加选择文本的处理逻辑
    }
}

void DocumentViewer::highlightSearchResults()
{
    QString searchText = searchEdit_->text().trimmed();
    if (searchText.isEmpty()) {
        return;
    }
    
    searchPositions_.clear();
    
    QTextDocument* document = documentBrowser_->document();
    QTextCursor cursor(document);
    
    // 查找所有匹配位置
    while (!cursor.isNull() && !cursor.atEnd()) {
        cursor = document->find(searchText, cursor);
        if (!cursor.isNull()) {
            searchPositions_.append(cursor.position());
            
            // 应用搜索高亮格式
            cursor.mergeCharFormat(searchHighlightFormat_);
        }
    }
}

void DocumentViewer::clearSearchHighlights()
{
    searchPositions_.clear();
    currentSearchIndex_ = -1;
    
    // 清除所有高亮
    QTextCursor cursor = documentBrowser_->textCursor();
    cursor.select(QTextCursor::Document);
    cursor.setCharFormat(QTextCharFormat()); // 清除格式
    
    // 重新加载文档内容
    loadDocumentContent();
}

void DocumentViewer::highlightSectionInText(int section)
{
    if (section < 0 || section >= documentSections_.size()) {
        return;
    }
    
    // 高亮指定章节
    int position = documentSections_[section].second;
    scrollToPosition(position);
}

void DocumentViewer::updateDocumentInfo()
{
    if (currentDocument_.getId() <= 0) {
        documentInfoLabel_->setVisible(false);
        return;
    }
    
    QString info = QString("文档: %1 | 类别: %2 | 创建时间: %3 | 更新时间: %4")
                   .arg(currentDocument_.getTitle())
                   .arg(currentDocument_.getCategory())
                   .arg(formatDateTime(currentDocument_.getCreateTime()))
                   .arg(formatDateTime(currentDocument_.getUpdateTime()));
    
    documentInfoLabel_->setText(info);
    documentInfoLabel_->setVisible(true);
}

void DocumentViewer::updateSearchButtons()
{
    bool hasResults = !searchPositions_.isEmpty();
    nextButton_->setEnabled(hasResults && currentSearchIndex_ < searchPositions_.size() - 1);
    previousButton_->setEnabled(hasResults && currentSearchIndex_ > 0);
}

void DocumentViewer::updateZoomButtons()
{
    zoomInButton_->setEnabled(zoomLevel_ < 3.0);
    zoomOutButton_->setEnabled(zoomLevel_ > 0.5);
}

void DocumentViewer::applyZoom()
{
    QFont font = originalFont_;
    font.setPointSizeF(font.pointSizeF() * zoomLevel_);
    documentBrowser_->setFont(font);
    
    updateZoomButtons();
}

void DocumentViewer::findNextSearchResult()
{
    if (searchPositions_.isEmpty()) {
        return;
    }
    
    currentSearchIndex_ = qMin(currentSearchIndex_ + 1, searchPositions_.size() - 1);
    scrollToPosition(searchPositions_[currentSearchIndex_]);
    updateSearchButtons();
}

void DocumentViewer::findPreviousSearchResult()
{
    if (searchPositions_.isEmpty()) {
        return;
    }
    
    currentSearchIndex_ = qMax(currentSearchIndex_ - 1, 0);
    scrollToPosition(searchPositions_[currentSearchIndex_]);
    updateSearchButtons();
}

void DocumentViewer::scrollToPosition(int position)
{
    QTextCursor cursor = documentBrowser_->textCursor();
    cursor.setPosition(position);
    documentBrowser_->setTextCursor(cursor);
    documentBrowser_->ensureCursorVisible();
}

void DocumentViewer::exportDocument()
{
    if (currentDocument_.getId() <= 0) {
        showMessage("提示", "没有文档可以导出。", QMessageBox::Information);
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "导出文档", 
        QString("%1.html").arg(currentDocument_.getTitle()),
        "HTML文档 (*.html);;纯文本 (*.txt);;PDF文档 (*.pdf)");
    
    if (!fileName.isEmpty()) {
        if (fileName.endsWith(".html")) {
            exportToHTML();
        } else if (fileName.endsWith(".txt")) {
            exportToText();
        } else if (fileName.endsWith(".pdf")) {
            exportToPDF();
        }
        
        showMessage("导出成功", QString("文档已导出到：%1").arg(fileName));
    }
}

void DocumentViewer::printDocument()
{
    if (currentDocument_.getId() <= 0) {
        showMessage("提示", "没有文档可以打印。", QMessageBox::Information);
        return;
    }
    
    printDocumentContent();
}

void DocumentViewer::exportToPDF()
{
    // PDF导出功能（简化实现）
    showMessage("提示", "PDF导出功能正在开发中...", QMessageBox::Information);
}

void DocumentViewer::exportToHTML()
{
    // HTML导出功能
    QString fileName = QFileDialog::getSaveFileName(this, "导出HTML", 
        QString("%1.html").arg(currentDocument_.getTitle()),
        "HTML文档 (*.html)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            stream << documentBrowser_->toHtml();
        }
    }
}

void DocumentViewer::exportToText()
{
    // 文本导出功能
    QString fileName = QFileDialog::getSaveFileName(this, "导出文本", 
        QString("%1.txt").arg(currentDocument_.getTitle()),
        "文本文档 (*.txt)");
    
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream.setEncoding(QStringConverter::Utf8);
            stream << documentBrowser_->toPlainText();
        }
    }
}

void DocumentViewer::printDocumentContent()
{
    // 打印功能（简化实现）
    showMessage("提示", "打印功能正在开发中...", QMessageBox::Information);
}

QString DocumentViewer::formatDocumentContent()
{
    if (currentDocument_.getId() <= 0) {
        return "";
    }
    
    QString content = currentDocument_.getContent();
    
    // 检查内容是否为空
    if (content.isEmpty()) {
        return QString(
            "<html><head>"
            "<style>"
            "body { font-family: 'Microsoft YaHei', Arial, sans-serif; line-height: 1.6; color: #7f8c8d; }"
            "</style>"
            "</head><body>"
            "<h1>%1</h1>"
            "<p><strong>类别：</strong>%2</p>"
            "<hr>"
            "<p style='text-align: center; font-size: 16px; margin-top: 50px;'>文档内容为空</p>"
            "</body></html>"
        ).arg(currentDocument_.getTitle())
         .arg(currentDocument_.getCategory());
    }
    
    // 改进的HTML格式化
    content.replace("\n", "<br>");
    
    // 更精确的标题匹配
    content.replace(QRegularExpression("(第.+章)"), "<h2 style='color: #2c3e50; border-bottom: 2px solid #3498db; padding-bottom: 5px; margin-top: 30px;'>\\1</h2>");
    content.replace(QRegularExpression("(第.+条)"), "<h3 style='color: #34495e; margin-top: 20px; margin-bottom: 10px;'>\\1</h3>");
    content.replace(QRegularExpression("(\\d+\\..+)"), "<h4 style='color: #7f8c8d; margin-top: 15px; margin-bottom: 8px;'>\\1</h4>");
    
    // 处理段落
    content.replace(QRegularExpression("<br><br>"), "</p><p>");
    content = "<p>" + content + "</p>";
    
    QString html = QString(
        "<html><head>"
        "<meta charset='utf-8'>"
        "<style>"
        "body { "
        "    font-family: 'Microsoft YaHei', 'SimSun', Arial, sans-serif; "
        "    line-height: 1.8; "
        "    color: #2c3e50; "
        "    background-color: #fafafa; "
        "    margin: 0; "
        "    padding: 20px; "
        "}"
        "h1 { "
        "    color: #2c3e50; "
        "    text-align: center; "
        "    border-bottom: 3px solid #3498db; "
        "    padding-bottom: 15px; "
        "    margin-bottom: 30px; "
        "    font-size: 24px; "
        "}"
        "h2 { "
        "    color: #2c3e50; "
        "    border-bottom: 2px solid #3498db; "
        "    padding-bottom: 8px; "
        "    margin-top: 35px; "
        "    margin-bottom: 20px; "
        "    font-size: 20px; "
        "}"
        "h3 { "
        "    color: #34495e; "
        "    margin-top: 25px; "
        "    margin-bottom: 15px; "
        "    font-size: 18px; "
        "    font-weight: bold; "
        "}"
        "h4 { "
        "    color: #7f8c8d; "
        "    margin-top: 20px; "
        "    margin-bottom: 12px; "
        "    font-size: 16px; "
        "    font-weight: bold; "
        "}"
        "p { "
        "    margin: 12px 0; "
        "    text-align: justify; "
        "    text-indent: 2em; "
        "}"
        ".category-info { "
        "    background: #ecf0f1; "
        "    padding: 15px; "
        "    border-radius: 8px; "
        "    margin: 20px 0; "
        "    border-left: 4px solid #3498db; "
        "}"
        ".category-info strong { "
        "    color: #2c3e50; "
        "    font-weight: bold; "
        "}"
        "hr { "
        "    border: none; "
        "    border-top: 2px solid #e1e8ed; "
        "    margin: 30px 0; "
        "}"
        "</style>"
        "</head><body>"
        "<h1>%1</h1>"
        "<div class='category-info'>"
        "<p><strong>文档类别：</strong>%2</p>"
        "<p><strong>创建时间：</strong>%3</p>"
        "<p><strong>更新时间：</strong>%4</p>"
        "</div>"
        "<hr>"
        "%5"
        "</body></html>"
    ).arg(currentDocument_.getTitle())
     .arg(currentDocument_.getCategory())
     .arg(formatDateTime(currentDocument_.getCreateTime()))
     .arg(formatDateTime(currentDocument_.getUpdateTime()))
     .arg(content);
    
    return html;
}

QString DocumentViewer::formatDateTime(const QDateTime& dateTime)
{
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

void DocumentViewer::showMessage(const QString& title, const QString& message, QMessageBox::Icon icon)
{
    QMessageBox msgBox(icon, title, message, QMessageBox::Ok, this);
    msgBox.exec();
}

bool DocumentViewer::confirmAction(const QString& title, const QString& message)
{
    return QMessageBox::question(this, title, message, 
                                QMessageBox::Yes | QMessageBox::No,
                                QMessageBox::No) == QMessageBox::Yes;
}

void DocumentViewer::logDocumentAction(const QString& action, const QString& details)
{
    qDebug() << "文档操作:" << action << details;
}
