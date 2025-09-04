#include "formtemplate.h"
#include <QUuid>
#include <QDebug>
#include <QRegularExpression>

// ===== FormTemplate 实现 =====

FormTemplate::FormTemplate(const QString& name, const QString& description)
    : name_(name), description_(description)
{
    id_ = generateId();
}

void FormTemplate::addField(const FormField& field)
{
    fields_.append(field);
}

FormField FormTemplate::getField(const QString& fieldId) const
{
    for (const auto& field : fields_) {
        if (field.id == fieldId) {
            return field;
        }
    }
    return FormField();
}

bool FormTemplate::hasField(const QString& fieldId) const
{
    for (const auto& field : fields_) {
        if (field.id == fieldId) {
            return true;
        }
    }
    return false;
}

void FormTemplate::setFieldValue(const QString& fieldId, const QVariant& value)
{
    if (hasField(fieldId)) {
        values_[fieldId] = value;
        qDebug() << "[FormTemplate] 设置字段" << fieldId << "=" << value.toString();
    } else {
        qDebug() << "[FormTemplate] 警告：字段" << fieldId << "不存在，无法设置值" << value.toString();
    }
}

QVariant FormTemplate::getFieldValue(const QString& fieldId) const
{
    return values_.value(fieldId);
}

bool FormTemplate::isValid() const
{
    return getValidationErrors().isEmpty();
}

QStringList FormTemplate::getValidationErrors() const
{
    QStringList errors;
    
    for (const auto& field : fields_) {
        if (field.required && !values_.contains(field.id)) {
            errors.append(QString("%1 是必填字段").arg(field.label));
            continue;
        }
        
        if (values_.contains(field.id)) {
            QVariant value = values_[field.id];
            QString strValue = value.toString().trimmed();
            
            if (field.required && strValue.isEmpty()) {
                errors.append(QString("%1 不能为空").arg(field.label));
                continue;
            }
            
            // 类型验证
            switch (field.type) {
            case FieldType::Email:
                if (!strValue.isEmpty() && !strValue.contains('@')) {
                    errors.append(QString("%1 格式不正确").arg(field.label));
                }
                break;
            case FieldType::Phone:
                if (!strValue.isEmpty()) {
                    QRegularExpression phoneRegex("^[0-9\\-\\+\\(\\)\\s]{7,20}$");
                    if (!phoneRegex.match(strValue).hasMatch()) {
                        errors.append(QString("%1 格式不正确").arg(field.label));
                    }
                }
                break;
            case FieldType::Number:
                if (!strValue.isEmpty()) {
                    bool ok;
                    strValue.toDouble(&ok);
                    if (!ok) {
                        errors.append(QString("%1 必须是数字").arg(field.label));
                    }
                }
                break;
            default:
                break;
            }
        }
    }
    
    return errors;
}

QJsonObject FormTemplate::toJson() const
{
    QJsonObject obj;
    obj["id"] = id_;
    obj["name"] = name_;
    obj["description"] = description_;
    
    QJsonObject valuesObj;
    for (auto it = values_.begin(); it != values_.end(); ++it) {
        valuesObj[it.key()] = QJsonValue::fromVariant(it.value());
    }
    obj["values"] = valuesObj;
    
    return obj;
}

void FormTemplate::fromJson(const QJsonObject& json)
{
    id_ = json["id"].toString();
    name_ = json["name"].toString();
    description_ = json["description"].toString();
    
    QJsonObject valuesObj = json["values"].toObject();
    values_.clear();
    for (auto it = valuesObj.begin(); it != valuesObj.end(); ++it) {
        values_[it.key()] = it.value().toVariant();
    }
}

QString FormTemplate::toHtml() const
{
    qDebug() << "[FormTemplate::toHtml] 开始生成HTML，表单名称:" << name_;
    qDebug() << "[FormTemplate::toHtml] values_容器大小:" << values_.size();
    qDebug() << "[FormTemplate::toHtml] fields_容器大小:" << fields_.size();
    
    // 调试：输出所有values内容
    for (auto it = values_.begin(); it != values_.end(); ++it) {
        qDebug() << "[FormTemplate::toHtml] values_[" << it.key() << "] =" << it.value().toString();
    }
    
    QString html = QString(
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta charset='UTF-8'>"
        "<title>%1</title>"
        "<style>"
        "body { "
        "  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Microsoft YaHei', Arial, sans-serif; "
        "  margin: 0; padding: 32px; "
        "  background: #f5f7fa; "
        "  line-height: 1.65; "
        "  color: #1f2937; "
        "  display: flex; "
        "  justify-content: center; "
        "}"
        ".container { "
        "  width: 860px; "
        "  max-width: 92%; "
        "  margin: 0 auto; "
        "  background: #ffffff; "
        "  border-radius: 12px; "
        "  box-shadow: 0 6px 24px rgba(0,0,0,0.08); "
        "  overflow: hidden; "
        "}"
        ".form-header { "
        "  background: #1f2937; "
        "  color: #ffffff; "
        "  padding: 40px 30px; "
        "  text-align: center; "
        "}"
        ".form-title { "
        "  font-size: 32px; "
        "  font-weight: 700; "
        "  margin: 0 0 8px 0; "
        "  letter-spacing: 0.5px; "
        "}"
        ".form-desc { "
        "  font-size: 16px; "
        "  opacity: 0.9; "
        "  margin: 0; "
        "  font-weight: 300; "
        "}"
        ".form-table { "
        "  width: 80%%; "
        "  border-collapse: separate; "
        "  border-spacing: 0; "
        "  font-size: 16px; "
        "  margin: 0 auto; "
        "}"
        ".form-table tr:nth-child(even) { "
        "  background-color: #f8f9fa; "
        "}"
        ".form-table tr:hover { "
        "  background-color: #e3f2fd; "
        "  transition: background-color 0.2s ease; "
        "}"
        ".form-table td { "
        "  padding: 18px 24px; "
        "  border-bottom: 1px solid #e5e7eb; "
        "  vertical-align: middle; "
        "}"
        ".label-cell { "
        "  background: #111827; "
        "  color: #ffffff; "
        "  font-weight: 600; "
        "  width: 30%%; "
        "  text-align: right; "
        "  border-right: 3px solid #667eea; "
        "}"
        ".value-cell { "
        "  background: #ffffff; "
        "  color: #111827; "
        "  width: 70%%; "
        "  text-align: left; "
        "}"
        ".empty-value { "
        "  color: #6c757d; "
        "  font-style: italic; "
        "  opacity: 0.7; "
        "}"
        ".footer { "
        "  background: #f8f9fa; "
        "  padding: 20px 30px; "
        "  text-align: center; "
        "  font-size: 13px; "
        "  color: #6c757d; "
        "  border-top: 1px solid #e9ecef; "
        "}"
        "@media print { "
        "  body { background: white; padding: 0; } "
        "  .container { box-shadow: none; border-radius: 0; } "
        "  .form-header { background: #667eea !important; -webkit-print-color-adjust: exact; } "
        "  .form-table tr:nth-child(even) { background: #f8f9fa !important; -webkit-print-color-adjust: exact; } "
        "}"
        "</style>"
        "</head><body>"
        "<div class='container'>"
        "<div class='form-header'>"
        "<h1 class='form-title'>%1</h1>"
        "<p class='form-desc'>%2</p>"
        "</div>"
        "<table class='form-table'>"
    ).arg(name_, description_);
    
    // 显示所有字段，包括空值字段（用于调试）
    for (const auto& field : fields_) {
        QString value;
        QString cellClass = "value-cell";
        
        if (values_.contains(field.id)) {
            value = values_[field.id].toString();
            if (value.isEmpty()) {
                value = "（待填写）";
                cellClass = "empty-value";
            }
        } else {
            value = "（待填写）";
            cellClass = "empty-value";
        }
        
        // 调试输出，帮助定位问题
        qDebug() << "[表单导出] 字段" << field.label << "(" << field.id << ") 值:" << value << "包含在values中:" << values_.contains(field.id);
        
        html += QString(
            "<tr>"
            "<td class='label-cell'>%1</td>"
            "<td class='%2'>%3</td>"
            "</tr>"
        ).arg(field.label, cellClass, value);
    }
    
    html += "</table>";
    html += QString("<div class='footer'>生成时间：%1 | HR智能管理系统</div>").arg(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm"));
    html += "</div></body></html>";
    
    qDebug() << "[FormTemplate::toHtml] HTML生成完成，长度:" << html.length();
    qDebug() << "[FormTemplate::toHtml] HTML内容预览:" << html.left(200) << "...";
    
    return html;
}

QString FormTemplate::toCsv() const
{
    QString csv = QString("%1,%2\n").arg(name_, description_);
    csv += "字段,值\n";
    
    for (const auto& field : fields_) {
        if (values_.contains(field.id)) {
            QString value = values_[field.id].toString();
            if (!value.isEmpty()) {
                // CSV格式转义
                if (value.contains(',') || value.contains('"') || value.contains('\n')) {
                    value = QString("\"%1\"").arg(value.replace("\"", "\"\""));
                }
                csv += QString("%1,%2\n").arg(field.label, value);
            }
        }
    }
    
    return csv;
}

QString FormTemplate::generateId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

// ===== EmployeeInfoForm 实现 =====

EmployeeInfoForm::EmployeeInfoForm()
    : FormTemplate("员工信息表", "用于记录和管理员工基本信息")
{
    // 基本信息
    FormField nameField("name", "姓名", FieldType::Text);
    nameField.required = true;
    nameField.keywords = {"姓名", "名字", "叫", "我是", "我叫"};
    addField(nameField);
    
    FormField employeeIdField("employeeId", "工号", FieldType::Text);
    employeeIdField.required = true;
    employeeIdField.keywords = {"工号", "员工号", "编号", "工作编号"};
    addField(employeeIdField);
    
    FormField phoneField("phone", "手机号码", FieldType::Phone);
    phoneField.required = true;
    phoneField.keywords = {"手机", "电话", "联系方式", "号码", "联系电话"};
    addField(phoneField);
    
    FormField emailField("email", "邮箱", FieldType::Email);
    emailField.keywords = {"邮箱", "邮件", "email", "@"};
    addField(emailField);
    
    // 职位信息
    FormField departmentField("department", "部门", FieldType::Selection);
    departmentField.required = true;
    departmentField.options = {"人事部", "技术部", "财务部", "市场部", "运营部", "行政部"};
    departmentField.keywords = {"部门", "科室", "处室", "所在部门"};
    addField(departmentField);
    
    FormField positionField("position", "职位", FieldType::Text);
    positionField.required = true;
    positionField.keywords = {"职位", "岗位", "职务", "工作", "担任"};
    addField(positionField);
    
    FormField hireDateField("hireDate", "入职日期", FieldType::Date);
    hireDateField.required = true;
    hireDateField.keywords = {"入职", "入职时间", "入职日期", "开始工作", "加入公司"};
    addField(hireDateField);
    
    // 个人信息
    FormField genderField("gender", "性别", FieldType::Selection);
    genderField.options = {"男", "女"};
    genderField.keywords = {"性别", "男", "女"};
    addField(genderField);
    
    FormField birthdayField("birthday", "出生日期", FieldType::Date);
    birthdayField.keywords = {"生日", "出生", "出生日期", "年龄"};
    addField(birthdayField);
    
    FormField addressField("address", "家庭地址", FieldType::Text);
    addressField.keywords = {"地址", "住址", "家庭地址", "居住地", "住在"};
    addField(addressField);
}

void EmployeeInfoForm::setBasicInfo(const QString& name, const QString& id, const QString& phone)
{
    setFieldValue("name", name);
    setFieldValue("employeeId", id);
    setFieldValue("phone", phone);
}

void EmployeeInfoForm::setJobInfo(const QString& department, const QString& position, const QDate& hireDate)
{
    setFieldValue("department", department);
    setFieldValue("position", position);
    setFieldValue("hireDate", hireDate);
}

void EmployeeInfoForm::setPersonalInfo(const QString& gender, const QDate& birthday, const QString& email)
{
    setFieldValue("gender", gender);
    setFieldValue("birthday", birthday);
    setFieldValue("email", email);
}

// ===== LeaveApplicationForm 实现 =====

LeaveApplicationForm::LeaveApplicationForm()
    : FormTemplate("请假申请表", "用于员工请假申请和审批流程")
{
    // 申请人信息
    FormField nameField("applicantName", "申请人姓名", FieldType::Text);
    nameField.required = true;
    nameField.keywords = {"姓名", "名字", "叫", "我是", "我叫", "申请人"};
    addField(nameField);
    
    FormField employeeIdField("employeeId", "工号", FieldType::Text);
    employeeIdField.required = true;
    employeeIdField.keywords = {"工号", "员工号", "编号"};
    addField(employeeIdField);
    
    FormField departmentField("department", "所属部门", FieldType::Selection);
    departmentField.required = true;
    departmentField.options = {"人事部", "技术部", "财务部", "市场部", "运营部", "行政部"};
    departmentField.keywords = {"部门", "科室", "处室"};
    addField(departmentField);
    
    // 请假信息
    FormField leaveTypeField("leaveType", "请假类型", FieldType::Selection);
    leaveTypeField.required = true;
    leaveTypeField.options = {"病假", "事假", "年假", "婚假", "产假", "陪产假", "丧假", "其他"};
    leaveTypeField.keywords = {"请假类型", "假期类型", "病假", "事假", "年假", "婚假", "产假", "丧假"};
    addField(leaveTypeField);
    
    FormField startDateField("startDate", "请假开始日期", FieldType::Date);
    startDateField.required = true;
    startDateField.keywords = {"开始日期", "开始时间", "从", "自", "起始日期"};
    addField(startDateField);
    
    FormField endDateField("endDate", "请假结束日期", FieldType::Date);
    endDateField.required = true;
    endDateField.keywords = {"结束日期", "结束时间", "到", "至", "截止日期"};
    addField(endDateField);
    
    FormField daysField("totalDays", "请假天数", FieldType::Number);
    daysField.keywords = {"天数", "几天", "多少天", "总天数"};
    addField(daysField);
    
    FormField reasonField("reason", "请假原因", FieldType::Text);
    reasonField.required = true;
    reasonField.keywords = {"原因", "理由", "因为", "由于"};
    addField(reasonField);
    
    // 联系信息
    FormField phoneField("phone", "联系电话", FieldType::Phone);
    phoneField.required = true;
    phoneField.keywords = {"电话", "手机", "联系方式"};
    addField(phoneField);
    
    FormField emergencyContactField("emergencyContact", "紧急联系人", FieldType::Text);
    emergencyContactField.keywords = {"紧急联系人", "应急联系人", "家属", "联系人"};
    addField(emergencyContactField);
    
    FormField applicationDateField("applicationDate", "申请日期", FieldType::Date);
    applicationDateField.defaultValue = QDate::currentDate().toString("yyyy-MM-dd");
    applicationDateField.keywords = {"申请日期", "申请时间", "今天"};
    addField(applicationDateField);
}

void LeaveApplicationForm::setApplicantInfo(const QString& name, const QString& employeeId, const QString& department)
{
    setFieldValue("applicantName", name);
    setFieldValue("employeeId", employeeId);
    setFieldValue("department", department);
}

void LeaveApplicationForm::setLeaveInfo(const QString& type, const QDate& startDate, const QDate& endDate, const QString& reason)
{
    setFieldValue("leaveType", type);
    setFieldValue("startDate", startDate);
    setFieldValue("endDate", endDate);
    setFieldValue("reason", reason);
    
    // 自动计算天数
    int days = 0;
    if (startDate.isValid() && endDate.isValid()) {
        days = startDate.daysTo(endDate) + 1; // 含首尾
        if (days < 0) {
            days = 0;
        }
    }
    setFieldValue("totalDays", days);
}

void LeaveApplicationForm::setContactInfo(const QString& phone, const QString& emergencyContact)
{
    setFieldValue("phone", phone);
    setFieldValue("emergencyContact", emergencyContact);
}

// ===== FormFactory 实现 =====

FormTemplate* FormFactory::createForm(FormType type)
{
    switch (type) {
    case EmployeeInfo:
        return new EmployeeInfoForm();
    case LeaveApplication:
        return new LeaveApplicationForm();
    default:
        return nullptr;
    }
}

QStringList FormFactory::getAvailableFormTypes()
{
    return {"员工信息表", "请假申请表"};
}

QString FormFactory::getFormTypeName(FormType type)
{
    switch (type) {
    case EmployeeInfo:
        return "员工信息表";
    case LeaveApplication:
        return "请假申请表";
    default:
        return "未知类型";
    }
}
