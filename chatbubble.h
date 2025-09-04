#ifndef CHATBUBBLE_H
#define CHATBUBBLE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTextEdit>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include <QDateTime>
#include <QTimer>
#include <QTextBrowser>

enum class MessageType {
    UserMessage,     // 用户消息
    BotMessage,      // AI回复消息
    SystemMessage,   // 系统消息
    TypingIndicator  // 打字指示器
};

class ChatBubble : public QWidget
{
    Q_OBJECT

public:
    explicit ChatBubble(MessageType type, const QString& message = "", const QPixmap& avatar = QPixmap(), QWidget *parent = nullptr);
    ~ChatBubble();

    void setMessage(const QString& message);
    void setTimestamp(const QDateTime& timestamp);
    void setConfidence(const QString& confidence);
    void setCategory(const QString& category);
    void setAvatar(const QPixmap& avatar);
    void showTypingAnimation();
    void hideTypingAnimation();
    void animateIn();
    void setRelatedClausesVisible(bool visible);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void bubbleClicked();
    void copyRequested(const QString& text);
    void showRelatedClauses(const QString& question);

private slots:
    void onCopyClicked();
    void onAnimationTimer();

private:
    MessageType messageType_;
    QString messageText_;
    QDateTime timestamp_;
    QString confidence_;
    QString category_;
    QPixmap avatarPixmap_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QHBoxLayout* contentLayout_;
    QHBoxLayout* messageWithAvatarLayout_;
    QLabel* avatarLabel_;
    QWidget* messageContainer_;
    QVBoxLayout* messageContainerLayout_;
    QTextBrowser* messageLabel_;
    QLabel* timestampLabel_;
    QLabel* infoLabel_;
    QPushButton* copyButton_;
    QPushButton* relatedButton_;
    QWidget* typingIndicator_;
    QHBoxLayout* typingLayout_;
    
    // 动画相关
    QPropertyAnimation* fadeInAnimation_;
    QPropertyAnimation* slideInAnimation_;
    QGraphicsDropShadowEffect* shadowEffect_;
    QTimer* typingTimer_;
    int typingDots_;
    
    // 样式相关
    QColor bubbleColor_;
    QColor textColor_;
    QColor borderColor_;
    bool isAnimating_;
    
    // 私有方法
    void setupUI();
    void setupAnimations();
    void applyUserStyle();
    void applyBotStyle();
    void applySystemStyle();
    void applyTypingStyle();
    void createTypingIndicator();
    void updateTypingDots();
    void updateInfoLabel();
    QPixmap makeCircular(const QPixmap& pixmap);
    QRect getBubbleRect() const;
    void drawBubble(QPainter& painter, const QRect& rect);
    void drawTail(QPainter& painter, const QRect& rect);
};

#endif // CHATBUBBLE_H
