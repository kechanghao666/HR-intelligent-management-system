#ifndef MODERNQAWIDGET_H
#define MODERNQAWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QEasingCurve>
#include <QResizeEvent>
#include <QScrollBar>
#include <QComboBox>
#include <QStackedWidget>
#include <QListWidget>
#include <QGroupBox>
#include <QLineEdit>
#include "chatbubble.h"
#include "avatarwidget.h"
#include "qaengine.h"
#include "database.h"
#include "user.h"
#include "settingsdialog.h"
#include "documentprocessor.h"
#include "relatedclausesdialog.h"

class ModernQAWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ModernQAWidget(QAEngine* qaEngine, Database* database, const User& currentUser, QWidget *parent = nullptr);
    ~ModernQAWidget();

public slots:
    void askQuestion();
    void showQuickQuestions();
    void showHistory();
    void clearHistory();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

private slots:
    void onSendClicked();
    void onInputTextChanged();
    void onQuickQuestionClicked(QListWidgetItem* item);
    void onHistoryItemClicked(QListWidgetItem* item);
    void onSidebarToggled();
    void onTypingAnimationFinished();
    void simulateTyping();
    void finishTyping();
    void onAvatarChangeClicked();
    void onSettingsClicked();
    void onShowRelatedClauses(const QString& question);
    
    // AI相关槽函数
    void onAIAnswerReceived(const QString& question, const QAResult& result);
    
    // 简化历史记录槽函数
    void onAdvancedSearchTextChanged();
    void onFilterHistoryByDate();


signals:
    void questionAsked(const QString& question, const QAResult& result);

private:
    QAEngine* qaEngine_;
    Database* database_;
    User currentUser_;
    DocumentProcessor* documentProcessor_;
    
    // 主布局组件
    QHBoxLayout* mainLayout_;
    QStackedWidget* stackedWidget_;
    
    // 聊天界面
    QWidget* chatWidget_;
    QVBoxLayout* chatLayout_;
    
    // 顶部标题栏
    QFrame* titleBar_;
    QHBoxLayout* titleLayout_;
    QLabel* titleLabel_;
    QLabel* statusLabel_;
    QPushButton* sidebarButton_;
    QPushButton* settingsButton_;
    
    // 消息区域
    QScrollArea* messagesScrollArea_;
    QWidget* messagesContainer_;
    QVBoxLayout* messagesLayout_;
    QList<ChatBubble*> messageBubbles_;
    
    // 输入区域
    QFrame* inputFrame_;
    QHBoxLayout* inputLayout_;
    QTextEdit* inputEdit_;
    QPushButton* sendButton_;
    QLabel* characterCountLabel_;
    
    // 侧边栏
    QWidget* sidebarWidget_;
    QVBoxLayout* sidebarLayout_;
    QFrame* sidebarFrame_;
    
    // 侧边栏内容
    QLabel* welcomeLabel_;
    AvatarWidget* avatarWidget_;
    QPushButton* avatarChangeButton_;
    QComboBox* categoryCombo_;
    QGroupBox* quickQuestionsGroup_;
    QListWidget* quickQuestionsList_;
    QGroupBox* historyGroup_;
    QListWidget* historyList_;
    QPushButton* clearHistoryButton_;
    
    // 简化历史记录UI组件
    QComboBox* historyFilterCombo_;
    QLineEdit* advancedSearchEdit_;
    
    // 动画和效果
    QPropertyAnimation* sidebarAnimation_;
    QTimer* typingTimer_;
    ChatBubble* typingBubble_;
    QGraphicsOpacityEffect* fadeEffect_;
    
    // 状态变量
    bool sidebarVisible_;
    bool isTyping_;
    QList<QAResult> currentHistory_;
    
    // 私有方法
    void setupUI();
    void setupMainChat();
    void setupSidebar();
    void setupTitleBar();
    void setupInputArea();
    void setupConnections();
    void applyModernStyles();
    
    // 聊天功能
    void addMessage(MessageType type, const QString& message);
    void addBotResponse(const QAResult& result);
    void showTypingIndicator();
    void hideTypingIndicator();
    void scrollToBottom();
    void updateCharacterCount();
    
    // 历史记录和快速问题
    void loadQuickQuestions();
    void loadHistory();
    void addToHistory(const QString& question, const QAResult& result);
    void displayHistoryItem(const QString& question, const QAResult& result);
    
    // 增强的历史记录功能
    void searchHistory(const QString& keyword);
    void filterHistoryByCategory(const QString& category);
    void exportHistoryToFile();
    void clearHistoryByDateRange(const QDate& startDate, const QDate& endDate);
    void showHistoryStatistics();
    
    // 工具方法
    QString formatAnswer(const QAResult& result);
    QString formatTimestamp(const QDateTime& timestamp);
    void animateNewMessage(ChatBubble* bubble);
    void updateOnlineStatus();
    
    // 样式辅助方法
    QString getGradientBackground();
    QString getModernButtonStyle();
    QString getInputAreaStyle();
    QString getSidebarStyle();
};

#endif // MODERNQAWIDGET_H
