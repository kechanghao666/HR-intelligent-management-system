#ifndef AIASSISTANTWIDGET_H
#define AIASSISTANTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QScrollArea>
#include <QTimer>
#include <QScrollBar>
#include <QListWidget>
#include <QListWidgetItem>
#include <QGroupBox>
#include <QLineEdit>
#include <QMessageBox>
#include "xfyun_api.h"
#include "chatbubble.h"
#include "formtemplate.h"
#include "nlpprocessor.h"
#include "formexporter.h"

/**
 * @brief AI助手界面 - 仿照智能问答页面设计，基于SmartQA的稳定实现
 */
class AIAssistantWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AIAssistantWidget(QWidget *parent = nullptr);
    ~AIAssistantWidget();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void onSendClicked();
    void onInputTextChanged();
    void onAIAnswerReceived(const QString &answer);
    void onClearChat();
    void updateConnectionStatus();
    void onNetworkTimeout();
    
    // 侧边栏历史记录槽函数
    void onSidebarToggled();
    void onHistoryItemClicked(QListWidgetItem* item);
    void onClearHistoryClicked();
    void onHistorySearchChanged();
    void onShowRelatedClauses(const QString& question);
    
    // 自动填表功能槽函数
    void onAutoFormRequest(const QString& userInput);
    void onExportFormRequest(const QString& format);
    void onViewFormRequest();
    void onClearFormRequest();
    void onModeToggled();

private:
    void setupUI();
    void setupTitleBar();
    void setupMessagesArea();
    void setupInputArea();
    void setupSidebar();
    void setupConnections();
    void applyModernStyles();
    
    void addUserMessage(const QString &message);
    void addAIMessage(const QString &message);
    void addSystemMessage(const QString &message);
    void scrollToBottom();
    void showWelcomeMessage();
    void updateWelcomeMessageForCurrentMode();
    void enableInput(bool enabled);
    
    // 历史记录管理
    void addToHistory(const QString &question, const QString &answer);
    void loadHistory();
    void saveHistory();
    void filterHistory(const QString &keyword);
    
    // 自动填表功能
    void processFormInput(const QString &input);
    bool isFormCommand(const QString &input);
    void handleFormCommand(const QString &command);
    void showFormPreview(FormTemplate* form);
    void exportCurrentForm(ExportFormat format);
    void addFormResponseMessage(const QString &message);
    bool hasActiveForm() const;
    void clearActiveForm();
    QString getFormStatusMessage();
    QString generateRelatedTerms(const QString& message);
    
    // 布局组件
    QVBoxLayout* mainLayout_;
    
    // 标题栏 - 仿照ModernQAWidget
    QFrame* titleBar_;
    QHBoxLayout* titleLayout_;
    QLabel* titleLabel_;
    QLabel* statusLabel_;
    QPushButton* clearButton_;
    QPushButton* sidebarToggleButton_;
    QPushButton* modeToggleButton_;
    
    // 消息区域 - 仿照ModernQAWidget
    QScrollArea* messagesScrollArea_;
    QWidget* messagesContainer_;
    QVBoxLayout* messagesLayout_;
    
    // 输入区域 - 仿照ModernQAWidget
    QFrame* inputFrame_;
    QHBoxLayout* inputLayout_;
    QTextEdit* inputEdit_;
    QPushButton* sendButton_;
    QLabel* characterCountLabel_;
    
    // 侧边栏历史记录模块
    QWidget* sidebarWidget_;
    QVBoxLayout* sidebarLayout_;
    QFrame* sidebarFrame_;
    QGroupBox* historyGroup_;
    QListWidget* historyList_;
    QPushButton* clearHistoryButton_;
    QLineEdit* historySearchEdit_;
    bool sidebarVisible_;
    
    // 功能组件
    XfyunAPI* xfyunAPI_;
    QTimer* statusTimer_;
    QTimer* timeoutTimer_;
    
    // 状态管理
    bool isProcessing_;
    int messageCount_;
    QString currentQuestion_; // 存储当前正在处理的问题
    static const int MAX_CHARS = 1000;
    
    // 自动填表组件
    AutoFormFiller* formFiller_;
    FormTemplate* currentForm_;
    FormExporter* formExporter_;
    FormPreviewer* formPreviewer_;
    bool formMode_; // 是否处于表单填写模式
    bool isFormModeEnabled_; // 是否启用表单模式
    bool hasShownWelcome_; // 是否已显示欢迎消息
};

#endif // AIASSISTANTWIDGET_H