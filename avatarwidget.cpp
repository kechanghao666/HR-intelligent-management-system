#include "avatarwidget.h"
#include <QApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QImageReader>
#include <QPainterPath>
#include <QtMath>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFile>
#include <QIcon>

// ============================================================================
// AvatarWidget Implementation
// ============================================================================

AvatarWidget::AvatarWidget(const User& user, QWidget *parent)
    : QWidget(parent)
    , user_(user)
    , avatarSize_(48)
    , isHovered_(false)
    , hoverAnimation_(nullptr)
    , shadowEffect_(nullptr)
{
    setFixedSize(avatarSize_, avatarSize_);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::PointingHandCursor);
    
    setupAnimations();
    loadUserAvatar();
}

AvatarWidget::~AvatarWidget()
{
}

void AvatarWidget::setupAnimations()
{
    // 悬停动画
    hoverAnimation_ = new QPropertyAnimation(this, "geometry");
    hoverAnimation_->setDuration(200);
    hoverAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    // 阴影效果
    shadowEffect_ = new QGraphicsDropShadowEffect();
    shadowEffect_->setBlurRadius(8);
    shadowEffect_->setOffset(0, 2);
    shadowEffect_->setColor(QColor(0, 0, 0, 100));
    setGraphicsEffect(shadowEffect_);
}

void AvatarWidget::loadUserAvatar()
{
    AvatarManager& manager = AvatarManager::instance();
    
    if (manager.hasUserAvatar(user_.getEmployeeId())) {
        currentAvatar_ = manager.loadUserAvatar(user_.getEmployeeId());
    } else {
        currentAvatar_ = manager.createPersonalizedAvatar(user_.getName());
    }
    
    currentAvatar_ = makeCircular(currentAvatar_.scaled(avatarSize_, avatarSize_, 
                                                       Qt::KeepAspectRatioByExpanding, 
                                                       Qt::SmoothTransformation));
}

QPixmap AvatarWidget::createDefaultAvatar()
{
    QPixmap pixmap(avatarSize_, avatarSize_);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 创建渐变黄色背景
    QLinearGradient gradient(0, 0, avatarSize_, avatarSize_);
    gradient.setColorAt(0, QColor("#ffeaa7"));
    gradient.setColorAt(1, QColor("#fab1a0"));
    
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, avatarSize_, avatarSize_);
    
    // 绘制用户名首字母
    QString initials = user_.getName().left(1);
    painter.setPen(Qt::white);
    painter.setFont(QFont("Microsoft YaHei", avatarSize_ / 3, QFont::Bold));
    painter.drawText(QRect(0, 0, avatarSize_, avatarSize_), Qt::AlignCenter, initials);
    
    return pixmap;
}

QPixmap AvatarWidget::makeCircular(const QPixmap& pixmap)
{
    QPixmap result(pixmap.size());
    result.fill(Qt::transparent);
    
    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    QPainterPath path;
    path.addEllipse(result.rect());
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, pixmap);
    
    return result;
}

void AvatarWidget::setAvatarPixmap(const QPixmap& pixmap)
{
    currentAvatar_ = makeCircular(pixmap.scaled(avatarSize_, avatarSize_, 
                                               Qt::KeepAspectRatioByExpanding, 
                                               Qt::SmoothTransformation));
    update();
    
    // 保存头像
    AvatarManager::instance().saveUserAvatar(user_.getEmployeeId(), pixmap);
    emit avatarChanged(currentAvatar_);
}

QPixmap AvatarWidget::getAvatarPixmap() const
{
    return currentAvatar_;
}

void AvatarWidget::setSize(int size)
{
    avatarSize_ = size;
    setFixedSize(size, size);
    loadUserAvatar();
}

void AvatarWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    if (!currentAvatar_.isNull()) {
        painter.drawPixmap(0, 0, currentAvatar_);
    } else {
        currentAvatar_ = createDefaultAvatar();
        painter.drawPixmap(0, 0, currentAvatar_);
    }
    
    // 悬停效果
    if (isHovered_) {
        painter.setPen(QPen(QColor("#007AFF"), 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(1, 1, width() - 2, height() - 2);
    }
}

void AvatarWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit avatarClicked();
    }
    QWidget::mousePressEvent(event);
}

void AvatarWidget::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    isHovered_ = true;
    shadowEffect_->setBlurRadius(12);
    shadowEffect_->setOffset(0, 3);
    update();
}

void AvatarWidget::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    isHovered_ = false;
    shadowEffect_->setBlurRadius(8);
    shadowEffect_->setOffset(0, 2);
    update();
}

void AvatarWidget::onHoverAnimation()
{
    // 悬停动画实现
}

// ============================================================================
// AvatarSelectionDialog Implementation  
// ============================================================================

AvatarSelectionDialog::AvatarSelectionDialog(const User& user, QWidget *parent)
    : QDialog(parent)
    , user_(user)
    , mainLayout_(nullptr)
    , titleLabel_(nullptr)
    , currentAvatarLabel_(nullptr)
    , defaultAvatarsGroup_(nullptr)
    , defaultAvatarsLayout_(nullptr)
    , buttonLayout_(nullptr)
    , uploadButton_(nullptr)
    , confirmButton_(nullptr)
    , cancelButton_(nullptr)
{
    setWindowTitle("选择头像");
    setModal(true);
    setMinimumSize(650, 700);  // 增加对话框大小以适应更大的头像按钮
    
    setupUI();
    setupConnections();
    applyStyles();
    loadDefaultAvatars();
}

AvatarSelectionDialog::~AvatarSelectionDialog()
{
}

void AvatarSelectionDialog::setupUI()
{
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setSpacing(20);
    mainLayout_->setContentsMargins(30, 30, 30, 30);
    
    // 标题
    titleLabel_ = new QLabel("选择您的头像");
    titleLabel_->setAlignment(Qt::AlignCenter);
    titleLabel_->setStyleSheet("font-size: 18px; font-weight: bold; color: #1a1a1a;");
    mainLayout_->addWidget(titleLabel_);
    
    // 当前头像显示
    currentAvatarLabel_ = new QLabel();
    currentAvatarLabel_->setAlignment(Qt::AlignCenter);
    currentAvatarLabel_->setFixedSize(120, 120);
    currentAvatarLabel_->setStyleSheet(
        "border: 2px solid #e0e0e0; border-radius: 60px; background: white;"
    );
    
    // 加载当前头像
    AvatarManager& manager = AvatarManager::instance();
    QPixmap currentAvatar;
    if (manager.hasUserAvatar(user_.getEmployeeId())) {
        currentAvatar = manager.loadUserAvatar(user_.getEmployeeId());
    } else {
        currentAvatar = manager.createPersonalizedAvatar(user_.getName());
    }
    
    QPixmap scaledAvatar = currentAvatar.scaled(116, 116, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap circularAvatar(116, 116);
    circularAvatar.fill(Qt::transparent);
    
    QPainter painter(&circularAvatar);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addEllipse(0, 0, 116, 116);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaledAvatar);
    
    currentAvatarLabel_->setPixmap(circularAvatar);
    selectedAvatar_ = currentAvatar;
    
    mainLayout_->addWidget(currentAvatarLabel_, 0, Qt::AlignCenter);
    
    // 默认头像组
    defaultAvatarsGroup_ = new QGroupBox("选择默认头像");
    defaultAvatarsLayout_ = new QGridLayout(defaultAvatarsGroup_);
    defaultAvatarsLayout_->setSpacing(20);  // 增加间距以适应更大的头像按钮
    mainLayout_->addWidget(defaultAvatarsGroup_);
    
    // 按钮区域
    buttonLayout_ = new QHBoxLayout();
    
    uploadButton_ = new QPushButton("上传自定义头像");
    uploadButton_->setMinimumHeight(40);
    
    confirmButton_ = new QPushButton("确认");
    confirmButton_->setMinimumHeight(40);
    confirmButton_->setMinimumWidth(80);
    
    cancelButton_ = new QPushButton("取消");
    cancelButton_->setMinimumHeight(40);
    cancelButton_->setMinimumWidth(80);
    
    buttonLayout_->addWidget(uploadButton_);
    buttonLayout_->addStretch();
    buttonLayout_->addWidget(confirmButton_);
    buttonLayout_->addWidget(cancelButton_);
    
    mainLayout_->addLayout(buttonLayout_);
}

void AvatarSelectionDialog::setupConnections()
{
    connect(uploadButton_, &QPushButton::clicked, this, &AvatarSelectionDialog::onUploadAvatarClicked);
    connect(confirmButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
}

void AvatarSelectionDialog::loadDefaultAvatars()
{
    AvatarManager& manager = AvatarManager::instance();
    QList<QPixmap> defaultAvatars = manager.getDefaultAvatars();
    
    int row = 0, col = 0;
    const int maxCols = 5;
    
    for (int i = 0; i < defaultAvatars.size(); ++i) {
        QPushButton* button = createAvatarButton(defaultAvatars[i], i);
        defaultAvatarButtons_.append(button);
        defaultAvatarsLayout_->addWidget(button, row, col);
        
        connect(button, &QPushButton::clicked, this, &AvatarSelectionDialog::onAvatarButtonClicked);
        
        col++;
        if (col >= maxCols) {
            col = 0;
            row++;
        }
    }
}

QPushButton* AvatarSelectionDialog::createAvatarButton(const QPixmap& pixmap, int index)
{
    QPushButton* button = new QPushButton();
    button->setFixedSize(100, 100);  // 增加按钮大小以确保头像完全显示
    button->setProperty("avatarIndex", index);
    
    QPixmap scaledPixmap = pixmap.scaled(94, 94, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap circularPixmap(94, 94);
    circularPixmap.fill(Qt::transparent);
    
    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addEllipse(0, 0, 94, 94);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaledPixmap);
    
    button->setIcon(QIcon(circularPixmap));
    button->setIconSize(QSize(94, 94));  // 对应增加图标大小
    button->setStyleSheet(
        "QPushButton {"
        "    border: 2px solid transparent;"
        "    border-radius: 50px;"  // 增加圆角半径
        "    background: transparent;"
        "}"
        "QPushButton:hover {"
        "    border: 2px solid #007AFF;"
        "    background: rgba(0, 122, 255, 0.1);"
        "}"
        "QPushButton:pressed {"
        "    border: 2px solid #0056CC;"
        "    background: rgba(0, 122, 255, 0.2);"
        "}"
    );
    
    return button;
}

void AvatarSelectionDialog::onAvatarButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    int index = button->property("avatarIndex").toInt();
    AvatarManager& manager = AvatarManager::instance();
    QPixmap avatar = manager.getDefaultAvatar(index);
    
    selectAvatar(avatar);
}

void AvatarSelectionDialog::onUploadAvatarClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择头像图片", "", 
        "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    
    if (!fileName.isEmpty()) {
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            selectAvatar(pixmap);
        } else {
            QMessageBox::warning(this, "错误", "无法加载选择的图片文件，请选择有效的图片。");
        }
    }
}

void AvatarSelectionDialog::onDefaultAvatarClicked()
{
    // 默认头像点击处理
}

void AvatarSelectionDialog::selectAvatar(const QPixmap& avatar)
{
    selectedAvatar_ = avatar;
    
    // 更新当前头像显示
    QPixmap scaledAvatar = avatar.scaled(116, 116, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    QPixmap circularAvatar(116, 116);
    circularAvatar.fill(Qt::transparent);
    
    QPainter painter(&circularAvatar);
    painter.setRenderHint(QPainter::Antialiasing, true);
    QPainterPath path;
    path.addEllipse(0, 0, 116, 116);
    painter.setClipPath(path);
    painter.drawPixmap(0, 0, scaledAvatar);
    
    currentAvatarLabel_->setPixmap(circularAvatar);
    
    emit avatarSelected(avatar);
}

QPixmap AvatarSelectionDialog::getSelectedAvatar() const
{
    return selectedAvatar_;
}

void AvatarSelectionDialog::applyStyles()
{
    setStyleSheet(
        "QDialog {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 #ffffff, stop:1 #f8f9fa);"
        "    border-radius: 12px;"
        "}"
        "QGroupBox {"
        "    font-weight: bold;"
        "    border: 2px solid rgba(0,0,0,0.1);"
        "    border-radius: 8px;"
        "    margin-top: 10px;"
        "    padding-top: 15px;"
        "    background: rgba(255,255,255,0.8);"
        "}"
        "QGroupBox::title {"
        "    subcontrol-origin: margin;"
        "    left: 10px;"
        "    padding: 4px 8px;"
        "    color: #1a1a1a;"
        "    font-size: 14px;"
        "}"
        "QPushButton {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #007AFF, stop:1 #0056CC);"
        "    border: none;"
        "    border-radius: 8px;"
        "    color: white;"
        "    font-weight: bold;"
        "    font-size: 14px;"
        "    padding: 10px 20px;"
        "}"
        "QPushButton:hover {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #0056CC, stop:1 #003D99);"
        "}"
        "QPushButton:pressed {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1, "
        "                stop:0 #003D99, stop:1 #002966);"
        "}"
    );
}

// ============================================================================
// AvatarManager Implementation
// ============================================================================

AvatarManager& AvatarManager::instance()
{
    static AvatarManager instance;
    return instance;
}

AvatarManager::AvatarManager()
{
    avatarDirectory_ = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/avatars";
    ensureAvatarDirectory();
    createDefaultAvatars();
}

AvatarManager::~AvatarManager()
{
}

void AvatarManager::ensureAvatarDirectory()
{
    QDir dir;
    if (!dir.exists(avatarDirectory_)) {
        dir.mkpath(avatarDirectory_);
    }
}

void AvatarManager::createDefaultAvatars()
{
    defaultAvatars_.clear();
    
    // 创建一些渐变色默认头像
    QStringList colors = {
        "#667eea,#764ba2", "#f093fb,#f5576c", "#4facfe,#00f2fe",
        "#43e97b,#38f9d7", "#ffecd2,#fcb69f", "#a8edea,#fed6e3",
        "#d299c2,#fef9d7", "#89f7fe,#66a6ff", "#fbd3e9,#bb377d",
        "#fef9d7,#d299c2", "#96fbc4,#f9f586", "#fce38a,#f38181"
    };
    
    QStringList initials = {
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"
    };
    
    for (int i = 0; i < colors.size() && i < initials.size(); ++i) {
        QStringList colorPair = colors[i].split(",");
        if (colorPair.size() == 2) {
            QPixmap avatar = generateGradientAvatar(initials[i], 
                                                   QColor(colorPair[0]), 
                                                   QColor(colorPair[1]));
            defaultAvatars_.append(avatar);
        }
    }
}

QPixmap AvatarManager::generateGradientAvatar(const QString& text, const QColor& color1, const QColor& color2)
{
    int size = 120;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 创建渐变背景
    QLinearGradient gradient(0, 0, size, size);
    gradient.setColorAt(0, color1);
    gradient.setColorAt(1, color2);
    
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);
    
    // 绘制文字
    painter.setPen(Qt::white);
    
    // 统一使用单字符字体大小
    int fontSize = size / 3;
    painter.setFont(QFont("Microsoft YaHei", fontSize, QFont::Bold));
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, text);
    
    return pixmap;
}

void AvatarManager::saveUserAvatar(const QString& employeeId, const QPixmap& avatar)
{
    QString filePath = getAvatarPath(employeeId);
    avatar.save(filePath, "PNG");
}

QPixmap AvatarManager::loadUserAvatar(const QString& employeeId)
{
    QString filePath = getAvatarPath(employeeId);
    QPixmap avatar(filePath);
    
    if (avatar.isNull()) {
        return createPersonalizedAvatar(employeeId);
    }
    
    return avatar;
}

bool AvatarManager::hasUserAvatar(const QString& employeeId) const
{
    QString filePath = getAvatarPath(employeeId);
    return QFile::exists(filePath);
}

void AvatarManager::removeUserAvatar(const QString& employeeId)
{
    QString filePath = getAvatarPath(employeeId);
    QFile::remove(filePath);
}

QList<QPixmap> AvatarManager::getDefaultAvatars()
{
    return defaultAvatars_;
}

QPixmap AvatarManager::getDefaultAvatar(int index)
{
    if (index >= 0 && index < defaultAvatars_.size()) {
        return defaultAvatars_[index];
    }
    
    return defaultAvatars_.isEmpty() ? QPixmap() : defaultAvatars_.first();
}

QPixmap AvatarManager::createPersonalizedAvatar(const QString& name)
{
    // 根据姓名创建个性化头像 - 统一使用渐变黄色系
    QStringList colors = {
        "#ffeaa7,#fab1a0", "#ffb347,#ffa726", "#ffd54f,#ffcc02",
        "#fff350,#ffeb3b", "#ffe082,#ffc107", "#ffd740,#ff9800",
        "#ffcc80,#ff8a65", "#ffab91,#ff7043", "#ffc947,#ff9800",
        "#ffeaa7,#fab1a0", "#ffb347,#ffa726", "#ffd54f,#ffcc02"
    };
    
    // 统一只取第一个字符作为头像
    QString displayText;
    if (name.isEmpty()) {
        displayText = "?";
    } else {
        displayText = name.left(1);  // 统一只取第一个字符
    }
    
    // 根据名字哈希值选择颜色
    int colorIndex = qHash(name) % colors.size();
    
    QStringList colorPair = colors[colorIndex].split(",");
    if (colorPair.size() == 2) {
        return generateGradientAvatar(displayText, QColor(colorPair[0]), QColor(colorPair[1]));
    }
    
    return generateGradientAvatar(displayText, QColor("#ffeaa7"), QColor("#fab1a0"));
}

QPixmap AvatarManager::createAssistantAvatar()
{
    // 创建HR助手专属头像
    int size = 120;
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    // 创建现代化渐变背景
    QLinearGradient gradient(0, 0, size, size);
    gradient.setColorAt(0, QColor("#667eea"));
    gradient.setColorAt(0.3, QColor("#764ba2"));
    gradient.setColorAt(0.7, QColor("#667eea"));
    gradient.setColorAt(1, QColor("#5a67d8"));
    
    painter.setBrush(gradient);
    painter.setPen(Qt::NoPen);
    painter.drawEllipse(0, 0, size, size);
    
    // 绘制HR标志
    painter.setPen(QPen(Qt::white, 3));
    painter.setFont(QFont("Microsoft YaHei", size / 4, QFont::Bold));
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, "HR");
    
    // 添加装饰元素
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawEllipse(size/6, size/6, size*2/3, size*2/3);
    
    return pixmap;
}

QString AvatarManager::getAvatarPath(const QString& employeeId) const
{
    return avatarDirectory_ + "/" + employeeId + "_avatar.png";
}

QString AvatarManager::getDefaultAvatarPath(int index) const
{
    return avatarDirectory_ + "/default_" + QString::number(index) + ".png";
}
