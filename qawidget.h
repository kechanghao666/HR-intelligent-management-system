#ifndef QAWIDGET_H
#define QAWIDGET_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QListWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QScrollArea>
#include <QFrame>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QListWidgetItem>
#include <QTextBrowser>
#include <QSplitter>
#include "qaengine.h"
#include "database.h"
#include "user.h"

class QAWidget : public QWidget
{
    Q_OBJECT

public:
    explicit QAWidget(QAEngine* qaEngine, Database* database, const User& currentUser, QWidget *parent = nullptr);
    ~QAWidget();

public slots:
    void askQuestion();
    void showQuickQuestions();
    void showHistory();
    void clearHistory();
    void showRelatedDocuments(const QAResult& result);

private slots:
    void onQuestionSubmitted();
    void onQuickQuestionClicked(QListWidgetItem* item);
    void onHistoryItemClicked(QListWidgetItem* item);
    void onCategoryChanged(const QString& category);
    void onSearchTextChanged(const QString& text);
    void performSearch(); // 执行搜索的槽函数
    void onDocumentLinkClicked(const QString& link);
    void animateAnswer();
    
    // AI相关槽函数
    void onAIAnswerReceived(const QString& question, const QAResult& result);


signals:
    void questionAsked(const QString& question, const QAResult& result);

private:
    QAEngine* qaEngine_;
    Database* database_;
    User currentUser_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QSplitter* mainSplitter_;
    
    // 左侧面板
    QWidget* leftPanel_;
    QVBoxLayout* leftLayout_;
    QLabel* welcomeLabel_;
    QComboBox* categoryCombo_;
    QLineEdit* searchEdit_;
    QPushButton* searchButton_;
    QGroupBox* quickQuestionsGroup_;
    QListWidget* quickQuestionsList_;
    QGroupBox* historyGroup_;
    QListWidget* historyList_;
    QPushButton* clearHistoryButton_;
    
    // 右侧问答区域
    QWidget* rightPanel_;
    QVBoxLayout* rightLayout_;
    QGroupBox* questionGroup_;
    QTextEdit* questionEdit_;
    QPushButton* askButton_;
    QGroupBox* answerGroup_;
    QTextBrowser* answerBrowser_;
    QLabel* confidenceLabel_;
    QLabel* categoryLabel_;
    QGroupBox* documentsGroup_;
    QTextBrowser* documentsBrowser_;
    
    // 动画相关
    QPropertyAnimation* answerAnimation_;
    QGraphicsOpacityEffect* answerOpacityEffect_;
    
    // 状态相关
    QTimer* searchTimer_;
    QList<QAResult> currentHistory_;
    QString currentSearchText_; // 当前搜索文本

    // 私有方法
    void setupUI();
    void setupConnections();
    void applyStyles();
    void loadQuickQuestions();
    void loadHistory();
    void displayAnswer(const QAResult& result);
    void displayRelatedDocuments(const QList<Document>& documents);
    void addToHistory(const QString& question, const QAResult& result);
    void updateConfidenceLabel(const QString& confidence);
    void updateCategoryLabel(const QString& category);
    void showLoadingAnimation();
    void hideLoadingAnimation();
    void createQuickQuestionItems();
    void createHistoryItems();
    QString formatAnswer(const QAResult& result);
    QString formatDocumentList(const QList<Document>& documents);
    
    // 样式相关
    void setupAnimations();
    void createGradientBackground();
    void applyAnswerStyle();
};

#endif // QAWIDGET_H
