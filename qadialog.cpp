#include "qadialog.h"
#include <QApplication>
#include <QStyle>

QADialog::QADialog(Database* database, QWidget* parent)
    : QDialog(parent)
    , database_(database)
    , editMode_(false)
    , employeeIdEdit_(nullptr)
    , questionEdit_(nullptr)
    , answerEdit_(nullptr)
    , categoryCombo_(nullptr)
    , buttonBox_(nullptr)
    , saveButton_(nullptr)
    , cancelButton_(nullptr)
{
    setWindowTitle("添加问答");
    setModal(true);
    setMinimumSize(600, 500);
    setupUI();
}

QADialog::QADialog(Database* database, const QuestionRecord& qa, QWidget* parent)
    : QDialog(parent)
    , database_(database)
    , editMode_(true)
    , employeeIdEdit_(nullptr)
    , questionEdit_(nullptr)
    , answerEdit_(nullptr)
    , categoryCombo_(nullptr)
    , buttonBox_(nullptr)
    , saveButton_(nullptr)
    , cancelButton_(nullptr)
    , qa_(qa)
{
    setWindowTitle("编辑问答");
    setModal(true);
    setMinimumSize(600, 500);
    setupUI();
    loadQA(qa);
}

void QADialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 表单布局
    QFormLayout* formLayout = new QFormLayout();
    
    // 员工工号输入
    employeeIdEdit_ = new QLineEdit();
    employeeIdEdit_->setPlaceholderText("请输入员工工号（如：EMP001）");
    employeeIdEdit_->setMinimumHeight(35);
    employeeIdEdit_->setStyleSheet("QLineEdit { padding: 8px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 14px; }"
                                   "QLineEdit:focus { border-color: #3498db; }");
    formLayout->addRow("员工工号:", employeeIdEdit_);
    
    // 类别选择
    categoryCombo_ = new QComboBox();
    categoryCombo_->addItem("考勤", "考勤");
    categoryCombo_->addItem("薪酬", "薪酬");
    categoryCombo_->addItem("绩效", "绩效");
    categoryCombo_->addItem("入职离职", "入职离职");
    categoryCombo_->addItem("员工福利", "员工福利");
    categoryCombo_->addItem("通用", "通用");
    categoryCombo_->setMinimumHeight(35);
    categoryCombo_->setStyleSheet("QComboBox { padding: 8px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 14px; }"
                                  "QComboBox:focus { border-color: #3498db; }"
                                  "QComboBox::drop-down { border: none; }"
                                  "QComboBox::down-arrow { image: none; border-left: 5px solid transparent; border-right: 5px solid transparent; border-top: 5px solid #7f8c8d; }");
    formLayout->addRow("问题类别:", categoryCombo_);
    
    mainLayout->addLayout(formLayout);
    
    // 问题输入
    QLabel* questionLabel = new QLabel("问题:");
    questionLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    mainLayout->addWidget(questionLabel);
    
    questionEdit_ = new QTextEdit();
    questionEdit_->setPlaceholderText("请输入问题内容...");
    questionEdit_->setMaximumHeight(100);
    questionEdit_->setStyleSheet("QTextEdit { padding: 10px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 13px; line-height: 1.5; }"
                                 "QTextEdit:focus { border-color: #3498db; }");
    mainLayout->addWidget(questionEdit_);
    
    // 答案输入
    QLabel* answerLabel = new QLabel("答案:");
    answerLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin-top: 10px;");
    mainLayout->addWidget(answerLabel);
    
    answerEdit_ = new QTextEdit();
    answerEdit_->setPlaceholderText("请输入答案内容...");
    answerEdit_->setStyleSheet("QTextEdit { padding: 10px; border: 2px solid #bdc3c7; border-radius: 5px; font-size: 13px; line-height: 1.5; }"
                               "QTextEdit:focus { border-color: #3498db; }");
    mainLayout->addWidget(answerEdit_);
    
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
    connect(saveButton_, &QPushButton::clicked, this, &QADialog::onSaveClicked);
    connect(cancelButton_, &QPushButton::clicked, this, &QADialog::onCancelClicked);
    connect(employeeIdEdit_, &QLineEdit::textChanged, this, &QADialog::validateInput);
    connect(questionEdit_, &QTextEdit::textChanged, this, &QADialog::validateInput);
    connect(answerEdit_, &QTextEdit::textChanged, this, &QADialog::validateInput);
    
    // 初始验证
    validateInput();
}

void QADialog::loadQA(const QuestionRecord& qa)
{
    employeeIdEdit_->setText(qa.getEmployeeId());
    questionEdit_->setPlainText(qa.getQuestion());
    answerEdit_->setPlainText(qa.getAnswer());
    
    // 设置类别
    QString category = qa.getCategory();
    for (int i = 0; i < categoryCombo_->count(); ++i) {
        if (categoryCombo_->itemData(i).toString() == category) {
            categoryCombo_->setCurrentIndex(i);
            break;
        }
    }
}

QuestionRecord QADialog::getQA() const
{
    return qa_;
}

void QADialog::onSaveClicked()
{
    if (!validateForm()) {
        return;
    }
    
    // 创建或更新问答记录
    if (editMode_) {
        qa_.setEmployeeId(employeeIdEdit_->text().trimmed());
        qa_.setQuestion(questionEdit_->toPlainText().trimmed());
        qa_.setAnswer(answerEdit_->toPlainText().trimmed());
        qa_.setCategory(categoryCombo_->currentData().toString());
        
        if (database_->updateQuestionRecord(qa_)) {
            QMessageBox::information(this, "保存成功", "问答记录已成功更新！");
            accept();
        } else {
            QMessageBox::critical(this, "保存失败", "更新问答记录时发生错误！");
        }
    } else {
        QuestionRecord newQA(employeeIdEdit_->text().trimmed(),
                            questionEdit_->toPlainText().trimmed(),
                            answerEdit_->toPlainText().trimmed(),
                            categoryCombo_->currentData().toString());
        
        if (database_->addQuestionRecord(newQA)) {
            QMessageBox::information(this, "保存成功", "问答记录已成功添加！");
            accept();
        } else {
            QMessageBox::critical(this, "保存失败", "添加问答记录时发生错误！");
        }
    }
}

void QADialog::onCancelClicked()
{
    reject();
}

void QADialog::validateInput()
{
    bool isValid = !employeeIdEdit_->text().trimmed().isEmpty() && 
                   !questionEdit_->toPlainText().trimmed().isEmpty() && 
                   !answerEdit_->toPlainText().trimmed().isEmpty();
    
    saveButton_->setEnabled(isValid);
}

bool QADialog::validateForm()
{
    QString employeeId = employeeIdEdit_->text().trimmed();
    QString question = questionEdit_->toPlainText().trimmed();
    QString answer = answerEdit_->toPlainText().trimmed();
    
    if (employeeId.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入员工工号！");
        employeeIdEdit_->setFocus();
        return false;
    }
    
    if (question.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入问题内容！");
        questionEdit_->setFocus();
        return false;
    }
    
    if (answer.isEmpty()) {
        QMessageBox::warning(this, "验证失败", "请输入答案内容！");
        answerEdit_->setFocus();
        return false;
    }
    
    // 检查工号格式
    QRegularExpression regex("^[A-Za-z][A-Za-z0-9]{4,9}$");
    if (!regex.match(employeeId).hasMatch()) {
        QMessageBox::warning(this, "验证失败", "工号格式不正确！应为字母开头，5-10位字符。");
        employeeIdEdit_->setFocus();
        return false;
    }
    
    if (question.length() > 500) {
        QMessageBox::warning(this, "验证失败", "问题内容不能超过500个字符！");
        questionEdit_->setFocus();
        return false;
    }
    
    if (answer.length() > 2000) {
        QMessageBox::warning(this, "验证失败", "答案内容不能超过2000个字符！");
        answerEdit_->setFocus();
        return false;
    }
    
    return true;
}
