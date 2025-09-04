#ifndef FORMTEMPLATE_H
#define FORMTEMPLATE_H

#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDate>

/**
 * @brief 表单字段类型枚举
 */
enum class FieldType {
    Text,        // 文本字段
    Number,      // 数字字段  
    Date,        // 日期字段
    Email,       // 邮箱字段
    Phone,       // 电话字段
    Selection,   // 选择字段
    Boolean      // 布尔字段
};

/**
 * @brief 表单字段定义
 */
struct FormField {
    QString id;              // 字段ID
    QString label;           // 字段标签
    FieldType type;          // 字段类型
    bool required;           // 是否必填
    QStringList options;     // 选择项（用于Selection类型）
    QString defaultValue;    // 默认值
    QString placeholder;     // 占位符文本
    QStringList keywords;    // 关键词列表（用于自然语言识别）
    
    FormField(const QString& id = "", const QString& label = "", FieldType type = FieldType::Text)
        : id(id), label(label), type(type), required(false) {}
};

/**
 * @brief 表单模板基类
 */
class FormTemplate
{
public:
    FormTemplate(const QString& name = "", const QString& description = "");
    virtual ~FormTemplate() = default;

    // 基础属性
    QString getName() const { return name_; }
    QString getDescription() const { return description_; }
    QString getId() const { return id_; }
    
    // 字段管理
    void addField(const FormField& field);
    QList<FormField> getFields() const { return fields_; }
    FormField getField(const QString& fieldId) const;
    bool hasField(const QString& fieldId) const;
    
    // 数据处理
    void setFieldValue(const QString& fieldId, const QVariant& value);
    QVariant getFieldValue(const QString& fieldId) const;
    QMap<QString, QVariant> getAllValues() const { return values_; }
    
    // 验证
    bool isValid() const;
    QStringList getValidationErrors() const;
    
    // 序列化
    QJsonObject toJson() const;
    void fromJson(const QJsonObject& json);
    
    // 导出
    QString toHtml() const;
    QString toCsv() const;
    
protected:
    QString id_;                           // 表单ID
    QString name_;                         // 表单名称  
    QString description_;                  // 表单描述
    QList<FormField> fields_;             // 字段列表
    QMap<QString, QVariant> values_;      // 字段值

private:
    static QString generateId();
};

/**
 * @brief 员工信息表模板
 */
class EmployeeInfoForm : public FormTemplate
{
public:
    EmployeeInfoForm();
    
    // 便捷方法
    void setBasicInfo(const QString& name, const QString& id, const QString& phone);
    void setJobInfo(const QString& department, const QString& position, const QDate& hireDate);
    void setPersonalInfo(const QString& gender, const QDate& birthday, const QString& email);
};

/**
 * @brief 请假申请表模板
 */
class LeaveApplicationForm : public FormTemplate
{
public:
    LeaveApplicationForm();
    
    // 便捷方法
    void setApplicantInfo(const QString& name, const QString& employeeId, const QString& department);
    void setLeaveInfo(const QString& type, const QDate& startDate, const QDate& endDate, const QString& reason);
    void setContactInfo(const QString& phone, const QString& emergencyContact);
};

/**
 * @brief 表单工厂类
 */
class FormFactory
{
public:
    enum FormType {
        EmployeeInfo,
        LeaveApplication
    };
    
    static FormTemplate* createForm(FormType type);
    static QStringList getAvailableFormTypes();
    static QString getFormTypeName(FormType type);
};

#endif // FORMTEMPLATE_H
