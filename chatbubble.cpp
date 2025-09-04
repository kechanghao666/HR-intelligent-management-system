#include "chatbubble.h"
#include "avatarwidget.h"
#include <QApplication>
#include <QPainter>
#include <QFontMetrics>
#include <QClipboard>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPainterPath>
#include <QtMath>
#include <QMessageBox>

ChatBubble::ChatBubble(MessageType type, const QString& message, const QPixmap& avatar, QWidget *parent)
    : QWidget(parent)
    , messageType_(type)
    , messageText_(message)
    , timestamp_(QDateTime::currentDateTime())
    , confidence_("")
    , category_("")
    , avatarPixmap_(avatar)
    , mainLayout_(nullptr)
    , contentLayout_(nullptr)
    , messageWithAvatarLayout_(nullptr)
    , avatarLabel_(nullptr)
    , messageContainer_(nullptr)
    , messageContainerLayout_(nullptr)
    , messageLabel_(nullptr)
    , timestampLabel_(nullptr)
    , infoLabel_(nullptr)
    , copyButton_(nullptr)
    , relatedButton_(nullptr)
    , typingIndicator_(nullptr)
    , typingLayout_(nullptr)
    , fadeInAnimation_(nullptr)
    , slideInAnimation_(nullptr)
    , shadowEffect_(nullptr)
    , typingTimer_(nullptr)
    , typingDots_(0)
    , isAnimating_(false)
{
    setupUI();
    setupAnimations();
    
    switch (messageType_) {
        case MessageType::UserMessage:
            applyUserStyle();
            break;
        case MessageType::BotMessage:
            applyBotStyle();
            break;
        case MessageType::SystemMessage:
            applySystemStyle();
            break;
        case MessageType::TypingIndicator:
            applyTypingStyle();
            createTypingIndicator();
            break;
    }
    
    setMessage(message);
}

ChatBubble::~ChatBubble()
{
    if (typingTimer_) {
        typingTimer_->stop();
    }
}

void ChatBubble::setupUI()
{
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(16, 8, 16, 8);
    
    mainLayout_ = new QVBoxLayout(this);
    mainLayout_->setSpacing(4);
    mainLayout_->setContentsMargins(0, 0, 0, 0);
    
    // 创建包含头像和消息的水平布局
    messageWithAvatarLayout_ = new QHBoxLayout();
    messageWithAvatarLayout_->setSpacing(12);
    messageWithAvatarLayout_->setContentsMargins(0, 0, 0, 0);
    
    // 头像标签
    avatarLabel_ = new QLabel();
    avatarLabel_->setFixedSize(36, 36);
    avatarLabel_->setScaledContents(true);
    avatarLabel_->setStyleSheet("border-radius: 18px; background: #f0f0f0;");
    
    // 消息容器（包含消息气泡、时间戳等）
    messageContainer_ = new QWidget();
    messageContainerLayout_ = new QVBoxLayout(messageContainer_);
    messageContainerLayout_->setSpacing(4);
    messageContainerLayout_->setContentsMargins(0, 0, 0, 0);
    
    // 创建消息内容水平布局（包含消息和复制按钮）
    contentLayout_ = new QHBoxLayout();
    contentLayout_->setSpacing(8);
    contentLayout_->setContentsMargins(0, 0, 0, 0);
    
    // 消息文本显示
    messageLabel_ = new QTextBrowser();
    messageLabel_->setObjectName("messageBrowser");
    messageLabel_->setFrameShape(QFrame::NoFrame);
    messageLabel_->setOpenExternalLinks(false);
    messageLabel_->setReadOnly(true);
    messageLabel_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageLabel_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    messageLabel_->setMaximumWidth(480);  // 限制最大宽度，类似微信/ChatGPT
    
    // 复制按钮
    copyButton_ = new QPushButton("复制");
    copyButton_->setObjectName("copyButton");
    copyButton_->setFixedSize(50, 32);  // 增加尺寸以完整显示"复制"文字
    copyButton_->setToolTip("复制消息");
    copyButton_->setVisible(false);
    
    // 相关条款按钮（仅对用户消息显示）
    if (messageType_ == MessageType::UserMessage) {
        relatedButton_ = new QPushButton("相关条款");
        relatedButton_->setObjectName("relatedButton");
        relatedButton_->setFixedSize(70, 32);
        relatedButton_->setToolTip("查看相关条款");
        relatedButton_->setVisible(false);
        connect(relatedButton_, &QPushButton::clicked, [this]() {
            emit showRelatedClauses(messageText_);
        });
    }
    
    // 时间戳标签
    timestampLabel_ = new QLabel();
    timestampLabel_->setObjectName("timestampLabel");
    
    // 信息标签（置信度、类别等）
    infoLabel_ = new QLabel();
    infoLabel_->setObjectName("infoLabel");
    
    connect(copyButton_, &QPushButton::clicked, this, &ChatBubble::onCopyClicked);
    
    // 为相关条款按钮添加样式
    if (relatedButton_) {
        relatedButton_->setStyleSheet(
            "#relatedButton {"
            "    background: rgba(34,197,94,0.08);"
            "    border: 2px solid rgba(34,197,94,0.2);"
            "    border-radius: 12px;"
            "    color: #065F46;"
            "    font-size: 12px;"
            "    font-weight: 600;"
            "    font-family: 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
            "    padding: 4px 6px;"
            "}"
            "#relatedButton:hover {"
            "    background: rgba(34,197,94,0.15);"
            "    border: 2px solid rgba(34,197,94,0.4);"
            "    color: #047857;"
            "}"
            "#relatedButton:pressed {"
            "    background: rgba(34,197,94,0.1);"
            "    color: #065F46;"
            "}"
        );
    }
}

void ChatBubble::setupAnimations()
{
    // 淡入动画
    fadeInAnimation_ = new QPropertyAnimation(this, "windowOpacity");
    fadeInAnimation_->setDuration(300);
    fadeInAnimation_->setStartValue(0.0);
    fadeInAnimation_->setEndValue(1.0);
    fadeInAnimation_->setEasingCurve(QEasingCurve::OutCubic);
    
    // 滑入动画
    slideInAnimation_ = new QPropertyAnimation(this, "pos");
    slideInAnimation_->setDuration(400);
    slideInAnimation_->setEasingCurve(QEasingCurve::OutBack);
    
    // 阴影效果
    shadowEffect_ = new QGraphicsDropShadowEffect();
    shadowEffect_->setBlurRadius(10);
    shadowEffect_->setOffset(0, 2);
    shadowEffect_->setColor(QColor(0, 0, 0, 50));
    setGraphicsEffect(shadowEffect_);
    
    // 打字动画计时器
    typingTimer_ = new QTimer(this);
    typingTimer_->setInterval(500);
    connect(typingTimer_, &QTimer::timeout, this, &ChatBubble::onAnimationTimer);
}

void ChatBubble::applyUserStyle()
{
    bubbleColor_ = QColor("#007AFF");  // iOS蓝色
    textColor_ = Qt::white;
    borderColor_ = QColor("#005BBB");
    
    // 设置头像（如果有的话）
    if (!avatarPixmap_.isNull()) {
        QPixmap circularAvatar = makeCircular(avatarPixmap_.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        avatarLabel_->setPixmap(circularAvatar);
    } else {
        avatarLabel_->setStyleSheet("border-radius: 18px; background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #007AFF, stop:1 #0056CC);");
    }
    
    // 消息样式（现代化设计）
    messageLabel_->setStyleSheet(
        "#messageBrowser {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #007AFF, stop:1 #0056CC);"
        "    border: none;"
        "    border-radius: 18px;"
        "    color: white;"
        "    font-size: 15px;"
        "    font-family: 'SF Pro Text', 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "    font-weight: 400;"
        "    line-height: 1.4;"
        "    padding: 12px 16px;"
        "    selection-background-color: rgba(255,255,255,0.3);"
        "    selection-color: white;"
        "}"
    );
    
    copyButton_->setStyleSheet(
        "#copyButton {"
        "    background: rgba(59,130,246,0.1);"
        "    border: 2px solid rgba(59,130,246,0.3);"
        "    border-radius: 12px;"
        "    color: #1E40AF;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    font-family: 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "    padding: 4px 6px;"
        "}"
        "#copyButton:hover {"
        "    background: rgba(59,130,246,0.2);"
        "    border: 2px solid rgba(59,130,246,0.5);"
        "    color: #1D4ED8;"
        "}"
        "#copyButton:pressed {"
        "    background: rgba(59,130,246,0.15);"
        "    color: #1E3A8A;"
        "}"
    );
    
    timestampLabel_->setStyleSheet(
        "#timestampLabel {"
        "    color: rgba(0,0,0,0.4);"
        "    font-size: 11px;"
        "    font-family: 'SF Pro Text', 'Microsoft YaHei', sans-serif;"
        "    padding: 2px 0px;"
        "}"
    );
    
    // 右对齐布局（用户消息）
    messageWithAvatarLayout_->addStretch();
    messageWithAvatarLayout_->addWidget(messageContainer_);
    messageWithAvatarLayout_->addWidget(avatarLabel_);
    
    // 消息容器内容布局
    contentLayout_->addWidget(messageLabel_);
    if (relatedButton_) {
        contentLayout_->addWidget(relatedButton_);
    }
    contentLayout_->addWidget(copyButton_);
    messageContainerLayout_->addLayout(contentLayout_);
    messageContainerLayout_->addWidget(timestampLabel_);
    timestampLabel_->setAlignment(Qt::AlignRight);
    
    mainLayout_->addLayout(messageWithAvatarLayout_);
}

void ChatBubble::applyBotStyle()
{
    bubbleColor_ = QColor("#F8F9FA");  // 现代化浅灰色
    textColor_ = QColor("#1F2937");    // 深灰色文字
    borderColor_ = QColor("#E5E7EB");
    
    // 设置AI助手头像
    if (!avatarPixmap_.isNull()) {
        QPixmap circularAvatar = makeCircular(avatarPixmap_.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        avatarLabel_->setPixmap(circularAvatar);
    } else {
        // 使用默认的AI助手头像
        AvatarManager& manager = AvatarManager::instance();
        QPixmap assistantAvatar = manager.createAssistantAvatar();
        QPixmap circularAvatar = makeCircular(assistantAvatar.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        avatarLabel_->setPixmap(circularAvatar);
    }
    
    // AI消息样式（现代化设计）
    messageLabel_->setStyleSheet(
        "#messageBrowser {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(248,249,250,1), "
        "                stop:1 rgba(243,244,246,0.95));"
        "    border: 1px solid rgba(229,231,235,0.8);"
        "    border-radius: 18px;"
        "    color: #1F2937;"
        "    font-size: 15px;"
        "    font-family: 'SF Pro Text', 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "    font-weight: 400;"
        "    line-height: 1.5;"
        "    padding: 12px 16px;"
        "    selection-background-color: rgba(59,130,246,0.2);"
        "    selection-color: #1F2937;"
        "}"
    );
    
    copyButton_->setStyleSheet(
        "#copyButton {"
        "    background: rgba(59,130,246,0.08);"
        "    border: 2px solid rgba(59,130,246,0.2);"
        "    border-radius: 12px;"
        "    color: #374151;"
        "    font-size: 12px;"
        "    font-weight: 600;"
        "    font-family: 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "    padding: 4px 6px;"
        "}"
        "#copyButton:hover {"
        "    background: rgba(59,130,246,0.15);"
        "    border: 2px solid rgba(59,130,246,0.4);"
        "    color: #1E40AF;"
        "}"
        "#copyButton:pressed {"
        "    background: rgba(59,130,246,0.1);"
        "    color: #1D4ED8;"
        "}"
    );
    
    timestampLabel_->setStyleSheet(
        "#timestampLabel {"
        "    color: rgba(0,0,0,0.4);"
        "    font-size: 11px;"
        "    font-family: 'SF Pro Text', 'Microsoft YaHei', sans-serif;"
        "    padding: 2px 0px;"
        "}"
    );
    
    infoLabel_->setStyleSheet(
        "#infoLabel {"
        "    color: #3B82F6;"
        "    font-size: 10px;"
        "    font-weight: 600;"
        "    font-family: 'SF Pro Text', 'Microsoft YaHei', sans-serif;"
        "    padding: 4px 8px;"
        "    background: rgba(59, 130, 246, 0.1);"
        "    border: 1px solid rgba(59, 130, 246, 0.2);"
        "    border-radius: 10px;"
        "    margin: 2px 0px;"
        "}"
    );
    
    // 左对齐布局（AI消息）
    messageWithAvatarLayout_->addWidget(avatarLabel_);
    messageWithAvatarLayout_->addWidget(messageContainer_);
    messageWithAvatarLayout_->addStretch();
    
    // 消息容器内容布局
    contentLayout_->addWidget(copyButton_);
    contentLayout_->addWidget(messageLabel_);
    messageContainerLayout_->addLayout(contentLayout_);
    
    // 信息和时间戳布局
    QHBoxLayout* infoLayout = new QHBoxLayout();
    infoLayout->setSpacing(8);
    infoLayout->setContentsMargins(0, 0, 0, 0);
    infoLayout->addWidget(infoLabel_);
    infoLayout->addStretch();
    infoLayout->addWidget(timestampLabel_);
    messageContainerLayout_->addLayout(infoLayout);
    
    mainLayout_->addLayout(messageWithAvatarLayout_);
}

void ChatBubble::applySystemStyle()
{
    bubbleColor_ = QColor("#FFF3CD");  // 更柔和的黄色
    textColor_ = QColor("#856404");    // 深色文字
    borderColor_ = QColor("#FFEAA7");
    
    messageLabel_->setStyleSheet(
        "#messageBrowser {"
        "    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, "
        "                stop:0 rgba(255,243,205,0.8), "
        "                stop:1 rgba(255,235,167,0.6));"
        "    border: 1px solid rgba(255,193,7,0.3);"
        "    border-radius: 15px;"
        "    color: #856404;"
        "    font-size: 14px;"
        "    font-family: 'SF Pro Text', 'Microsoft YaHei', 'Segoe UI', Arial, sans-serif;"
        "    font-weight: 400;"
        "    line-height: 1.4;"
        "    padding: 10px 16px;"
        "    max-width: 400px;"
        "    text-align: center;"
        "}"
    );
    
    // 居中对齐（系统消息）
    contentLayout_->addStretch();
    contentLayout_->addWidget(messageLabel_);
    contentLayout_->addStretch();
    
    mainLayout_->addLayout(contentLayout_);
}

void ChatBubble::applyTypingStyle()
{
    bubbleColor_ = QColor("#F2F2F7");
    textColor_ = Qt::gray;
    borderColor_ = QColor("#E5E5EA");
}

void ChatBubble::createTypingIndicator()
{
    typingIndicator_ = new QWidget();
    typingLayout_ = new QHBoxLayout(typingIndicator_);
    typingLayout_->setSpacing(4);
    typingLayout_->setContentsMargins(16, 12, 16, 12);
    
    // 创建三个点
    for (int i = 0; i < 3; ++i) {
        QLabel* dot = new QLabel("•");
        dot->setStyleSheet("color: #8E8E93; font-size: 20px; font-weight: bold;");
        dot->setAlignment(Qt::AlignCenter);
        dot->setFixedSize(10, 20);
        typingLayout_->addWidget(dot);
    }
    
    contentLayout_->addWidget(copyButton_);
    contentLayout_->addWidget(typingIndicator_);
    contentLayout_->addStretch();
    
    mainLayout_->addLayout(contentLayout_);
    mainLayout_->addWidget(timestampLabel_);
}

void ChatBubble::setMessage(const QString& message)
{
    messageText_ = message;
    if (messageLabel_) {
        messageLabel_->setHtml(messageText_);
        
        // 自动调整高度
        QTextDocument* doc = messageLabel_->document();
        doc->setTextWidth(messageLabel_->width());
        int height = doc->size().height() + 20;
        messageLabel_->setFixedHeight(qMax(40, qMin(height, 200)));
    }
}

void ChatBubble::setTimestamp(const QDateTime& timestamp)
{
    timestamp_ = timestamp;
    if (timestampLabel_) {
        timestampLabel_->setText(timestamp_.toString("hh:mm"));
    }
}

void ChatBubble::setConfidence(const QString& confidence)
{
    confidence_ = confidence;
    updateInfoLabel();
}

void ChatBubble::setCategory(const QString& category)
{
    category_ = category;
    updateInfoLabel();
}

void ChatBubble::setAvatar(const QPixmap& avatar)
{
    avatarPixmap_ = avatar;
    if (avatarLabel_ && !avatar.isNull()) {
        QPixmap circularAvatar = makeCircular(avatar.scaled(36, 36, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        avatarLabel_->setPixmap(circularAvatar);
    }
}

QPixmap ChatBubble::makeCircular(const QPixmap& pixmap)
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

void ChatBubble::updateInfoLabel()
{
    if (!infoLabel_ || messageType_ != MessageType::BotMessage) return;
    
    QStringList info;
    if (!confidence_.isEmpty()) {
        info << QString("置信度: %1").arg(confidence_);
    }
    if (!category_.isEmpty()) {
        info << QString("类别: %1").arg(category_);
    }
    
    infoLabel_->setText(info.join(" | "));
    infoLabel_->setVisible(!info.isEmpty());
}

void ChatBubble::showTypingAnimation()
{
    if (messageType_ == MessageType::TypingIndicator && typingTimer_) {
        typingTimer_->start();
    }
}

void ChatBubble::hideTypingAnimation()
{
    if (typingTimer_) {
        typingTimer_->stop();
    }
}

void ChatBubble::animateIn()
{
    if (isAnimating_) return;
    
    isAnimating_ = true;
    setWindowOpacity(0.0);
    
    // 计算滑入起始位置
    QPoint startPos = pos();
    if (messageType_ == MessageType::UserMessage) {
        startPos.setX(startPos.x() + 50);  // 从右侧滑入
    } else {
        startPos.setX(startPos.x() - 50);  // 从左侧滑入
    }
    
    slideInAnimation_->setStartValue(startPos);
    slideInAnimation_->setEndValue(pos());
    
    fadeInAnimation_->start();
    slideInAnimation_->start();
    
    connect(fadeInAnimation_, &QPropertyAnimation::finished, [this]() {
        isAnimating_ = false;
    });
}

void ChatBubble::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    
    QRect bubbleRect = getBubbleRect();
    if (!bubbleRect.isValid()) return;
    
    drawBubble(painter, bubbleRect);
    
    if (messageType_ != MessageType::SystemMessage) {
        drawTail(painter, bubbleRect);
    }
    
    QWidget::paintEvent(event);
}

void ChatBubble::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit bubbleClicked();
    }
    QWidget::mousePressEvent(event);
}

QRect ChatBubble::getBubbleRect() const
{
    if (messageType_ == MessageType::TypingIndicator && typingIndicator_) {
        return typingIndicator_->geometry();
    } else if (messageLabel_) {
        return messageLabel_->geometry();
    }
    return rect().adjusted(20, 10, -20, -30);
}

void ChatBubble::drawBubble(QPainter& painter, const QRect& rect)
{
    // 创建圆角矩形路径
    QPainterPath path;
    int radius = 18;
    path.addRoundedRect(rect, radius, radius);
    
    // 绘制阴影
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(QColor(0, 0, 0, 20)));
    painter.drawPath(path.translated(0, 1));
    
    // 绘制主体
    QLinearGradient gradient(rect.topLeft(), rect.bottomLeft());
    gradient.setColorAt(0, bubbleColor_.lighter(105));
    gradient.setColorAt(1, bubbleColor_);
    
    painter.setBrush(QBrush(gradient));
    painter.setPen(QPen(borderColor_, 1));
    painter.drawPath(path);
}

void ChatBubble::drawTail(QPainter& painter, const QRect& rect)
{
    QPainterPath tailPath;
    QPointF tailTip;
    QPointF tailBase1, tailBase2;
    
    if (messageType_ == MessageType::UserMessage) {
        // 右侧小尾巴
        tailTip = QPointF(rect.right() + 8, rect.bottom() - 10);
        tailBase1 = QPointF(rect.right(), rect.bottom() - 5);
        tailBase2 = QPointF(rect.right(), rect.bottom() - 15);
    } else {
        // 左侧小尾巴
        tailTip = QPointF(rect.left() - 8, rect.bottom() - 10);
        tailBase1 = QPointF(rect.left(), rect.bottom() - 5);
        tailBase2 = QPointF(rect.left(), rect.bottom() - 15);
    }
    
    tailPath.moveTo(tailBase1);
    tailPath.lineTo(tailTip);
    tailPath.lineTo(tailBase2);
    tailPath.closeSubpath();
    
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(bubbleColor_));
    painter.drawPath(tailPath);
}

void ChatBubble::onCopyClicked()
{
    if (!messageText_.isEmpty()) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(messageText_);
        emit copyRequested(messageText_);
        
        // 简单的视觉反馈
        copyButton_->setText("✓");
        QTimer::singleShot(1000, [this]() {
            copyButton_->setText("📋");
        });
    }
}

void ChatBubble::onAnimationTimer()
{
    if (messageType_ != MessageType::TypingIndicator || !typingIndicator_) return;
    
    typingDots_ = (typingDots_ + 1) % 4;
    updateTypingDots();
}

void ChatBubble::updateTypingDots()
{
    if (!typingLayout_) return;
    
    for (int i = 0; i < 3; ++i) {
        QLabel* dot = qobject_cast<QLabel*>(typingLayout_->itemAt(i)->widget());
        if (dot) {
            if (i < typingDots_) {
                dot->setStyleSheet("color: #007AFF; font-size: 20px; font-weight: bold;");
            } else {
                dot->setStyleSheet("color: #8E8E93; font-size: 20px; font-weight: bold;");
            }
        }
    }
}

void ChatBubble::enterEvent(QEnterEvent* event)
{
    Q_UNUSED(event)
    if (messageType_ != MessageType::SystemMessage && messageType_ != MessageType::TypingIndicator) {
        copyButton_->setVisible(true);
        // 只有在相关条款按钮被允许显示且是用户消息时才显示
        // 这样可以确保表单模式下不显示相关条款按钮
        if (relatedButton_ && messageType_ == MessageType::UserMessage && !relatedButton_->property("forceHidden").toBool()) {
            relatedButton_->setVisible(true);
        }
    }
}



void ChatBubble::leaveEvent(QEvent* event)
{
    Q_UNUSED(event)
    copyButton_->setVisible(false);
    if (relatedButton_) {
        relatedButton_->setVisible(false);
    }
}

void ChatBubble::setRelatedClausesVisible(bool visible)
{
    if (relatedButton_) {
        // 使用属性标记来控制是否强制隐藏，这样enterEvent也会遵守这个设置
        relatedButton_->setProperty("forceHidden", !visible);
        relatedButton_->setVisible(visible);
    }
}