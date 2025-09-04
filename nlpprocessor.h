#ifndef NLPPROCESSOR_H
#define NLPPROCESSOR_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QRegularExpression>
#include <QDate>
#include <QDateTime>
#include "formtemplate.h"

/**
 * @brief 提取结果结构
 */
struct ExtractionResult {
    QString fieldId;        // 字段ID
    QVariant value;         // 提取的值
    QString source;         // 源文本片段
    double confidence;      // 置信度 (0.0-1.0)
    
    ExtractionResult(const QString& id = "", const QVariant& val = QVariant(), 
                    const QString& src = "", double conf = 0.0)
        : fieldId(id), value(val), source(src), confidence(conf) {}
};

/**
 * @brief 表单识别结果
 */
struct FormRecognitionResult {
    FormFactory::FormType formType;
    QString formName;
    double confidence;
    QList<ExtractionResult> extractions;
    
    FormRecognitionResult(FormFactory::FormType type = FormFactory::EmployeeInfo, 
                         const QString& name = "", double conf = 0.0)
        : formType(type), formName(name), confidence(conf) {}
};

/**
 * @brief 自然语言处理器 - 用于从用户输入中提取表单信息
 */
class NLPProcessor
{
public:
    NLPProcessor();
    ~NLPProcessor() = default;

    // 主要功能接口
    FormRecognitionResult recognizeFormType(const QString& text);
    QList<ExtractionResult> extractFieldValues(const QString& text, FormTemplate* form);
    FormTemplate* autoFillForm(const QString& text);
    
    // 辅助功能
    QString cleanText(const QString& text);
    QStringList tokenize(const QString& text);
    
    // 特定提取功能
    QString extractName(const QString& text);
    QString extractPhone(const QString& text);
    QString extractEmail(const QString& text);
    QDate extractDate(const QString& text);
    QString extractDepartment(const QString& text);
    QString extractLeaveType(const QString& text);
    int extractNumber(const QString& text);
    bool isValidName(const QString& name);
    
    // 备用简单提取
    QList<ExtractionResult> simpleExtractBackup(const QString& text, const FormField& field);

private:
    // 初始化方法
    void initializePatterns();
    void initializeDepartments();
    void initializeLeaveTypes();
    void initializeDatePatterns();
    
    // 核心提取方法
    QList<ExtractionResult> extractWithKeywords(const QString& text, const FormField& field);
    QList<ExtractionResult> extractWithPatterns(const QString& text, const FormField& field);
    double calculateConfidence(const QString& text, const QStringList& keywords, const QString& extracted);
    
    // 特定类型提取器
    QList<ExtractionResult> extractTextFields(const QString& text, const FormField& field);
    QList<ExtractionResult> extractDateFields(const QString& text, const FormField& field);
    QList<ExtractionResult> extractSelectionFields(const QString& text, const FormField& field);
    QList<ExtractionResult> extractNumberFields(const QString& text, const FormField& field);
    
    // 数据成员
    QMap<QString, QRegularExpression> patterns_;          // 正则表达式模式
    QStringList departments_;                              // 部门列表
    QStringList leaveTypes_;                              // 请假类型列表
    QList<QRegularExpression> datePatterns_;             // 日期模式列表
    QMap<QString, QString> dateFormats_;                 // 日期格式映射
    
    // 常量
    static const double MIN_CONFIDENCE;
    static const int MAX_EXTRACTION_DISTANCE;
};

/**
 * @brief 自动填表助手 - 整合NLP处理器和表单模板
 */
class AutoFormFiller
{
public:
    AutoFormFiller();
    ~AutoFormFiller() = default;
    
    // 主要功能
    FormTemplate* processUserInput(const QString& userInput, QString& responseMessage);
    QString generateResponseMessage(const FormRecognitionResult& result, FormTemplate* form);
    FormRecognitionResult updateExistingForm(const QString& input);
    
    // 配置
    void setMinConfidence(double minConfidence) { minConfidence_ = minConfidence; }
    double getMinConfidence() const { return minConfidence_; }

private:
    NLPProcessor processor_;
    double minConfidence_;
    
    // 辅助方法
    QString formatFieldValue(const FormField& field, const QVariant& value);
    QString generateSummary(FormTemplate* form);
    QString generateMissingFieldsHint(FormTemplate* form);
};

#endif // NLPPROCESSOR_H
