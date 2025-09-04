#include "registerwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QRegularExpressionValidator>
#include <QRegularExpression>
#include <QScrollArea>

RegisterWindow::RegisterWindow(Database* database, QWidget *parent)
    : QWidget(parent)
    , database_(database)
    , mainLayout_(nullptr)
    , registerFrame_(nullptr)
    , titleLabel_(nullptr)
    , subtitleLabel_(nullptr)
    , employeeIdEdit_(nullptr)
    , nameEdit_(nullptr)
    , departmentCombo_(nullptr)
    , roleCombo_(nullptr)
    , passwordEdit_(nullptr)
    , confirmPasswordEdit_(nullptr)
    , registerButton_(nullptr)
    , backButton_(nullptr)
    , statusLabel_(nullptr)
    , employeeIdStatusLabel_(nullptr)
    , passwordStatusLabel_(nullptr)
    , employeeIdValidator_(nullptr)
    , nameValidator_(nullptr)
{
    setupUI();
    setupConnections();
    setupValidators();
    centerWindow();
    applyStyles();
    loadDepartments();
}

RegisterWindow::~RegisterWindow()
{
    // 确保所有子对象被正确释放
    if (employeeIdValidator_) {
        employeeIdValidator_->deleteLater();
        employeeIdValidator_ = nullptr;
    }
    if (nameValidator_) {
        nameValidator_->deleteLater();
        nameValidator_ = nullptr;
    }
    
    qDebug() << "🗑️ 注册窗口已销毁";
}

void RegisterWindow::setupUI()
{
    setWindowTitle("HR制度智能问答系统 - 注册");
    setFixedSize(650, 1000); // 扩大窗口尺寸，取消滚动
    setObjectName("registerWidget");
    
    // 创建主布局 - 直接布局，无需滚动
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(40, 30, 40, 30);
    mainLayout_->setSpacing(20);
    
    // 标题区域
    QLabel* mainTitle = new QLabel("用户注册");
    mainTitle->setObjectName("registerMainTitle");
    mainTitle->setAlignment(Qt::AlignCenter);
    mainTitle->setMinimumHeight(60);
    mainLayout_->addWidget(mainTitle);
    
    QLabel* subtitle = new QLabel("请填写以下信息完成注册");
    subtitle->setObjectName("registerSubtitle");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setMinimumHeight(30);
    mainLayout_->addWidget(subtitle);
    
    mainLayout_->addSpacing(25);
    
    // 创建注册表单卡片
    registerFrame_ = new QFrame();
    registerFrame_->setObjectName("registerCard");
    registerFrame_->setFrameStyle(QFrame::NoFrame);
    registerFrame_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    QVBoxLayout* frameLayout = new QVBoxLayout(registerFrame_);
    frameLayout->setContentsMargins(30, 30, 30, 30);
    frameLayout->setSpacing(18);
    
    // 工号输入区域
    QLabel* employeeIdLabel = new QLabel("工号");
    employeeIdLabel->setObjectName("fieldLabel");
    employeeIdLabel->setMinimumHeight(25);
    frameLayout->addWidget(employeeIdLabel);
    
    employeeIdEdit_ = new QLineEdit();
    employeeIdEdit_->setPlaceholderText("例如：EMP001、HR001");
    employeeIdEdit_->setMaxLength(10);
    employeeIdEdit_->setMinimumHeight(48);
    employeeIdEdit_->setMaximumHeight(48);
    frameLayout->addWidget(employeeIdEdit_);
    
    employeeIdStatusLabel_ = new QLabel();
    employeeIdStatusLabel_->setObjectName("statusLabel");
    employeeIdStatusLabel_->setMinimumHeight(20);
    employeeIdStatusLabel_->setMaximumHeight(20);
    employeeIdStatusLabel_->setVisible(false);
    frameLayout->addWidget(employeeIdStatusLabel_);
    
    frameLayout->addSpacing(8);
    
    // 姓名输入区域
    QLabel* nameLabel = new QLabel("姓名");
    nameLabel->setObjectName("fieldLabel");
    nameLabel->setMinimumHeight(25);
    frameLayout->addWidget(nameLabel);
    
    nameEdit_ = new QLineEdit();
    nameEdit_->setPlaceholderText("请输入您的真实姓名");
    nameEdit_->setMaxLength(20);
    nameEdit_->setMinimumHeight(48);
    nameEdit_->setMaximumHeight(48);
    frameLayout->addWidget(nameEdit_);
    
    frameLayout->addSpacing(8);
    
    // 部门选择区域
    QLabel* departmentLabel = new QLabel("部门");
    departmentLabel->setObjectName("fieldLabel");
    departmentLabel->setMinimumHeight(25);
    frameLayout->addWidget(departmentLabel);
    
    departmentCombo_ = new QComboBox();
    departmentCombo_->setEditable(false);
    departmentCombo_->setMinimumHeight(48);
    departmentCombo_->setMaximumHeight(48);
    frameLayout->addWidget(departmentCombo_);
    
    frameLayout->addSpacing(8);
    
    // 角色选择区域
    QLabel* roleLabel = new QLabel("角色");
    roleLabel->setObjectName("fieldLabel");
    roleLabel->setMinimumHeight(25);
    frameLayout->addWidget(roleLabel);
    
    roleCombo_ = new QComboBox();
    roleCombo_->addItem("普通员工", static_cast<int>(UserRole::Employee));
    roleCombo_->addItem("HR管理员", static_cast<int>(UserRole::HRAdmin));
    roleCombo_->setMinimumHeight(48);
    roleCombo_->setMaximumHeight(48);
    frameLayout->addWidget(roleCombo_);
    
    frameLayout->addSpacing(8);
    
    // 密码输入区域
    QLabel* passwordLabel = new QLabel("密码");
    passwordLabel->setObjectName("fieldLabel");
    passwordLabel->setMinimumHeight(25);
    frameLayout->addWidget(passwordLabel);
    
    passwordEdit_ = new QLineEdit();
    passwordEdit_->setPlaceholderText("请输入密码（至少6位）");
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setMaxLength(20);
    passwordEdit_->setMinimumHeight(48);
    passwordEdit_->setMaximumHeight(48);
    frameLayout->addWidget(passwordEdit_);
    
    passwordStatusLabel_ = new QLabel();
    passwordStatusLabel_->setObjectName("statusLabel");
    passwordStatusLabel_->setMinimumHeight(20);
    passwordStatusLabel_->setMaximumHeight(20);
    passwordStatusLabel_->setVisible(false);
    frameLayout->addWidget(passwordStatusLabel_);
    
    frameLayout->addSpacing(8);
    
    // 确认密码输入区域
    QLabel* confirmPasswordLabel = new QLabel("确认密码");
    confirmPasswordLabel->setObjectName("fieldLabel");
    confirmPasswordLabel->setMinimumHeight(25);
    frameLayout->addWidget(confirmPasswordLabel);
    
    confirmPasswordEdit_ = new QLineEdit();
    confirmPasswordEdit_->setPlaceholderText("请再次输入密码");
    confirmPasswordEdit_->setEchoMode(QLineEdit::Password);
    confirmPasswordEdit_->setMaxLength(20);
    confirmPasswordEdit_->setMinimumHeight(48);
    confirmPasswordEdit_->setMaximumHeight(48);
    frameLayout->addWidget(confirmPasswordEdit_);
    
    confirmPasswordStatusLabel_ = new QLabel();
    confirmPasswordStatusLabel_->setObjectName("statusLabel");
    confirmPasswordStatusLabel_->setMinimumHeight(20);
    confirmPasswordStatusLabel_->setMaximumHeight(20);
    confirmPasswordStatusLabel_->setVisible(false);
    frameLayout->addWidget(confirmPasswordStatusLabel_);
    
    frameLayout->addSpacing(20);
    
    // 注册按钮
    registerButton_ = new QPushButton("立即注册");
    registerButton_->setMinimumHeight(55);
    registerButton_->setMaximumHeight(55);
    registerButton_->setDefault(true);
    frameLayout->addWidget(registerButton_);
    
    frameLayout->addSpacing(12);
    
    // 返回按钮
    backButton_ = new QPushButton("返回登录");
    backButton_->setObjectName("secondaryButton");
    backButton_->setMinimumHeight(45);
    backButton_->setMaximumHeight(45);
    frameLayout->addWidget(backButton_);
    
    frameLayout->addSpacing(15);
    
    // 状态标签 - 使用固定高度避免布局问题
    statusLabel_ = new QLabel();
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setObjectName("statusLabel");
    statusLabel_->setVisible(true); // 始终可见，但透明
    statusLabel_->setFixedHeight(40); // 固定高度，避免布局变化
    statusLabel_->setWordWrap(true);
    statusLabel_->setStyleSheet("color: transparent; background: transparent; border: none;");
    frameLayout->addWidget(statusLabel_);
    
    // 将注册表单添加到主布局
    mainLayout_->addWidget(registerFrame_);
    mainLayout_->addStretch(1); // 底部弹性空间
}

void RegisterWindow::setupConnections()
{
    connect(registerButton_, &QPushButton::clicked, this, &RegisterWindow::onRegisterClicked);
    connect(backButton_, &QPushButton::clicked, this, &RegisterWindow::onBackToLoginClicked);
    connect(roleCombo_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),
            this, &RegisterWindow::onRoleChanged);
    connect(employeeIdEdit_, &QLineEdit::textChanged, this, &RegisterWindow::onEmployeeIdChanged);
    connect(departmentCombo_, static_cast<void(QComboBox::*)(const QString&)>(&QComboBox::currentTextChanged),
            this, &RegisterWindow::onDepartmentChanged);
    connect(passwordEdit_, &QLineEdit::textChanged, this, &RegisterWindow::onPasswordChanged);
    connect(confirmPasswordEdit_, &QLineEdit::textChanged, this, &RegisterWindow::onConfirmPasswordChanged);
}

void RegisterWindow::centerWindow()
{
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void RegisterWindow::applyStyles()
{
    createGradientBackground();
}

void RegisterWindow::setupValidators()
{
    // 工号验证器：字母开头，后跟字母或数字，总长度5-10位
    employeeIdValidator_ = new QRegularExpressionValidator(
        QRegularExpression("^[A-Za-z][A-Za-z0-9]{4,9}$"), this);
    employeeIdEdit_->setValidator(employeeIdValidator_);
    
    // 姓名验证器：中文或字母，长度2-20位（暂时移除验证器以修复输入问题）
    // nameValidator_ = new QRegularExpressionValidator(
    //     QRegularExpression("^[\\u4e00-\\u9fa5A-Za-z\\s]{2,20}$"), this);
    // nameEdit_->setValidator(nameValidator_);
    
    // 暂时不设置验证器，确保姓名可以正常输入
    nameEdit_->setValidator(nullptr);
}

void RegisterWindow::loadDepartments()
{
    // 加载部门列表
    QStringList departments = {
        "人事部",
        "技术部",
        "市场部",
        "财务部",
        "运营部",
        "销售部",
        "客服部",
        "行政部"
    };
    
    departmentCombo_->addItems(departments);
    
    // 修复部门和角色选择框的样式问题
    QString comboBoxStyle = "QComboBox { "
                           "padding: 12px 15px; "
                           "border: 2px solid #ddd; "
                           "border-radius: 6px; "
                           "font-size: 14px; "
                           "background: white; "
                           "color: #2c3e50; }"
                           "QComboBox:focus { "
                           "border: 2px solid #3498db; }"
                           "QComboBox::drop-down { "
                           "border: none; width: 25px; }"
                           "QComboBox::down-arrow { "
                           "width: 8px; height: 8px; }"
                           "QComboBox QAbstractItemView { "
                           "background: white; "
                           "border: 2px solid #ddd; "
                           "border-radius: 6px; "
                           "selection-background-color: #3498db; "
                           "selection-color: white; }";
    
    departmentCombo_->setStyleSheet(comboBoxStyle);
    roleCombo_->setStyleSheet(comboBoxStyle);
}

void RegisterWindow::onRegisterClicked()
{
    // 防止重复点击
    if (!registerButton_->isEnabled()) {
        return;
    }
    
    if (!validateInput()) {
        return;
    }
    
    // 禁用按钮，防止重复提交
    registerButton_->setEnabled(false);
    registerButton_->setText("注册中...");
    backButton_->setEnabled(false);
    
    QString employeeId = employeeIdEdit_->text().trimmed();
    QString name = nameEdit_->text().trimmed();
    QString department = departmentCombo_->currentText();
    UserRole role = static_cast<UserRole>(roleCombo_->currentData().toInt());
    QString password = passwordEdit_->text();
    
    // 检查工号是否已存在
    User existingUser = database_->getUser(employeeId);
    if (!existingUser.getEmployeeId().isEmpty()) {
        showError("该工号已经注册，请使用其他工号");
        return;
    }
    
    // 创建用户
    // 所有新注册账号（包括普通员工）均需管理员审核激活
    UserStatus status = UserStatus::Pending;
    User newUser(employeeId, name, department, role, status);
    
            // 添加到数据库
        if (database_->addUser(newUser, password)) {
            // 清空表单字段
            clearFields();
            
            // 所有角色均需审核
            QMessageBox::information(this, "注册成功", 
                "账号已提交审核，请等待管理员审核通过后再登录。\n\n点击确定后返回登录界面。");
            
            // 返回登录界面
            emit showLoginWindow();
            this->close();
        } else {
            showError("注册失败，请稍后重试");
            // 重新启用按钮
            registerButton_->setEnabled(true);
            registerButton_->setText("立即注册");
            backButton_->setEnabled(true);
        }
}

void RegisterWindow::onBackToLoginClicked()
{
    this->close(); // 关闭当前注册窗口
    emit showLoginWindow();
}

void RegisterWindow::onRoleChanged(const QString& role)
{
    Q_UNUSED(role)
    updateButtonStates();
}

void RegisterWindow::onEmployeeIdChanged(const QString& employeeId)
{
    if (validateEmployeeId()) {
        employeeIdStatusLabel_->setText("✓ 工号格式正确");
        employeeIdStatusLabel_->setStyleSheet("color: #27ae60; font-size: 12px; font-weight: bold;");
    } else {
        if (employeeId.isEmpty()) {
            employeeIdStatusLabel_->setText("");
        } else {
            employeeIdStatusLabel_->setText("✗ 工号格式错误，请检查格式");
            employeeIdStatusLabel_->setStyleSheet("color: #e74c3c; font-size: 12px; font-weight: bold;");
        }
    }
    updateButtonStates();
}

void RegisterWindow::onDepartmentChanged(const QString& department)
{
    Q_UNUSED(department)
    updateButtonStates();
}

void RegisterWindow::onPasswordChanged(const QString& password)
{
    if (validatePassword()) {
        passwordStatusLabel_->setText("✓ 密码强度良好");
        passwordStatusLabel_->setStyleSheet("color: #27ae60; font-size: 12px; font-weight: bold;");
    } else {
        if (password.isEmpty()) {
            passwordStatusLabel_->setText("");
        } else {
            passwordStatusLabel_->setText("✗ 密码至少6位字符");
            passwordStatusLabel_->setStyleSheet("color: #e74c3c; font-size: 12px; font-weight: bold;");
        }
    }
    updateButtonStates();
}

void RegisterWindow::onConfirmPasswordChanged(const QString& confirmPassword)
{
    Q_UNUSED(confirmPassword)
    
    // 添加密码一致性验证提示
    if (!confirmPassword.isEmpty() && !passwordEdit_->text().isEmpty()) {
        if (passwordEdit_->text() == confirmPassword) {
            confirmPasswordEdit_->setStyleSheet("QLineEdit { "
                                              "padding: 12px 15px; "
                                              "border: 2px solid #27ae60; "
                                              "border-radius: 6px; "
                                              "font-size: 14px; "
                                              "background: white; }");
            confirmPasswordStatusLabel_->setText("✓ 密码一致");
            confirmPasswordStatusLabel_->setStyleSheet("color: #27ae60; font-size: 12px; font-weight: bold;");
        } else {
            confirmPasswordEdit_->setStyleSheet("QLineEdit { "
                                              "padding: 12px 15px; "
                                              "border: 2px solid #e74c3c; "
                                              "border-radius: 6px; "
                                              "font-size: 14px; "
                                              "background: white; }");
            confirmPasswordStatusLabel_->setText("✗ 密码不一致");
            confirmPasswordStatusLabel_->setStyleSheet("color: #e74c3c; font-size: 12px; font-weight: bold;");
        }
    } else {
        confirmPasswordEdit_->setStyleSheet("QLineEdit { "
                                          "padding: 12px 15px; "
                                          "border: 2px solid #ddd; "
                                          "border-radius: 6px; "
                                          "font-size: 14px; "
                                          "background: white; }"
                                          "QLineEdit:focus { "
                                          "border: 2px solid #3498db; }");
        if (confirmPassword.isEmpty()) {
            confirmPasswordStatusLabel_->setText("");
        }
    }
    
    updateButtonStates();
}

void RegisterWindow::showError(const QString& message)
{
    statusLabel_->setText(message);
    statusLabel_->setStyleSheet("color: #e74c3c; font-size: 13px; font-weight: bold; "
                               "padding: 8px; border-radius: 6px; "
                               "background-color: #ffeaea; border: 1px solid #e74c3c;");
    
    // 3秒后隐藏错误信息
    QTimer::singleShot(3000, [this]() {
        if (this && !this->isHidden()) {
            statusLabel_->setText(""); // 清空文本
            statusLabel_->setStyleSheet("color: transparent; background: transparent; border: none;");
        }
    });
}

void RegisterWindow::showSuccess(const QString& message)
{
    statusLabel_->setText(message);
    statusLabel_->setStyleSheet("color: #27ae60; font-size: 13px; font-weight: bold; "
                               "padding: 8px; border-radius: 6px; "
                               "background-color: #eafaf1; border: 1px solid #27ae60;");
    
    // 1秒后隐藏成功信息，避免与后续操作冲突
    QTimer::singleShot(1000, [this]() {
        if (this && !this->isHidden()) {
            statusLabel_->setText(""); // 清空文本
            statusLabel_->setStyleSheet("color: transparent; background: transparent; border: none;");
        }
    });
}

void RegisterWindow::clearFields()
{
    employeeIdEdit_->clear();
    nameEdit_->clear();
    passwordEdit_->clear();
    confirmPasswordEdit_->clear();
    departmentCombo_->setCurrentIndex(0);
    roleCombo_->setCurrentIndex(0);
    statusLabel_->setText(""); // 清空状态信息
    statusLabel_->setStyleSheet("color: transparent; background: transparent; border: none;");
    
    // 清空状态标签
    employeeIdStatusLabel_->setText("");
    passwordStatusLabel_->setText("");
    confirmPasswordStatusLabel_->setText("");
    
    // 重置输入框样式
    employeeIdEdit_->setStyleSheet("QLineEdit { "
                                  "padding: 12px 15px; "
                                  "border: 2px solid #ddd; "
                                  "border-radius: 6px; "
                                  "font-size: 14px; "
                                  "background: white; }"
                                  "QLineEdit:focus { "
                                  "border: 2px solid #3498db; }");
    passwordEdit_->setStyleSheet("QLineEdit { "
                                "padding: 12px 15px; "
                                "border: 2px solid #ddd; "
                                "border-radius: 6px; "
                                "font-size: 14px; "
                                "background: white; }"
                                "QLineEdit:focus { "
                                "border: 2px solid #3498db; }");
    confirmPasswordEdit_->setStyleSheet("QLineEdit { "
                                       "padding: 12px 15px; "
                                       "border: 2px solid #ddd; "
                                       "border-radius: 6px; "
                                       "font-size: 14px; "
                                       "background: white; }"
                                       "QLineEdit:focus { "
                                       "border: 2px solid #3498db; }");
}

bool RegisterWindow::validateInput()
{
    if (!validateEmployeeId()) {
        showError("请输入正确的工号格式");
        employeeIdEdit_->setFocus();
        return false;
    }
    
    if (!validateName()) {
        showError("请输入正确的姓名");
        nameEdit_->setFocus();
        return false;
    }
    
    if (!validateDepartment()) {
        showError("请选择部门");
        departmentCombo_->setFocus();
        return false;
    }
    
    if (!validatePassword()) {
        showError("密码至少6位字符");
        passwordEdit_->setFocus();
        return false;
    }
    
    if (passwordEdit_->text() != confirmPasswordEdit_->text()) {
        showError("两次输入的密码不一致");
        confirmPasswordEdit_->setFocus();
        return false;
    }
    
    return true;
}

bool RegisterWindow::validateEmployeeId()
{
    QString employeeId = employeeIdEdit_->text().trimmed();
    if (employeeId.isEmpty() || employeeId.length() < 5 || employeeId.length() > 10) {
        return false;
    }
    int pos = 0;
    return employeeIdValidator_->validate(employeeId, pos) == QValidator::Acceptable;
}

bool RegisterWindow::validatePassword()
{
    return passwordEdit_->text().length() >= 6;
}

bool RegisterWindow::validateName()
{
    QString name = nameEdit_->text().trimmed();
    return !name.isEmpty() && 
           name.length() >= 2 && 
           name.length() <= 20;
}

bool RegisterWindow::validateDepartment()
{
    return departmentCombo_->currentIndex() >= 0;
}

void RegisterWindow::updateStatusLabels()
{
    onEmployeeIdChanged(employeeIdEdit_->text());
    onPasswordChanged(passwordEdit_->text());
    onConfirmPasswordChanged(confirmPasswordEdit_->text());
}

void RegisterWindow::animateRegistration()
{
    registerButton_->setEnabled(false);
    registerButton_->setText("注册中...");
    
    QTimer::singleShot(1000, [this]() {
        registerButton_->setEnabled(true);
        registerButton_->setText("注册");
    });
}

void RegisterWindow::createGradientBackground()
{
    setStyleSheet(
        "QWidget#registerWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #667eea, stop:1 #764ba2);"
        "}"
        "QFrame#registerFrame {"
        "    background: white;"
        "    border-radius: 15px;"
        "    border: 2px solid rgba(255, 255, 255, 0.8);"
        "}"
    );
}

void RegisterWindow::setupAnimations()
{
    // 可以添加窗口显示动画等
}

void RegisterWindow::updateButtonStates()
{
    bool canRegister = validateEmployeeId() && 
                      validateName() && 
                      validateDepartment() && 
                      validatePassword() &&
                      (passwordEdit_->text() == confirmPasswordEdit_->text());
    
    registerButton_->setEnabled(canRegister);
}
