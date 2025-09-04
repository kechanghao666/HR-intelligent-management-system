#include "loginwindow.h"
#include "registerwindow.h"
#include <QSettings>
#include <QCryptographicHash>
#include <QInputDialog>
#include <QTimer>
#include <QFont>
#include <QDebug>

LoginWindow::LoginWindow(Database* database, QWidget *parent)
    : QWidget(parent)
    , database_(database)
    , mainLayout_(nullptr)
    , loginFrame_(nullptr)
    , titleLabel_(nullptr)
    , employeeIdEdit_(nullptr)
    , passwordEdit_(nullptr)
    , loginButton_(nullptr)
    , registerButton_(nullptr)
    , forgotPasswordButton_(nullptr)
    , rememberPasswordCheck_(nullptr)
    , autoLoginCheck_(nullptr)
    , statusLabel_(nullptr)
{
    setupUI();
    setupConnections();
    centerWindow();
    applyStyles();
    loadLoginSettings();
}

LoginWindow::~LoginWindow()
{
}

void LoginWindow::setupUI()
{
    setWindowTitle("HR制度智能问答系统 - 登录");
    setFixedSize(1200, 800);  // 适当缩小界面尺寸
    setObjectName("loginWidget");
    
    // 创建主布局
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    mainLayout_->setSpacing(0);
    
    // 创建主容器
    QWidget* centerContainer = new QWidget();
    centerContainer->setObjectName("centerContainer");
    QHBoxLayout* centerLayout = new QHBoxLayout(centerContainer);
    centerLayout->setContentsMargins(0, 0, 0, 0);
    centerLayout->setSpacing(0);
    
    // 左侧品牌展示区域
    QWidget* leftDecoration = new QWidget();
    leftDecoration->setObjectName("leftDecoration");
    leftDecoration->setMinimumWidth(600);
    leftDecoration->setMaximumWidth(600);
    
    // 左侧品牌内容
    QVBoxLayout* decorationLayout = new QVBoxLayout(leftDecoration);
    decorationLayout->setContentsMargins(60, 80, 40, 80);
    decorationLayout->setSpacing(40);
    
    // 品牌标题区域
    QWidget* titleSection = new QWidget();
    QVBoxLayout* titleLayout = new QVBoxLayout(titleSection);
    titleLayout->setContentsMargins(0, 0, 0, 0);
    titleLayout->setSpacing(15);
    
    QLabel* techTitle = new QLabel("智能化HR管理系统");
    techTitle->setObjectName("brandTitle");
    techTitle->setAlignment(Qt::AlignLeft);
    titleLayout->addWidget(techTitle);
    
    QLabel* subtitle = new QLabel("企业级人力资源智能管理平台");
    subtitle->setObjectName("brandSubtitle");
    subtitle->setAlignment(Qt::AlignLeft);
    titleLayout->addWidget(subtitle);
    
    decorationLayout->addWidget(titleSection);
    
    // 特性展示区域
    QWidget* featuresSection = new QWidget();
    QVBoxLayout* featuresLayout = new QVBoxLayout(featuresSection);
    featuresLayout->setContentsMargins(0, 0, 0, 0);
    featuresLayout->setSpacing(30);
    
    // 创建特性项目
    auto createFeatureItem = [](const QString& title, const QString& desc) {
        QWidget* item = new QWidget();
        QHBoxLayout* itemLayout = new QHBoxLayout(item);
        itemLayout->setContentsMargins(0, 0, 0, 0);
        itemLayout->setSpacing(15);
        
        QLabel* icon = new QLabel("●");
        icon->setObjectName("featureIcon");
        icon->setFixedSize(8, 8);
        icon->setAlignment(Qt::AlignTop);
        
        QWidget* textWidget = new QWidget();
        QVBoxLayout* textLayout = new QVBoxLayout(textWidget);
        textLayout->setContentsMargins(0, 0, 0, 0);
        textLayout->setSpacing(5);
        
        QLabel* titleLabel = new QLabel(title);
        titleLabel->setObjectName("featureTitle");
        
        QLabel* descLabel = new QLabel(desc);
        descLabel->setObjectName("featureDesc");
        descLabel->setWordWrap(true);
        
        textLayout->addWidget(titleLabel);
        textLayout->addWidget(descLabel);
        
        itemLayout->addWidget(icon);
        itemLayout->addWidget(textWidget);
        
        return item;
    };
    
    featuresLayout->addWidget(createFeatureItem("AI智能问答", "基于自然语言处理的智能问答系统"));
    featuresLayout->addWidget(createFeatureItem("数据可视化", "直观的数据报表和分析图表"));
    featuresLayout->addWidget(createFeatureItem("企业级安全", "企业级数据安全保障"));
    featuresLayout->addWidget(createFeatureItem("高效协同办公", "优化工作流程，提升效率"));
    
    decorationLayout->addWidget(featuresSection);
    decorationLayout->addStretch();
    
    // 右侧登录区域
    QWidget* loginSection = new QWidget();
    loginSection->setObjectName("loginSection");
    loginSection->setMinimumWidth(600);
    loginSection->setMaximumWidth(600);
    
    QVBoxLayout* loginSectionLayout = new QVBoxLayout(loginSection);
    loginSectionLayout->setContentsMargins(60, 40, 60, 40);
    loginSectionLayout->setSpacing(0);
    loginSectionLayout->setAlignment(Qt::AlignCenter);
    
    // 登录卡片容器
    loginFrame_ = new QFrame();
    loginFrame_->setObjectName("loginCard");
    loginFrame_->setMaximumWidth(480);
    loginFrame_->setFrameStyle(QFrame::NoFrame);
    
    QVBoxLayout* frameLayout = new QVBoxLayout(loginFrame_);
    frameLayout->setContentsMargins(30, 30, 30, 30);
    frameLayout->setSpacing(12);
    
    // 现代化标题设计
    titleLabel_ = new QLabel("HR智能问答系统");
    titleLabel_->setObjectName("titleLabel");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setWordWrap(true);
    frameLayout->addWidget(titleLabel_);
    
    // 添加副标题
    QLabel* subtitleLabel = new QLabel("企业级人力资源智能管理平台");
    subtitleLabel->setObjectName("subtitleLabel");
    subtitleLabel->setAlignment(Qt::AlignCenter);
    subtitleLabel->setWordWrap(true);
    frameLayout->addWidget(subtitleLabel);
    
    // 添加分隔空间
    frameLayout->addSpacing(16);
    
    // 创建角色选择卡片容器
    QWidget* roleCard = new QWidget();
    roleCard->setObjectName("cardContainer");
    QVBoxLayout* roleCardLayout = new QVBoxLayout(roleCard);
    roleCardLayout->setContentsMargins(12, 12, 12, 12);
    roleCardLayout->setSpacing(8);
    
    QLabel* roleLabel = new QLabel("选择身份");
    roleLabel->setObjectName("sectionTitle");
    roleCardLayout->addWidget(roleLabel);
    
    QHBoxLayout* roleLayout = new QHBoxLayout();
    roleLayout->setSpacing(20);
    roleGroup_ = new QButtonGroup(this);
    
    QRadioButton* employeeRadio = new QRadioButton("普通员工");
    QRadioButton* adminRadio = new QRadioButton("HR管理员");
    
    employeeRadio->setChecked(true);
    
    roleGroup_->addButton(employeeRadio, 0);
    roleGroup_->addButton(adminRadio, 1);
    
    roleLayout->addWidget(employeeRadio);
    roleLayout->addWidget(adminRadio);
    roleLayout->addStretch();
    
    roleCardLayout->addLayout(roleLayout);
    frameLayout->addWidget(roleCard);
    
    // 创建输入表单卡片
    QWidget* formCard = new QWidget();
    formCard->setObjectName("cardContainer");
    QVBoxLayout* formCardLayout = new QVBoxLayout(formCard);
    formCardLayout->setContentsMargins(12, 12, 12, 12);
    formCardLayout->setSpacing(10);
    
    // 工号输入
    QLabel* employeeIdLabel = new QLabel("工号");
    employeeIdLabel->setObjectName("sectionTitle");
    formCardLayout->addWidget(employeeIdLabel);
    
    employeeIdEdit_ = new QLineEdit();
    employeeIdEdit_->setMaxLength(10);
    employeeIdEdit_->setMinimumHeight(40);
    employeeIdEdit_->setMaximumHeight(40);
    employeeIdEdit_->setPlaceholderText("请输入您的工号");
    formCardLayout->addWidget(employeeIdEdit_);
    
    // 密码输入
    QLabel* passwordLabel = new QLabel("密码");
    passwordLabel->setObjectName("sectionTitle");
    formCardLayout->addWidget(passwordLabel);
    
    // 创建密码输入框容器
    QWidget* passwordContainer = new QWidget();
    passwordContainer->setMinimumHeight(40);
    passwordContainer->setMaximumHeight(40);
    QHBoxLayout* passwordLayout = new QHBoxLayout(passwordContainer);
    passwordLayout->setContentsMargins(0, 0, 0, 0);
    passwordLayout->setSpacing(5);
    
    passwordEdit_ = new QLineEdit();
    passwordEdit_->setEchoMode(QLineEdit::Password);
    passwordEdit_->setMaxLength(20);
    passwordEdit_->setMinimumHeight(40);
    passwordEdit_->setMaximumHeight(40);
    passwordEdit_->setPlaceholderText("请输入您的密码");
    
    // 创建显示/隐藏密码按钮
    QPushButton* togglePasswordBtn = new QPushButton();
    togglePasswordBtn->setFixedSize(35, 35);
    
    // 尝试加载图标，如果失败则使用文本备用方案
    // 初始状态：密码隐藏，应该显示闭眼图标
    QIcon eyeIcon(":/icons/eye-closed.svg");
    bool useIcon = false;
    
    if (!eyeIcon.isNull() && !eyeIcon.pixmap(QSize(16, 16)).isNull()) {
        togglePasswordBtn->setIcon(eyeIcon);
        togglePasswordBtn->setIconSize(QSize(20, 20));
        useIcon = true;
        qDebug() << "成功加载eye-closed.svg图标";
    } else {
        qDebug() << "警告：无法加载eye-closed.svg图标，使用文本替代";
        // 使用Unicode眼睛符号作为备用（闭眼状态）
        togglePasswordBtn->setText("🙈");
        togglePasswordBtn->setToolTip("点击显示/隐藏密码");
    }
    
    // 存储是否使用图标的状态
    togglePasswordBtn->setProperty("useIcon", useIcon);
    
    // 设置按钮样式
    if (useIcon) {
        togglePasswordBtn->setStyleSheet("QPushButton { "
                                        "border: 2px solid #bdc3c7; "
                                        "border-radius: 8px; "
                                        "background: white; "
                                        "padding: 5px; }"
                                        "QPushButton:hover { "
                                        "border: 2px solid #3498db; "
                                        "background: #f8f9fa; }");
    } else {
        // 文本模式的样式
        togglePasswordBtn->setStyleSheet("QPushButton { "
                                        "border: 2px solid #bdc3c7; "
                                        "border-radius: 8px; "
                                        "background: white; "
                                        "padding: 5px; "
                                        "font-size: 18px; "
                                        "font-family: 'Segoe UI Emoji', sans-serif; }"
                                        "QPushButton:hover { "
                                        "border: 2px solid #3498db; "
                                        "background: #f8f9fa; }");
    }
    
    // 连接显示/隐藏密码功能
    connect(togglePasswordBtn, &QPushButton::clicked, [this, togglePasswordBtn]() {
        bool useIcon = togglePasswordBtn->property("useIcon").toBool();
        
        if (passwordEdit_->echoMode() == QLineEdit::Password) {
            // 当前密码隐藏，点击后显示密码，图标变为睁眼
            passwordEdit_->setEchoMode(QLineEdit::Normal);
            if (useIcon) {
                QIcon openEyeIcon(":/icons/eye-open.svg");
                if (!openEyeIcon.isNull()) {
                    togglePasswordBtn->setIcon(openEyeIcon);
                } else {
                    togglePasswordBtn->setText("👁");
                }
            } else {
                togglePasswordBtn->setText("👁");
            }
        } else {
            // 当前密码显示，点击后隐藏密码，图标变为闭眼
            passwordEdit_->setEchoMode(QLineEdit::Password);
            if (useIcon) {
                QIcon closedEyeIcon(":/icons/eye-closed.svg");
                if (!closedEyeIcon.isNull()) {
                    togglePasswordBtn->setIcon(closedEyeIcon);
                } else {
                    togglePasswordBtn->setText("🙈");
                }
            } else {
                togglePasswordBtn->setText("🙈");
            }
        }
    });
    
    passwordLayout->addWidget(passwordEdit_, 1);  // 让密码框占据大部分空间
    passwordLayout->addWidget(togglePasswordBtn, 0);  // 按钮固定大小
    
    formCardLayout->addWidget(passwordContainer);
    
    // 记住密码和自动登录选项
    QHBoxLayout* optionsLayout = new QHBoxLayout();
    optionsLayout->setSpacing(20);
    
    rememberPasswordCheck_ = new QCheckBox("记住密码");
    autoLoginCheck_ = new QCheckBox("自动登录");
    
    optionsLayout->addWidget(rememberPasswordCheck_);
    optionsLayout->addWidget(autoLoginCheck_);
    optionsLayout->addStretch();
    
    formCardLayout->addLayout(optionsLayout);
    frameLayout->addWidget(formCard);
    
    // 登录按钮区域
    frameLayout->addSpacing(10);
    
    loginButton_ = new QPushButton("立即登录");
    loginButton_->setMinimumHeight(42);
    loginButton_->setMaximumHeight(42);
    loginButton_->setDefault(true);
    frameLayout->addWidget(loginButton_);
    
    // 次要操作按钮布局
    frameLayout->addSpacing(8);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);
    
    registerButton_ = new QPushButton("注册账号");
    registerButton_->setObjectName("secondaryButton");
    registerButton_->setMinimumHeight(35);
    registerButton_->setMaximumHeight(35);
    
    forgotPasswordButton_ = new QPushButton("忘记密码");
    forgotPasswordButton_->setObjectName("secondaryButton");
    forgotPasswordButton_->setMinimumHeight(35);
    forgotPasswordButton_->setMaximumHeight(35);
    
    buttonLayout->addWidget(registerButton_);
    buttonLayout->addWidget(forgotPasswordButton_);
    
    frameLayout->addLayout(buttonLayout);
    
    // 状态标签 - 修复布局问题，预留固定空间
    frameLayout->addSpacing(8);
    
    statusLabel_ = new QLabel();
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet("color: transparent; font-size: 14px; font-weight: bold; padding: 10px; border-radius: 8px;");
    statusLabel_->setVisible(true); // 始终可见，但透明
    statusLabel_->setFixedHeight(40); // 固定高度，避免布局变化
    statusLabel_->setWordWrap(true);
    statusLabel_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    frameLayout->addWidget(statusLabel_);
    
    // 将登录卡片添加到登录区域
    loginSectionLayout->addWidget(loginFrame_);
    
    // 组装左右分栏布局
    centerLayout->addWidget(leftDecoration);
    centerLayout->addWidget(loginSection);
    
    mainLayout_->addWidget(centerContainer);
}

void LoginWindow::setupConnections()
{
    connect(loginButton_, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(registerButton_, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
    connect(forgotPasswordButton_, &QPushButton::clicked, this, &LoginWindow::onForgotPasswordClicked);
    connect(rememberPasswordCheck_, &QCheckBox::toggled, this, &LoginWindow::onRememberPasswordChanged);
    connect(autoLoginCheck_, &QCheckBox::toggled, this, &LoginWindow::onAutoLoginChanged);
    
    // 回车键登录
    connect(employeeIdEdit_, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
    connect(passwordEdit_, &QLineEdit::returnPressed, this, &LoginWindow::onLoginClicked);
}

void LoginWindow::centerWindow()
{
    QScreen* screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void LoginWindow::applyStyles()
{
    createGradientBackground();
}

void LoginWindow::onLoginClicked()
{
    if (!validateInput()) {
        return;
    }
    
    QString employeeId = employeeIdEdit_->text().trimmed();
    QString password = passwordEdit_->text();
    
    // 验证用户
    if (!database_->verifyPassword(employeeId, password)) {
        showError("登录失败：工号或密码错误");
        return;
    }
    
    // 获取选择的角色
    bool isAdmin = (roleGroup_->checkedId() == 1);
    
    // 获取用户信息
    User user = database_->getUser(employeeId);
    if (user.getEmployeeId().isEmpty()) {
        showError("登录失败：用户不存在");
        return;
    }
    
    // 检查用户状态和角色
    if (!user.canLogin()) {
        if (user.getStatus() == UserStatus::Pending) {
            showError("您的账号还在审核中，请等待管理员审核");
        } else {
            showError("您的账号已被禁用，请联系管理员");
        }
        return;
    }
    
    // 验证用户角色是否匹配
    if (isAdmin && !user.isHRAdmin()) {
        showError("登录失败：您不是HR管理员");
        return;
    } else if (!isAdmin && user.isHRAdmin()) {
        showError("请选择正确的用户角色");
        return;
    }
    
    // 更新登录时间
    database_->updateLastLoginTime(employeeId);
    
    // 保存登录设置
    if (rememberPasswordCheck_->isChecked() || autoLoginCheck_->isChecked()) {
        saveLoginSettings();
    }
    
    showSuccess("登录成功！");
    
    // 发出登录成功信号
    emit loginSuccessful(user);
}

void LoginWindow::onRegisterClicked()
{
    emit showRegisterWindow();
}

void LoginWindow::onForgotPasswordClicked()
{
    // 改进的密码重置功能，需要身份验证
    QString employeeId = employeeIdEdit_->text().trimmed();
    if (employeeId.isEmpty()) {
        showError("请先输入工号");
        return;
    }
    
    User user = database_->getUser(employeeId);
    if (user.getEmployeeId().isEmpty()) {
        showError("用户不存在");
        return;
    }
    
    // 第一步：验证部门信息
    bool ok;
    QString department = QInputDialog::getItem(this, "身份验证", 
        "请选择您的部门:", 
        QStringList() << "人事部" << "技术部" << "市场部" << "财务部" << "运营部" << "销售部" << "客服部" << "行政部", 
        0, false, &ok);
    
    if (!ok || department.isEmpty()) {
        return;
    }
    
    if (department != user.getDepartment()) {
        showError("部门信息不匹配，验证失败");
        return;
    }
    
    // 第二步：验证姓名
    QString name = QInputDialog::getText(this, "身份验证", 
        "请输入您的姓名:", QLineEdit::Normal, "", &ok);
    
    if (!ok || name.isEmpty()) {
        return;
    }
    
    if (name != user.getName()) {
        showError("姓名信息不匹配，验证失败");
        return;
    }
    
    // 第三步：输入新密码
    QString newPassword = QInputDialog::getText(this, "密码重置", 
        "请输入新密码:", QLineEdit::Password, "", &ok);
    
    if (!ok || newPassword.isEmpty()) {
        return;
    }
    
    if (newPassword.length() < 6) {
        showError("密码长度至少6位");
        return;
    }
    
    // 第四步：确认新密码
    QString confirmPassword = QInputDialog::getText(this, "密码重置", 
        "请再次输入新密码:", QLineEdit::Password, "", &ok);
    
    if (!ok || confirmPassword.isEmpty()) {
        return;
    }
    
    if (newPassword != confirmPassword) {
        showError("两次输入的密码不一致");
        return;
    }
    
    // 执行密码重置
    if (database_->resetPassword(employeeId, newPassword)) {
        showSuccess("密码重置成功！请使用新密码登录");
        passwordEdit_->clear();
    } else {
        showError("密码重置失败，请稍后重试");
    }
}

void LoginWindow::onRememberPasswordChanged(bool checked)
{
    // 如果取消记住密码，也要取消自动登录
    if (!checked) {
        autoLoginCheck_->setChecked(false);
    }
    
    // 立即保存设置
    saveLoginSettings();
}

void LoginWindow::onAutoLoginChanged(bool checked)
{
    // 如果选择自动登录，必须同时记住密码
    if (checked) {
        rememberPasswordCheck_->setChecked(true);
    }
    
    // 立即保存设置
    saveLoginSettings();
}

void LoginWindow::showError(const QString& message)
{
    statusLabel_->setText(message);
    statusLabel_->setStyleSheet("color: #e74c3c; font-size: 13px; font-weight: bold; "
                               "padding: 10px; border-radius: 6px; "
                               "background-color: #ffeaea; border: 1px solid #e74c3c;");
    
    // 3秒后隐藏错误信息
    QTimer::singleShot(3000, [this]() {
        statusLabel_->setText(""); // 清空文本
        statusLabel_->setStyleSheet("color: transparent; font-size: 14px; font-weight: bold; "
                                   "padding: 15px; border-radius: 8px; background: transparent; border: none;");
    });
}

void LoginWindow::showSuccess(const QString& message)
{
    statusLabel_->setText(message);
    statusLabel_->setStyleSheet("color: #27ae60; font-size: 13px; font-weight: bold; "
                               "padding: 10px; border-radius: 6px; "
                               "background-color: #eafaf1; border: 1px solid #27ae60;");
    
    // 2秒后隐藏成功信息
    QTimer::singleShot(2000, [this]() {
        statusLabel_->setText(""); // 清空文本
        statusLabel_->setStyleSheet("color: transparent; font-size: 14px; font-weight: bold; "
                                   "padding: 15px; border-radius: 8px; background: transparent; border: none;");
    });
}

void LoginWindow::clearFields()
{
    employeeIdEdit_->clear();
    passwordEdit_->clear();
    statusLabel_->setText(""); // 清空状态信息
    statusLabel_->setStyleSheet("color: transparent; font-size: 14px; font-weight: bold; "
                               "padding: 15px; border-radius: 8px; background: transparent; border: none;");
}

bool LoginWindow::validateInput()
{
    if (employeeIdEdit_->text().trimmed().isEmpty()) {
        showError("请输入工号");
        employeeIdEdit_->setFocus();
        return false;
    }
    
    if (passwordEdit_->text().isEmpty()) {
        showError("请输入密码");
        passwordEdit_->setFocus();
        return false;
    }
    
    return true;
}

void LoginWindow::saveLoginSettings()
{
    QSettings settings;
    
    // 保存复选框状态
    settings.setValue("loginWindow/rememberPassword", rememberPasswordCheck_->isChecked());
    settings.setValue("loginWindow/autoLogin", autoLoginCheck_->isChecked());
    
    if (rememberPasswordCheck_->isChecked()) {
        // 保存工号
        settings.setValue("loginWindow/employeeId", employeeIdEdit_->text());
        // 保存身份（0=员工,1=HR管理员）
        settings.setValue("loginWindow/role", roleGroup_ ? roleGroup_->checkedId() : 0);
        
        // 【安全修复】如果同时选择了自动登录，使用更安全的方式保存密码
        if (autoLoginCheck_->isChecked() && !passwordEdit_->text().isEmpty()) {
            // 使用简单加密而非明文Base64存储
            QByteArray passwordData = passwordEdit_->text().toUtf8();
            // 简单的XOR加密（密钥可以从系统信息生成）
            QByteArray key = QByteArray::fromHex("4A5F6C8E9B2D3E7A"); // 示例密钥，实际应用中应动态生成
            for (int i = 0; i < passwordData.size(); ++i) {
                passwordData[i] = passwordData[i] ^ key[i % key.size()];
            }
            settings.setValue("loginWindow/password", passwordData.toBase64());
            qDebug() << "[安全] 密码已使用加密方式存储";
        }
    } else {
        // 如果取消记住密码，清除所有保存的数据
        settings.remove("loginWindow/employeeId");
        settings.remove("loginWindow/role");
        settings.remove("loginWindow/password");
        settings.setValue("loginWindow/autoLogin", false);
    }
}

void LoginWindow::loadLoginSettings()
{
    QSettings settings;
    
    bool rememberPassword = settings.value("loginWindow/rememberPassword", false).toBool();
    bool autoLogin = settings.value("loginWindow/autoLogin", false).toBool();
    
    // 设置复选框状态
    rememberPasswordCheck_->setChecked(rememberPassword);
    autoLoginCheck_->setChecked(autoLogin);
    
    if (rememberPassword) {
        // 加载保存的工号
        QString employeeId = settings.value("loginWindow/employeeId").toString();
        if (!employeeId.isEmpty()) {
            employeeIdEdit_->setText(employeeId);
        }
        // 恢复身份（0=员工,1=HR管理员）
        int savedRole = settings.value("loginWindow/role", 0).toInt();
        if (roleGroup_) {
            if (QAbstractButton* b = roleGroup_->button(savedRole)) {
                b->setChecked(true);
            }
        }
        
        // 【安全修复】如果同时选择了自动登录，使用对应的解密方式加载密码
        if (autoLogin) {
            QByteArray passwordData = settings.value("loginWindow/password").toByteArray();
            if (!passwordData.isEmpty()) {
                // 解密密码（与加密时使用相同的密钥）
                QByteArray encryptedData = QByteArray::fromBase64(passwordData);
                QByteArray key = QByteArray::fromHex("4A5F6C8E9B2D3E7A");
                for (int i = 0; i < encryptedData.size(); ++i) {
                    encryptedData[i] = encryptedData[i] ^ key[i % key.size()];
                }
                QString password = QString::fromUtf8(encryptedData);
                if (!password.isEmpty()) {
                    passwordEdit_->setText(password);
                    qDebug() << "[安全] 密码已解密并自动填充";
                    
                    // 自动登录
                    QTimer::singleShot(1000, this, &LoginWindow::onLoginClicked);
                }
            }
        }
    }
}

void LoginWindow::animateLogin()
{
    // 登录动画效果
    loginButton_->setEnabled(false);
    loginButton_->setText("登录中...");
    
    QTimer::singleShot(500, [this]() {
        loginButton_->setEnabled(true);
        loginButton_->setText("登录");
    });
}

void LoginWindow::createGradientBackground()
{
    setStyleSheet(
        "QWidget#loginWidget {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                               stop:0 #667eea, stop:1 #764ba2);"
        "}"
        "QFrame#loginFrame {"
        "    background: #ffffff;"
        "    border-radius: 25px;"
        "    border: 3px solid rgba(255, 255, 255, 0.9);"
        "    box-shadow: 0 15px 35px rgba(0, 0, 0, 0.2);"
        "}"
    );
}

void LoginWindow::setupAnimations()
{
    // 可以添加窗口显示动画等
}
