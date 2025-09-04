#include "attachmentdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeType>
#include <QPixmap>
#include <QImageReader>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>
#include <QStandardPaths>
#include <QThread>
#include "documentprocessor.h"

AttachmentDialog::AttachmentDialog(Database* database, DocumentProcessor* processor, QWidget *parent)
    : QDialog(parent)
    , mainLayout_(nullptr)
    , fileGroup_(nullptr)
    , fileList_(nullptr)
    , previewGroup_(nullptr)
    , previewArea_(nullptr)
    , previewLabel_(nullptr)
    , database_(database)
    , documentProcessor_(processor)
    , uploadTimer_(new QTimer(this))
    , currentProgress_(0)
{
    setWindowTitle("附件管理");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(800, 600);
    
    // 初始化支持的文件类型
    supportedImageTypes_ << "jpg" << "jpeg" << "png" << "gif" << "bmp" << "tiff" << "webp";
    supportedDocTypes_ << "pdf" << "doc" << "docx" << "txt" << "rtf" << "odt";
    supportedOtherTypes_ << "xls" << "xlsx" << "ppt" << "pptx" << "csv" << "zip" << "rar";
    
    setupUI();
    
    // 连接上传模拟定时器
    connect(uploadTimer_, &QTimer::timeout, this, &AttachmentDialog::updateUploadProgress);
    
    // 连接文档处理器信号
    if (documentProcessor_) {
        connect(documentProcessor_, &DocumentProcessor::documentProcessed,
                this, &AttachmentDialog::onDocumentProcessed);
        connect(documentProcessor_, &DocumentProcessor::processingProgress,
                this, &AttachmentDialog::onProcessingProgress);
        connect(documentProcessor_, &DocumentProcessor::documentIndexed,
                this, &AttachmentDialog::onDocumentIndexed);
    }
}

AttachmentDialog::~AttachmentDialog()
{
}

void AttachmentDialog::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(15);
    
    setupFileArea();
    setupPreviewArea();
    setupButtonArea();
    
    // 应用样式
    setStyleSheet(
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid #DCDFE6;"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "    background: rgba(255,255,255,0.8);"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 15px;"
        "    padding: 4px 8px;"
        "    color: #409EFF;"
        "    font-size: 14px;"
        "}"
        "QListWidget {"
        "    border: 1px solid #DCDFE6;"
        "    border-radius: 4px;"
        "    background: white;"
        "    selection-background-color: #E6F7FF;"
        "    alternate-background-color: #FAFAFA;"
        "}"
        "QListWidget::item {"
        "    padding: 8px;"
        "    border-bottom: 1px solid #F0F0F0;"
        "}"
        "QListWidget::item:selected {"
        "    background: #E6F7FF;"
        "    color: #409EFF;"
        "}"
        "QPushButton {"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-weight: 500;"
        "    border: 1px solid #DCDFE6;"
        "}"
        "QPushButton:hover {"
        "    border-color: #409EFF;"
        "    color: #409EFF;"
        "}"
        "#uploadBtn {"
        "    background: #409EFF;"
        "    color: white;"
        "    border: none;"
        "}"
        "#uploadBtn:hover {"
        "    background: #337ECC;"
        "}"
        "#uploadBtn:disabled {"
        "    background: #C0C4CC;"
        "    color: #FFFFFF;"
        "}"
        "QProgressBar {"
        "    border: 1px solid #DCDFE6;"
        "    border-radius: 4px;"
        "    text-align: center;"
        "    background: #F5F7FA;"
        "}"
        "QProgressBar::chunk {"
        "    background: #409EFF;"
        "    border-radius: 3px;"
        "}"
    );
}

void AttachmentDialog::setupFileArea()
{
    fileGroup_ = new QGroupBox("文件选择");
    QVBoxLayout* fileLayout = new QVBoxLayout(fileGroup_);
    
    // 文件列表
    fileList_ = new QListWidget();
    fileList_->setAlternatingRowColors(true);
    fileList_->setMinimumHeight(200);
    connect(fileList_, &QListWidget::itemSelectionChanged, 
            this, &AttachmentDialog::onAttachmentSelectionChanged);
    fileLayout->addWidget(fileList_);
    
    // 文件操作按钮
    QHBoxLayout* fileButtonLayout = new QHBoxLayout();
    
    browseBtn_ = new QPushButton("浏览文件...");
    connect(browseBtn_, &QPushButton::clicked, this, &AttachmentDialog::onBrowseClicked);
    fileButtonLayout->addWidget(browseBtn_);
    
    removeBtn_ = new QPushButton("移除选中");
    removeBtn_->setEnabled(false);
    connect(removeBtn_, &QPushButton::clicked, this, &AttachmentDialog::onRemoveClicked);
    fileButtonLayout->addWidget(removeBtn_);
    
    previewBtn_ = new QPushButton("预览");
    previewBtn_->setEnabled(false);
    connect(previewBtn_, &QPushButton::clicked, this, &AttachmentDialog::onPreviewClicked);
    fileButtonLayout->addWidget(previewBtn_);
    
    fileButtonLayout->addStretch();
    fileLayout->addLayout(fileButtonLayout);
    
    // 文件汇总信息
    fileSummaryLabel_ = new QLabel("未选择文件");
    fileSummaryLabel_->setStyleSheet("color: #909399; font-size: 12px; padding: 5px;");
    fileLayout->addWidget(fileSummaryLabel_);
    
    mainLayout_->addWidget(fileGroup_);
}

void AttachmentDialog::setupPreviewArea()
{
    previewGroup_ = new QGroupBox("文件预览与描述");
    QHBoxLayout* previewLayout = new QHBoxLayout(previewGroup_);
    
    // 预览区域
    previewArea_ = new QScrollArea();
    previewArea_->setFixedWidth(300);
    previewArea_->setMinimumHeight(180);
    previewArea_->setStyleSheet("border: 1px solid #DCDFE6; background: white;");
    
    previewLabel_ = new QLabel();
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setText("选择文件查看预览");
    previewLabel_->setStyleSheet("color: #C0C4CC; padding: 20px;");
    previewArea_->setWidget(previewLabel_);
    
    previewLayout->addWidget(previewArea_);
    
    // 描述区域
    QVBoxLayout* descLayout = new QVBoxLayout();
    
    QLabel* descLabel = new QLabel("文件描述 (可选):");
    descLabel->setStyleSheet("font-weight: bold; color: #606266;");
    descLayout->addWidget(descLabel);
    
    descriptionEdit_ = new QTextEdit();
    descriptionEdit_->setMaximumHeight(120);
    descriptionEdit_->setPlaceholderText("请输入文件描述信息...");
    descriptionEdit_->setStyleSheet(
        "border: 1px solid #DCDFE6;"
        "border-radius: 4px;"
        "padding: 8px;"
        "background: white;"
    );
    descLayout->addWidget(descriptionEdit_);
    
    // 上传进度组
    progressGroup_ = new QGroupBox("上传进度");
    progressGroup_->setVisible(false);
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup_);
    
    uploadProgress_ = new QProgressBar();
    uploadProgress_->setTextVisible(true);
    uploadProgress_->setValue(0);
    progressLayout->addWidget(uploadProgress_);
    
    progressLabel_ = new QLabel("等待上传...");
    progressLabel_->setStyleSheet("color: #606266; font-size: 12px;");
    progressLayout->addWidget(progressLabel_);
    
    descLayout->addWidget(progressGroup_);
    descLayout->addStretch();
    
    previewLayout->addLayout(descLayout);
    
    mainLayout_->addWidget(previewGroup_);
}

void AttachmentDialog::setupButtonArea()
{
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    // 支持的格式提示
    QLabel* formatLabel = new QLabel(
        "支持格式: 图片 (jpg, png, gif), 文档 (pdf, doc, txt), 表格 (xls, csv), 压缩包 (zip, rar)"
    );
    formatLabel->setStyleSheet("color: #909399; font-size: 11px;");
    buttonLayout->addWidget(formatLabel);
    
    buttonLayout->addStretch();
    
    cancelBtn_ = new QPushButton("取消");
    connect(cancelBtn_, &QPushButton::clicked, this, &AttachmentDialog::onCancelClicked);
    buttonLayout->addWidget(cancelBtn_);
    
    uploadBtn_ = new QPushButton("上传");
    uploadBtn_->setObjectName("uploadBtn");
    uploadBtn_->setEnabled(false);
    connect(uploadBtn_, &QPushButton::clicked, this, &AttachmentDialog::onUploadClicked);
    buttonLayout->addWidget(uploadBtn_);
    
    mainLayout_->addLayout(buttonLayout);
}

void AttachmentDialog::onBrowseClicked()
{
    QStringList supportedFormats;
    supportedFormats << supportedImageTypes_ << supportedDocTypes_ << supportedOtherTypes_;
    
    QString filter = "支持的文件 (";
    for (const QString& ext : supportedFormats) {
        filter += "*." + ext + " ";
    }
    filter += ");;";
    filter += "图片文件 (*.jpg *.jpeg *.png *.gif *.bmp);;";
    filter += "文档文件 (*.pdf *.doc *.docx *.txt);;";
    filter += "表格文件 (*.xls *.xlsx *.csv);;";
    filter += "所有文件 (*.*)";
    
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "选择附件",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        filter
    );
    
    for (const QString& fileName : fileNames) {
        addAttachment(fileName);
    }
    
    updateFileInfo();
}

void AttachmentDialog::onRemoveClicked()
{
    QList<QListWidgetItem*> selectedItems = fileList_->selectedItems();
    
    if (selectedItems.isEmpty()) {
        return;
    }
    
    for (QListWidgetItem* item : selectedItems) {
        int row = fileList_->row(item);
        if (row >= 0 && row < attachments_.size()) {
            attachments_.removeAt(row);
        }
        delete item;
    }
    
    updateFileInfo();
    updatePreview();
}

void AttachmentDialog::onPreviewClicked()
{
    QListWidgetItem* currentItem = fileList_->currentItem();
    if (!currentItem) return;
    
    int row = fileList_->currentRow();
    if (row >= 0 && row < attachments_.size()) {
        const AttachmentInfo& info = attachments_[row];
        
        // 尝试用系统默认程序打开文件预览
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(info.filePath))) {
            QMessageBox::warning(this, "预览失败", "无法打开文件预览。");
        }
    }
}

void AttachmentDialog::onUploadClicked()
{
    if (attachments_.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择要上传的文件。");
        return;
    }
    
    qDebug() << "准备上传" << attachments_.size() << "个文件";
    qDebug() << "数据库连接状态:" << (database_ ? "已连接" : "未连接");
    qDebug() << "文档处理器状态:" << (documentProcessor_ ? "已初始化" : "未初始化");
    
    if (!database_) {
        QMessageBox::critical(this, "错误", "数据库连接未正确初始化，无法上传文件。");
        return;
    }
    
    if (!documentProcessor_) {
        QMessageBox::critical(this, "错误", "文档处理器未正确初始化，无法上传文件。");
        return;
    }
    
    // 开始真实的文档处理
    startRealUpload();
}

void AttachmentDialog::onCancelClicked()
{
    if (uploadTimer_->isActive()) {
        uploadTimer_->stop();
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "确认取消", "上传正在进行中，确定要取消吗？",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        
        if (reply == QMessageBox::No) {
            uploadTimer_->start(100);
            return;
        }
    }
    
    reject();
}

void AttachmentDialog::onAttachmentSelectionChanged()
{
    bool hasSelection = !fileList_->selectedItems().isEmpty();
    removeBtn_->setEnabled(hasSelection);
    previewBtn_->setEnabled(hasSelection);
    
    updatePreview();
}

void AttachmentDialog::updateUploadProgress()
{
    currentProgress_ += 2;
    uploadProgress_->setValue(currentProgress_);
    
    if (currentProgress_ <= 30) {
        progressLabel_->setText("准备上传...");
    } else if (currentProgress_ <= 70) {
        progressLabel_->setText("正在上传文件...");
    } else if (currentProgress_ <= 95) {
        progressLabel_->setText("处理文件...");
    } else if (currentProgress_ >= 100) {
        uploadTimer_->stop();
        progressLabel_->setText("上传完成！");
        
        QTimer::singleShot(1000, [this]() {
            QMessageBox::information(this, "上传成功", 
                QString("成功上传 %1 个文件！").arg(attachments_.size()));
            accept();
        });
        return;
    }
}

void AttachmentDialog::addAttachment(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        QMessageBox::warning(this, "错误", "文件不存在或无效: " + filePath);
        return;
    }
    
    // 检查文件是否已存在
    for (const AttachmentInfo& existing : attachments_) {
        if (existing.filePath == filePath) {
            QMessageBox::information(this, "提示", "文件已存在: " + fileInfo.fileName());
            return;
        }
    }
    
    // 检查文件大小 (限制为 10MB)
    if (fileInfo.size() > 10 * 1024 * 1024) {
        QMessageBox::warning(this, "文件过大", 
            QString("文件 '%1' 超过 10MB 限制。").arg(fileInfo.fileName()));
        return;
    }
    
    AttachmentInfo info;
    info.fileName = fileInfo.fileName();
    info.filePath = filePath;
    info.fileSize = fileInfo.size();
    info.fileType = fileInfo.suffix().toLower();
    info.preview = generatePreviewIcon(filePath);
    
    attachments_.append(info);
    
    // 添加到列表
    QString displayText = QString("%1 (%2)")
                         .arg(info.fileName)
                         .arg(formatFileSize(info.fileSize));
    
    QListWidgetItem* item = new QListWidgetItem(displayText);
    item->setIcon(QIcon(info.preview));
    item->setData(Qt::UserRole, attachments_.size() - 1);
    
    fileList_->addItem(item);
    
    uploadBtn_->setEnabled(!attachments_.isEmpty());
}

void AttachmentDialog::updateFileInfo()
{
    if (attachments_.isEmpty()) {
        fileSummaryLabel_->setText("未选择文件");
        uploadBtn_->setEnabled(false);
        return;
    }
    
    qint64 totalSize = 0;
    QMap<QString, int> typeCount;
    
    for (const AttachmentInfo& info : attachments_) {
        totalSize += info.fileSize;
        typeCount[info.fileType]++;
    }
    
    QString summary = QString("已选择 %1 个文件，总大小: %2")
                     .arg(attachments_.size())
                     .arg(formatFileSize(totalSize));
    
    if (typeCount.size() > 1) {
        QStringList typeInfo;
        for (auto it = typeCount.begin(); it != typeCount.end(); ++it) {
            typeInfo << QString("%1: %2个").arg(it.key().toUpper()).arg(it.value());
        }
        summary += " (" + typeInfo.join(", ") + ")";
    }
    
    fileSummaryLabel_->setText(summary);
    uploadBtn_->setEnabled(true);
}

void AttachmentDialog::updatePreview()
{
    QListWidgetItem* currentItem = fileList_->currentItem();
    if (!currentItem) {
        previewLabel_->setText("选择文件查看预览");
        previewLabel_->setPixmap(QPixmap());
        descriptionEdit_->clear();
        return;
    }
    
    int row = fileList_->currentRow();
    if (row >= 0 && row < attachments_.size()) {
        const AttachmentInfo& info = attachments_[row];
        
        if (isImageFile(info.filePath)) {
            // 显示图片预览
            QPixmap pixmap(info.filePath);
            if (!pixmap.isNull()) {
                pixmap = pixmap.scaled(280, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                previewLabel_->setPixmap(pixmap);
                previewLabel_->setText("");
            }
        } else {
            // 显示文件图标和信息
            previewLabel_->setPixmap(info.preview.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            previewLabel_->setText(QString("\n%1\n%2\n类型: %3")
                                  .arg(info.fileName)
                                  .arg(formatFileSize(info.fileSize))
                                  .arg(info.fileType.toUpper()));
        }
        
        descriptionEdit_->setText(info.description);
    }
}

bool AttachmentDialog::isImageFile(const QString& filePath)
{
    QString suffix = QFileInfo(filePath).suffix().toLower();
    return supportedImageTypes_.contains(suffix);
}

bool AttachmentDialog::isDocumentFile(const QString& filePath)
{
    QString suffix = QFileInfo(filePath).suffix().toLower();
    return supportedDocTypes_.contains(suffix);
}

QString AttachmentDialog::formatFileSize(qint64 bytes)
{
    if (bytes < 1024) {
        return QString("%1 B").arg(bytes);
    } else if (bytes < 1024 * 1024) {
        return QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    } else {
        return QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 1);
    }
}

QPixmap AttachmentDialog::generatePreviewIcon(const QString& filePath)
{
    QString suffix = QFileInfo(filePath).suffix().toLower();
    
    // 根据文件类型返回相应图标
    QStyle* style = QApplication::style();
    QIcon icon;
    
    if (supportedImageTypes_.contains(suffix)) {
        icon = style->standardIcon(QStyle::SP_FileDialogDetailedView);
    } else if (supportedDocTypes_.contains(suffix)) {
        icon = style->standardIcon(QStyle::SP_FileIcon);
    } else {
        icon = style->standardIcon(QStyle::SP_FileIcon);
    }
    
    return icon.pixmap(32, 32);
}

void AttachmentDialog::startUploadSimulation()
{
    progressGroup_->setVisible(true);
    uploadProgress_->setValue(0);
    currentProgress_ = 0;
    progressLabel_->setText("开始上传...");
    
    uploadBtn_->setEnabled(false);
    browseBtn_->setEnabled(false);
    removeBtn_->setEnabled(false);
    
    uploadTimer_->start(100); // 每100ms更新一次进度
}

void AttachmentDialog::startRealUpload()
{
    progressGroup_->setVisible(true);
    uploadProgress_->setValue(0);
    progressLabel_->setText("开始处理文档...");
    
    uploadBtn_->setEnabled(false);
    browseBtn_->setEnabled(false);
    removeBtn_->setEnabled(false);
    
    int totalFiles = attachments_.size();
    int processedFiles = 0;
    int successCount = 0;
    
    QStringList failedFiles;
    
    for (const AttachmentInfo& attachment : attachments_) {
        progressLabel_->setText(QString("正在处理: %1").arg(attachment.fileName));
        uploadProgress_->setValue((processedFiles * 100) / totalFiles);
        
        // 刷新界面
        QApplication::processEvents();
        
        qDebug() << "开始处理文档:" << attachment.fileName << "文件路径:" << attachment.filePath;
        
        // 检查文件是否仍然存在
        QFileInfo fileInfo(attachment.filePath);
        if (!fileInfo.exists()) {
            qWarning() << "文件不存在，跳过处理:" << attachment.filePath;
            failedFiles.append(attachment.fileName + " (文件不存在)");
            processedFiles++;
            continue;
        }
        
        // 确定文档类别
        QString category = "通用制度";
        if (attachment.fileType.contains("pdf") || attachment.fileType.contains("doc") || attachment.fileType.contains("txt")) {
            category = "通用制度";
        }
        
        qDebug() << "文档分类:" << category << "文件大小:" << fileInfo.size() << "字节";
        
        // 额外健壮性判断
        if (!documentProcessor_) {
            failedFiles.append(attachment.fileName + " (处理器未初始化)");
            qWarning() << "文档处理器未初始化";
            processedFiles++;
            continue;
        }
        if (!database_ || !database_->isConnected()) {
            failedFiles.append(attachment.fileName + " (数据库未连接)");
            qWarning() << "数据库未连接";
            processedFiles++;
            continue;
        }
        
        // 使用DocumentProcessor处理文档
        bool success = documentProcessor_->processDocument(attachment.filePath, category);
        
        if (success) {
            successCount++;
            qInfo() << "文档处理成功:" << attachment.fileName;
        } else {
            failedFiles.append(attachment.fileName);
            qWarning() << "文档处理失败:" << attachment.fileName;
        }
        
        processedFiles++;
        uploadProgress_->setValue((processedFiles * 100) / totalFiles);
        
        // 仅处理事件，避免阻塞UI线程
        QApplication::processEvents();
    }
    
    uploadProgress_->setValue(100);
    progressLabel_->setText("处理完成！");
    
    // 显示结果
    QString resultMessage;
    if (successCount == totalFiles) {
        resultMessage = QString("成功上传并处理了 %1 个文档！").arg(successCount);
        QTimer::singleShot(1000, [this, resultMessage]() {
            QMessageBox::information(this, "上传成功", resultMessage);
            accept();
        });
    } else {
        resultMessage = QString("成功处理 %1 个文档，失败 %2 个。\n失败的文档：%3")
                       .arg(successCount)
                       .arg(failedFiles.size())
                       .arg(failedFiles.join(", "));
        QTimer::singleShot(600, [this, resultMessage, successCount]() {
            QMessageBox::warning(this, "上传结果", resultMessage);
            
            // 如果有成功的，询问是否关闭对话框
            if (successCount > 0) {
                int ret = QMessageBox::question(this, "确认", "是否关闭对话框？",
                                               QMessageBox::Yes | QMessageBox::No, 
                                               QMessageBox::Yes);
                if (ret == QMessageBox::Yes) {
                    accept();
                }
            }
        });
    }
}

QList<AttachmentInfo> AttachmentDialog::getSelectedAttachments() const
{
    // 更新描述信息
    QList<AttachmentInfo> result = attachments_;
    
    QListWidgetItem* currentItem = fileList_->currentItem();
    if (currentItem) {
        int row = fileList_->currentRow();
        if (row >= 0 && row < result.size()) {
            result[row].description = descriptionEdit_->toPlainText();
        }
    }
    
    return result;
}

// 文档处理器信号处理方法
void AttachmentDialog::onDocumentProcessed(const QString& fileName, bool success)
{
    if (success) {
        qInfo() << "文档处理成功:" << fileName;
    } else {
        qWarning() << "文档处理失败:" << fileName;
    }
}

void AttachmentDialog::onProcessingProgress(int current, int total)
{
    // 这个信号目前由DocumentProcessor::processDocumentBatch方法发出
    // 但我们的startRealUpload方法没有使用批处理，所以这里可能不会被调用
    qDebug() << "批处理进度:" << current << "/" << total;
}

void AttachmentDialog::onDocumentIndexed(const QString& documentId, const QString& title)
{
    qInfo() << "文档已索引:" << title << "ID:" << documentId;
}
