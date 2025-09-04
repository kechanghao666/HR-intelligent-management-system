#include "documentdialog.h"
#include <QApplication>
#include <QStyle>
#include <QRegularExpression>

DocumentDialog::DocumentDialog(Database* database, QWidget* parent)
    : QDialog(parent)
    , database_(database)
    , editMode_(false)
    , titleEdit_(nullptr)
    , categoryCombo_(nullptr)
    , contentEdit_(nullptr)
    , buttonBox_(nullptr)
    , saveButton_(nullptr)
    , cancelButton_(nullptr)
{
    setWindowTitle("添加文档");
    setModal(true);
    setMinimumSize(600, 500);
    setupUI();
}

DocumentDialog::DocumentDialog(Database* database, const Document& document, QWidget* parent)
    : QDialog(parent)
    , database_(database)
    , editMode_(true)
    , document_(document)
    , titleEdit_(nullptr)
    , categoryCombo_(nullptr)
    , contentEdit_(nullptr)
    , buttonBox_(nullptr)
    , saveButton_(nullptr)
    , cancelButton_(nullptr)
{
    setWindowTitle("编辑文档");
    setModal(true);
    setMinimumSize(600, 500);
    setupUI();
    loadDocument(document);
}

void DocumentDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 表单布局
    QFormLayout* formLayout = new QFormLayout();
    
    // 标题输入
    titleEdit_ = new QLineEdit();
    titleEdit_->setPlaceholderText("请输入文档标题");
    titleEdit_->setMinimumHeight(35);
    titleEdit_->setStyleSheet("QLineEdit { padding: 8px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 14px; }"
                              "QLineEdit:focus { border-color: #3498db; }");
    formLayout->addRow("文档标题:", titleEdit_);
    
    // 类别选择
    categoryCombo_ = new QComboBox();
    categoryCombo_->addItem("考勤制度", "考勤");
    categoryCombo_->addItem("薪酬制度", "薪酬");
    categoryCombo_->addItem("绩效制度", "绩效");
    categoryCombo_->addItem("入职离职制度", "入职离职");
    categoryCombo_->addItem("员工福利制度", "员工福利");
    categoryCombo_->addItem("通用制度", "通用");
    categoryCombo_->setMinimumHeight(35);
    categoryCombo_->setStyleSheet("QComboBox { padding: 8px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 14px; }"
                                  "QComboBox:focus { border-color: #3498db; }"
                                  "QComboBox::drop-down { border: none; }"
                                  "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #7f8c8d; }");
    formLayout->addRow("文档类别:", categoryCombo_);
    
    mainLayout->addLayout(formLayout);
    
    // 内容输入
    QLabel* contentLabel = new QLabel("文档内容:");
    contentLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    mainLayout->addWidget(contentLabel);
    
    contentEdit_ = new QTextEdit();
    contentEdit_->setPlaceholderText("请输入文档内容...");
    contentEdit_->setStyleSheet("QTextEdit { padding: 10px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 13px; line-height: 1.5; }"
                                "QTextEdit:focus { border-color: #3498db; }");
    mainLayout->addWidget(contentEdit_);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    saveButton_ = new QPushButton("保存");
    saveButton_->setMinimumSize(80, 35);
    saveButton_->setStyleSheet("QPushButton { background-color: #3498db; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; }"
                               "QPushButton:hover { background-color: #2980b9; }"
                               "QPushButton:pressed { background-color: #21618c; }"
                               "QPushButton:disabled { background-color: #bdc3c7; }");
    
    cancelButton_ = new QPushButton("取消");
    cancelButton_->setMinimumSize(80, 35);
    cancelButton_->setStyleSheet("QPushButton { background-color: #95a5a6; color: white; border: none; border-radius: 5px; font-size: 14px; font-weight: bold; }"
                                 "QPushButton:hover { background-color: #7f8c8d; }"
                                 "QPushButton:pressed { background-color: #6c7b7d; }");
    
    buttonLayout->addWidget(saveButton_);
    buttonLayout->addWidget(cancelButton_);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(saveButton_, &QPushButton::clicked, this, &DocumentDialog::onSaveClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &DocumentDialog::onCancelClicked);
    connect(titleEdit_, &QLineEdit::textChanged, this, &DocumentDialog::validateInput);
    connect(contentEdit_, &QTextEdit::textChanged, this, &DocumentDialog::validateInput);
    
    // 初始验证
    validateInput();
}

void DocumentDialog::loadDocument(const Document& document)
{
    titleEdit_->setText(document.getTitle());
    
    // 改进文档内容的显示
    QString content = document.getContent().trimmed();
    if (content.isEmpty()) {
        contentEdit_->setPlainText("该文档暂无内容。请在此处添加文档内容。");
    } else {
        // 直接显示文档内容，编码问题应该在DocumentProcessor中解决
        contentEdit_->setPlainText(content);
        
        // 如果内容看起来像是乱码或编码问题，显示提示
        if (content.contains(QChar(0xFFFD)) || // 替换字符，表示编码问题
            (content.length() > 10 && content.count(QRegularExpression("[\\x00-\\x1F]")) > content.length() * 0.1)) {
            // 检测到可能的编码问题
            QMessageBox::information(this, "编码提示", 
                "检测到文档可能存在编码问题。建议：\n"
                "1. 确保原始文档使用UTF-8编码保存\n"
                "2. 重新上传文档\n"
                "3. 手动编辑文档内容");
        }
    }
    
    // 设置类别
    QString category = document.getCategory();
    for (int i = 0; i < categoryCombo_->count(); ++i) {
        if (categoryCombo_->itemData(i).toString() == category) {
            categoryCombo_->setCurrentIndex(i);
            break;
        }
    }
}

Document DocumentDialog::getDocument() const
{
    return document_;
}

void DocumentDialog::onSaveClicked()
{
    if (!validateForm()) {
        return;
    }
    
    // 创建或更新文档
    if (editMode_) {
        document_.setTitle(titleEdit_->text().trimmed());
        document_.setContent(contentEdit_->toPlainText().trimmed());
        document_.setCategory(categoryCombo_->currentData().toString());
        document_.setUpdateTime(QDateTime::currentDateTime());
        
        if (database_->updateDocument(document_)) {
            QMessageBox::information(this, "保存成功", "文档已成功更新！");
            accept();
        } else {
            QMessageBox::critical(this, "保存失败", "更新文档时发生错误！");
        }
    } else {
        Document newDoc(titleEdit_->text().trimmed(), 
                       contentEdit_->toPlainText().trimmed(),
                       categoryCombo_->currentData().toString());
        
        if (database_->addDocument(newDoc)) {
            QMessageBox::information(this, "保存成功", "文档已成功添加！");
            accept();
        } else {
            QMessageBox::critical(this, "保存失败", "添加文档时发生错误！");
        }
    }
}

void DocumentDialog::onCancelClicked()
{
    reject();
}

void DocumentDialog::validateInput()
{
    bool isValid = !titleEdit_->text().trimmed().isEmpty() && 
                   !contentEdit_->toPlainText().trimmed().isEmpty();
    
    saveButton_->setEnabled(isValid);
}

bool DocumentDialog::validateForm()
{
    QString title = titleEdit_->text().trimmed();
    QString content = contentEdit_->toPlainText().trimmed();
    
    if (title.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入文档标题！");
        titleEdit_->setFocus();
        return false;
    }
    
    if (content.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入文档内容！");
        contentEdit_->setFocus();
        return false;
    }
    
    if (title.length() > 100) {
        QMessageBox::warning(this, "验证失败", "文档标题不能超过100个字符！");
        titleEdit_->setFocus();
        return false;
    }
    
    return true;
}
