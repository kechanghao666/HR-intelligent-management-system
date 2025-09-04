

#include "batchuploaddialog.h"
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>
#include <QStringConverter>

BatchUploadDialog::BatchUploadDialog(Database* database, QWidget* parent)
    : QDialog(parent)
    , database_(database)
    , totalFiles_(0)
    , processedFiles_(0)
{
    setWindowTitle("批量上传文档");
    setModal(true);
    setMinimumSize(1000, 750);  // 增加窗口尺寸以容纳所有文字
    setupUI();
    setupConnections();
    applyStyles();
}

BatchUploadDialog::~BatchUploadDialog()
{
}

void BatchUploadDialog::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(20, 20, 20, 20);
    mainLayout_->setSpacing(15);
    
    // 文件选择区域
    QGroupBox* fileSelectionGroup = new QGroupBox("文件选择");
    QVBoxLayout* fileSelectionLayout = new QVBoxLayout(fileSelectionGroup);
    
    // 文件夹选择
    QHBoxLayout* folderLayout = new QHBoxLayout();
    folderPathEdit_ = new QLineEdit();
    folderPathEdit_->setPlaceholderText("选择文件夹路径或直接选择文件");
    folderPathEdit_->setReadOnly(true);
    
    selectFolderButton_ = new QPushButton("选择文件夹");
    selectFolderButton_->setMinimumSize(120, 35);  // 确保按钮文字完整显示
    selectFilesButton_ = new QPushButton("选择文件");
    selectFilesButton_->setMinimumSize(120, 35);   // 确保按钮文字完整显示
    
    folderLayout->addWidget(folderPathEdit_);
    folderLayout->addWidget(selectFolderButton_);
    folderLayout->addWidget(selectFilesButton_);
    fileSelectionLayout->addLayout(folderLayout);
    
    mainLayout_->addWidget(fileSelectionGroup);
    
    // 设置选项区域
    QGroupBox* settingsGroup = new QGroupBox("上传设置");
    QGridLayout* settingsLayout = new QGridLayout(settingsGroup);
    
    // 默认类别
    QLabel* categoryLabel = new QLabel("默认类别:");
    defaultCategoryCombo_ = new QComboBox();
    defaultCategoryCombo_->addItem("考勤制度", "考勤");
    defaultCategoryCombo_->addItem("薪酬制度", "薪酬");
    defaultCategoryCombo_->addItem("绩效制度", "绩效");
    defaultCategoryCombo_->addItem("入职离职制度", "入职离职");
    defaultCategoryCombo_->addItem("员工福利制度", "员工福利");
    defaultCategoryCombo_->addItem("通用制度", "通用");
    
    // 自动检测类别
    autoDetectCategoryCheck_ = new QCheckBox("自动检测文档类别");
    autoDetectCategoryCheck_->setChecked(true);
    
    // 覆盖现有文件
    overwriteExistingCheck_ = new QCheckBox("覆盖现有文档");
    
    // 最大文件大小
    QLabel* maxSizeLabel = new QLabel("最大文件大小(MB):");
    maxFileSizeSpin_ = new QSpinBox();
    maxFileSizeSpin_->setRange(1, 100);
    maxFileSizeSpin_->setValue(10);
    
    settingsLayout->addWidget(categoryLabel, 0, 0);
    settingsLayout->addWidget(defaultCategoryCombo_, 0, 1);
    settingsLayout->addWidget(autoDetectCategoryCheck_, 1, 0, 1, 2);
    settingsLayout->addWidget(overwriteExistingCheck_, 2, 0, 1, 2);
    settingsLayout->addWidget(maxSizeLabel, 3, 0);
    settingsLayout->addWidget(maxFileSizeSpin_, 3, 1);
    
    mainLayout_->addWidget(settingsGroup);
    
    // 文件列表区域
    QGroupBox* fileListGroup = new QGroupBox("待上传文件");
    QVBoxLayout* fileListLayout = new QVBoxLayout(fileListGroup);
    
    // 文件列表
    fileListWidget_ = new QListWidget();
    
    // 列表操作按钮
    QHBoxLayout* listButtonLayout = new QHBoxLayout();
    previewButton_ = new QPushButton("预览");
    clearListButton_ = new QPushButton("清空列表");
    
    listButtonLayout->addWidget(previewButton_);
    listButtonLayout->addWidget(clearListButton_);
    listButtonLayout->addStretch();
    
    fileListLayout->addWidget(fileListWidget_);
    fileListLayout->addLayout(listButtonLayout);
    
    mainLayout_->addWidget(fileListGroup);
    
    // 进度显示区域
    QGroupBox* progressGroup = new QGroupBox("上传进度");
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);
    
    progressBar_ = new QProgressBar();
    progressBar_->setVisible(false);
    
    statusLabel_ = new QLabel("准备就绪");
    
    progressLayout->addWidget(progressBar_);
    progressLayout->addWidget(statusLabel_);
    
    mainLayout_->addWidget(progressGroup);
    
    // 操作按钮
    buttonLayout_ = new QHBoxLayout();
    buttonLayout_->addStretch();
    
    uploadButton_ = new QPushButton("开始上传");
    uploadButton_->setMinimumSize(120, 40);
    
    cancelButton_ = new QPushButton("取消");
    cancelButton_->setMinimumSize(120, 40);
    
    buttonLayout_->addWidget(uploadButton_);
    buttonLayout_->addWidget(cancelButton_);
    
    mainLayout_->addLayout(buttonLayout_);
}

void BatchUploadDialog::setupConnections()
{
    connect(selectFolderButton_, &QPushButton::clicked, this, &BatchUploadDialog::onSelectFolderClicked);
    connect(selectFilesButton_, &QPushButton::clicked, this, &BatchUploadDialog::onSelectFilesClicked);
    connect(uploadButton_, &QPushButton::clicked, this, &BatchUploadDialog::onUploadClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &BatchUploadDialog::onCancelClicked);
    connect(previewButton_, &QPushButton::clicked, this, &BatchUploadDialog::onPreviewClicked);
    connect(clearListButton_, &QPushButton::clicked, this, &BatchUploadDialog::onClearListClicked);
}

void BatchUploadDialog::applyStyles()
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
    );
}

void BatchUploadDialog::onSelectFolderClicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(this, "选择文件夹", QDir::homePath());
    if (!folderPath.isEmpty()) {
        folderPathEdit_->setText(folderPath);
        loadFilesFromFolder(folderPath);
    }
}

void BatchUploadDialog::onSelectFilesClicked()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(this, "选择文档文件", QDir::homePath(),
        "文档文件 (*.txt *.doc *.docx *.pdf *.rtf);;文本文件 (*.txt);;所有文件 (*.*)");
    
    if (!filePaths.isEmpty()) {
        for (const QString& filePath : filePaths) {
            if (isValidFile(filePath)) {
                addFileToList(filePath);
            }
        }
        statusLabel_->setText(QString("已选择 %1 个文件").arg(fileListWidget_->count()));
    }
}

void BatchUploadDialog::onUploadClicked()
{
    if (fileListWidget_->count() == 0) {
        QMessageBox::warning(this, "警告", "请先选择要上传的文件！");
        return;
    }
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "确认上传", 
        QString("确定要上传 %1 个文件吗？").arg(fileListWidget_->count()),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        processFiles();
    }
}

void BatchUploadDialog::onCancelClicked()
{
    reject();
}

void BatchUploadDialog::onPreviewClicked()
{
    QListWidgetItem* currentItem = fileListWidget_->currentItem();
    if (currentItem) {
        QString filePath = currentItem->data(Qt::UserRole).toString();
        QString content = readFileContent(filePath);
        
        QDialog* previewDialog = new QDialog(this);
        previewDialog->setWindowTitle("文件预览");
        previewDialog->setModal(true);
        previewDialog->setMinimumSize(600, 400);
        
        QVBoxLayout* layout = new QVBoxLayout(previewDialog);
        
        QTextEdit* textEdit = new QTextEdit();
        textEdit->setPlainText(content);
        textEdit->setReadOnly(true);
        
        QPushButton* closeButton = new QPushButton("关闭");
        connect(closeButton, &QPushButton::clicked, previewDialog, &QDialog::accept);
        
        layout->addWidget(textEdit);
        layout->addWidget(closeButton);
        
        previewDialog->exec();
        delete previewDialog;
    }
}

void BatchUploadDialog::onClearListClicked()
{
    fileListWidget_->clear();
    selectedFiles_.clear();
    statusLabel_->setText("文件列表已清空");
}

void BatchUploadDialog::loadFilesFromFolder(const QString& folderPath)
{
    QDir dir(folderPath);
    QStringList filters;
    filters << "*.txt" << "*.doc" << "*.docx" << "*.pdf" << "*.rtf";
    
    QFileInfoList fileList = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : fileList) {
        if (isValidFile(fileInfo.absoluteFilePath())) {
            addFileToList(fileInfo.absoluteFilePath());
        }
    }
    
    statusLabel_->setText(QString("从文件夹加载了 %1 个文件").arg(fileListWidget_->count()));
}

void BatchUploadDialog::processFiles()
{
    progressBar_->setVisible(true);
    progressBar_->setRange(0, fileListWidget_->count());
    progressBar_->setValue(0);
    
    uploadButton_->setEnabled(false);
    selectFolderButton_->setEnabled(false);
    selectFilesButton_->setEnabled(false);
    
    totalFiles_ = fileListWidget_->count();
    processedFiles_ = 0;
    
    for (int i = 0; i < fileListWidget_->count(); ++i) {
        QListWidgetItem* item = fileListWidget_->item(i);
        QString filePath = item->data(Qt::UserRole).toString();
        
        QString content = readFileContent(filePath);
        if (content.isEmpty()) {
            qWarning() << "跳过空内容文件:" << filePath;
            continue;
        }
        
        // 检查是否是.docx文件的特殊处理信息
        QFileInfo fileInfo(filePath);
        bool isDocxFile = (fileInfo.suffix().toLower() == "docx");
        
        // 对于.docx文件，即使内容是提示信息，也要保存到数据库
        if (isDocxFile || !content.trimmed().isEmpty()) {
            // 继续处理
        } else {
            qWarning() << "跳过无有效内容的文件:" << filePath;
            continue;
        }
        
        QString title = extractTitleFromFileName(QFileInfo(filePath).fileName());
        QString category = defaultCategoryCombo_->currentData().toString();
        
        if (autoDetectCategoryCheck_->isChecked()) {
            QString detectedCategory = determineCategoryFromContent(content);
            if (!detectedCategory.isEmpty()) {
                category = detectedCategory;
            }
        }
        
        Document document(title, content, category);
        
        qDebug() << "正在保存文档:" << title << "类别:" << category << "内容长度:" << content.length();
        
        if (database_ && database_->addDocument(document)) {
            processedFiles_++;
            qDebug() << "成功保存文档:" << title;
        } else {
            qWarning() << "保存文档失败:" << title << "database_是否为空:" << (database_ == nullptr);
        }
        
        progressBar_->setValue(i + 1);
        statusLabel_->setText(QString("正在处理: %1/%2").arg(i + 1).arg(totalFiles_));
        
        QApplication::processEvents();
    }
    
    progressBar_->setVisible(false);
    statusLabel_->setText(QString("上传完成！成功上传 %1 个文件").arg(processedFiles_));
    
    uploadButton_->setEnabled(true);
    selectFolderButton_->setEnabled(true);
    selectFilesButton_->setEnabled(true);
    
    QMessageBox::information(this, "上传完成", 
        QString("批量上传完成！\n成功上传: %1 个文件").arg(processedFiles_));
    
    accept();
}

void BatchUploadDialog::addFileToList(const QString& filePath)
{
    if (!selectedFiles_.contains(filePath)) {
        selectedFiles_.append(filePath);
        
        QListWidgetItem* item = new QListWidgetItem();
        item->setText(QFileInfo(filePath).fileName());
        item->setData(Qt::UserRole, filePath);
        item->setToolTip(filePath);
        
        fileListWidget_->addItem(item);
    }
}

void BatchUploadDialog::updateProgress(int current, int total)
{
    progressBar_->setValue(current);
    statusLabel_->setText(QString("正在处理: %1/%2").arg(current).arg(total));
}

QString BatchUploadDialog::extractTitleFromFileName(const QString& fileName)
{
    QString title = QFileInfo(fileName).baseName();
    title = title.replace(QRegularExpression("^[0-9]+_?"), "");
    title = title.replace(QRegularExpression("_+$"), "");
    return title;
}

QString BatchUploadDialog::determineCategoryFromContent(const QString& content)
{
    QString lowerContent = content.toLower();
    
    if (lowerContent.contains("考勤") || lowerContent.contains("打卡") || lowerContent.contains("请假") || 
        lowerContent.contains("加班") || lowerContent.contains("出勤")) {
        return "考勤";
    } else if (lowerContent.contains("薪酬") || lowerContent.contains("工资") || lowerContent.contains("薪资") || 
               lowerContent.contains("奖金") || lowerContent.contains("补贴")) {
        return "薪酬";
    } else if (lowerContent.contains("绩效") || lowerContent.contains("考核") || lowerContent.contains("评估") || 
               lowerContent.contains("kpi") || lowerContent.contains("目标")) {
        return "绩效";
    } else if (lowerContent.contains("入职") || lowerContent.contains("离职") || lowerContent.contains("转正") || 
               lowerContent.contains("试用期") || lowerContent.contains("合同")) {
        return "入职离职";
    } else if (lowerContent.contains("福利") || lowerContent.contains("保险") || lowerContent.contains("社保") || 
               lowerContent.contains("公积金") || lowerContent.contains("医疗")) {
        return "员工福利";
    }
    
    return "通用";
}

QString BatchUploadDialog::readFileContent(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();
    
    // 特殊处理.docx文件
    if (extension == "docx") {
        return readDocxContent(filePath);
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return QString();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    // 尝试检测编码
    QString content;
    
    // 首先尝试UTF-8
    QTextStream utf8Stream(data);
    utf8Stream.setEncoding(QStringConverter::Utf8);
    content = utf8Stream.readAll();
    
    // 如果UTF-8解码失败或内容为空，尝试其他编码
    if (content.isEmpty() || content.contains(QChar::ReplacementCharacter)) {
        // 尝试GBK编码
        QStringDecoder gbkDecoder("GBK");
        if (gbkDecoder.isValid()) {
            content = gbkDecoder.decode(data);
            if (gbkDecoder.hasError()) {
                // 尝试系统默认编码
                QStringDecoder systemDecoder("System");
                if (systemDecoder.isValid()) {
                    content = systemDecoder.decode(data);
                    if (systemDecoder.hasError()) {
                        // 最后尝试Latin1
                        content = QString::fromLatin1(data);
                    }
                } else {
                    content = QString::fromLocal8Bit(data);
                }
            }
        } else {
            // GBK不可用，尝试系统默认编码
            content = QString::fromLocal8Bit(data);
        }
    }
    
    return content;
}

bool BatchUploadDialog::isValidFile(const QString& filePath)
{
    QFileInfo fileInfo(filePath);
    
    qint64 fileSize = fileInfo.size();
    qint64 maxSize = maxFileSizeSpin_->value() * 1024 * 1024;
    
    if (fileSize > maxSize) {
        return false;
    }
    
    QString extension = fileInfo.suffix().toLower();
    QStringList validExtensions = {"txt", "doc", "docx", "pdf", "rtf"};
    
    return validExtensions.contains(extension);
}

QString BatchUploadDialog::readDocxContent(const QString& filePath)
{
    Q_UNUSED(filePath)
    
    // .docx文件需要专门的解析器，暂时不支持
    // 建议用户将.docx转换为.txt格式上传
    QString content = QString("暂时不支持.docx文件直接解析。\n\n"
                             "建议操作步骤：\n"
                             "1. 在Word中打开此文档\n"
                             "2. 另存为 → 纯文本(.txt)格式\n"
                             "3. 选择UTF-8编码保存\n"
                             "4. 重新上传.txt文件\n\n"
                             "或者您可以直接将文档内容复制粘贴到'添加文档'功能中。");
    
    return content;
}
