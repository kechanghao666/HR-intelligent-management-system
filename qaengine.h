#ifndef QAENGINE_H
#define QAENGINE_H

#include <QObject>
#include <QString>
#include <QList>
#include <QMap>
#include <QRegularExpression>
#include "database.h"
#include "documentprocessor.h"
#include "xfyun_api.h"

struct QAResult {
    QString answer;
    QString category;
    QString confidence;
    QList<Document> relatedDocuments;
    QString sourceDocument;
    int documentSection;
    // AI相关字段
    bool isAIGenerated;
    QString aiAnswer;
    QString traditionalAnswer;
    QString aiErrorMessage;
    
    // 默认构造函数，确保字段被正确初始化
    QAResult() 
        : documentSection(0)
        , isAIGenerated(false)
    {}
};

class QAEngine : public QObject
{
    Q_OBJECT

public:
    explicit QAEngine(Database* database, QObject *parent = nullptr);
    explicit QAEngine(Database* database, DocumentProcessor* docProcessor, QObject *parent = nullptr);
    
    // 设置文档处理器
    void setDocumentProcessor(DocumentProcessor* docProcessor);

    // 主要问答功能
    QAResult askQuestion(const QString& question);
    
    // 分类问答功能
    QAResult askAttendanceQuestion(const QString& question);
    QAResult askSalaryQuestion(const QString& question);
    QAResult askPerformanceQuestion(const QString& question);
    QAResult askOnboardingQuestion(const QString& question);
    QAResult askBenefitsQuestion(const QString& question);

    // 文档检索功能
    QList<Document> searchDocuments(const QString& keyword);
    Document findRelatedDocument(const QString& question, const QString& category);

    // 知识库管理
    void updateKnowledgeBase();
    void addCustomQA(const QString& question, const QString& answer, const QString& category);
    
    // AI相关功能
    void askQuestionWithAI(const QString& question);
    void enableAI(bool enable);
    bool isAIEnabled() const;

signals:
    void aiAnswerReceived(const QString& question, const QAResult& result);

private slots:
    void onAIAnswerReceived(const QString& answer);

private:
    Database* database_;
    DocumentProcessor* documentProcessor_;
    QMap<QString, QList<QPair<QString, QString>>> knowledgeBase_;
    
    // AI相关成员
    XfyunAPI* xfyunAPI_;
    bool aiEnabled_;
    QString currentQuestion_;           // 用于跟踪当前正在处理的问题
    QAResult currentTraditionalResult_; // 缓存当前问题的传统答案
    
    // 关键词映射
    QMap<QString, QStringList> attendanceKeywords_;
    QMap<QString, QStringList> salaryKeywords_;
    QMap<QString, QStringList> performanceKeywords_;
    QMap<QString, QStringList> onboardingKeywords_;
    QMap<QString, QStringList> benefitsKeywords_;

    // 私有辅助方法
    void initializeKeywords();
    QString categorizeQuestion(const QString& question);
    QAResult searchInKnowledgeBase(const QString& question, const QString& category);
    QString findBestMatch(const QString& question, const QList<QPair<QString, QString>>& qaList);
    double calculateSimilarity(const QString& str1, const QString& str2);
    QString extractKeywords(const QString& text);
    QList<Document> findRelatedDocuments(const QString& question, const QString& category);
    QString extractAnswerFromDocuments(const QString& question, const QList<Document>& documents);
    QString extractRelevantParagraph(const QString& content, const QStringList& keywords);
    double calculateDocumentRelevance(const QString& question, const Document& document);
    
    // AI相关私有方法
    QAResult combineTraditionalAndAIAnswers(const QAResult& traditionalResult, const QString& aiAnswer);
    QString buildContextForAI(const QString& question, const QAResult& traditionalResult);

    // 预设问答数据
    void loadPresetQA();
    
    // 考勤相关问答
    void loadAttendanceQA();
    
    // 薪酬相关问答
    void loadSalaryQA();
    
    // 绩效相关问答
    void loadPerformanceQA();
    
    // 入职离职相关问答
    void loadOnboardingQA();
    
    // 员工福利相关问答
    void loadBenefitsQA();
};

#endif // QAENGINE_H
