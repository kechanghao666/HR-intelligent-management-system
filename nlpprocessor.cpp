#include "nlpprocessor.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>
#include <QLocale>
#include <QTextStream>
#include <algorithm>

// 常量定义
const double NLPProcessor::MIN_CONFIDENCE = 0.3;
const int NLPProcessor::MAX_EXTRACTION_DISTANCE = 20;

// ===== NLPProcessor 实现 =====

NLPProcessor::NLPProcessor()
{
    initializePatterns();
    initializeDepartments(); 
    initializeLeaveTypes();
    initializeDatePatterns();
}

void NLPProcessor::initializePatterns()
{
    // 姓名模式 - 改进中文姓名识别（支持可选冒号）
    patterns_["name"] = QRegularExpression(R"((?:我叫|我是|姓名是|名字是|叫做)[:：]?\s*([\u4e00-\u9fff]{2,4}))");
    patterns_["name_simple"] = QRegularExpression(R"(([\u4e00-\u9fff]{2,4})(?:申请|请假|入职))");
    
    // 工号模式 - 改进工号识别  
    patterns_["employeeId"] = QRegularExpression(R"((?:工号|员工号|编号|工作编号)[:：]?\s*([A-Za-z0-9]{1,20}))");
    patterns_["employeeId_simple"] = QRegularExpression(R"(\b([A-Za-z]+\d+|\d+[A-Za-z]+)\b)");
    
    // 电话号码模式
    patterns_["phone"] = QRegularExpression(R"((?:电话|手机|联系方式|号码)[:：]?\s*(1[3-9]\d{9}))");
    patterns_["phone_simple"] = QRegularExpression(R"(\b(1[3-9]\d{9})\b)");
    
    // 邮箱模式
    patterns_["email"] = QRegularExpression(R"(([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}))");
    
    // 年龄模式
    patterns_["age"] = QRegularExpression(R"((?:年龄|今年|现在)\s*(\d{1,2})\s*岁?)");
    
    // 数字模式
    patterns_["number"] = QRegularExpression(R"(\b(\d+)\b)");
}

void NLPProcessor::initializeDepartments()
{
    departments_ = {
        "人事部", "人力资源部", "HR部", "人事科",
        "技术部", "研发部", "IT部", "工程部", "开发部",
        "财务部", "会计部", "财务科",
        "市场部", "营销部", "市场营销部", "销售部",
        "运营部", "业务部", "运营管理部",
        "行政部", "综合部", "办公室", "总办",
        "法务部", "法律部", "合规部",
        "采购部", "供应链部", "物流部",
        "客服部", "服务部", "支持部"
    };
}

void NLPProcessor::initializeLeaveTypes()
{
    leaveTypes_ = {
        "病假", "事假", "年假", "调休", "年休假",
        "婚假", "产假", "陪产假", "陪护假", "护理假",
        "丧假", "哺乳假", "探亲假",
        "工伤假", "其他假期", "其他"
    };
}

void NLPProcessor::initializeDatePatterns()
{
    // 各种日期格式模式
    datePatterns_.append(QRegularExpression(R"(\b(\d{4})[年\-/](\d{1,2})[月\-/](\d{1,2})日?\b)"));
    datePatterns_.append(QRegularExpression(R"(\b(\d{1,2})[月\-/](\d{1,2})日?\b)"));
    datePatterns_.append(QRegularExpression(R"(\b(今天|明天|后天|昨天|前天)\b)"));
    datePatterns_.append(QRegularExpression(R"(\b(下周一|下周二|下周三|下周四|下周五|下周六|下周日)\b)"));
    datePatterns_.append(QRegularExpression(R"(\b(\d{4})-(\d{2})-(\d{2})\b)"));
    
    // 日期格式映射
    dateFormats_["今天"] = QDate::currentDate().toString("yyyy-MM-dd");
    dateFormats_["明天"] = QDate::currentDate().addDays(1).toString("yyyy-MM-dd");
    dateFormats_["后天"] = QDate::currentDate().addDays(2).toString("yyyy-MM-dd");
    dateFormats_["昨天"] = QDate::currentDate().addDays(-1).toString("yyyy-MM-dd");
    dateFormats_["前天"] = QDate::currentDate().addDays(-2).toString("yyyy-MM-dd");
}

FormRecognitionResult NLPProcessor::recognizeFormType(const QString& text)
{
    QString cleanedText = cleanText(text).toLower();
    
    // 请假申请表识别
    QStringList leaveKeywords = {"请假", "休假", "病假", "事假", "年假", "婚假", "产假", "申请假期", "需要请假", "想请假"};
    int leaveScore = 0;
    for (const QString& keyword : leaveKeywords) {
        if (cleanedText.contains(keyword)) {
            leaveScore += 10;
        }
    }
    
    // 员工信息表识别  
    QStringList employeeKeywords = {"员工信息", "个人资料", "基本信息", "入职", "新员工", "员工登记", "人员信息"};
    int employeeScore = 0;
    for (const QString& keyword : employeeKeywords) {
        if (cleanedText.contains(keyword)) {
            employeeScore += 10;
        }
    }
    
    // 检查是否包含表单相关字段
    if (cleanedText.contains("姓名") || cleanedText.contains("工号") || cleanedText.contains("部门")) {
        if (cleanedText.contains("请假") || cleanedText.contains("假期")) {
            leaveScore += 5;
        } else {
            employeeScore += 5;
        }
    }
    
    FormRecognitionResult result;
    if (leaveScore > employeeScore && leaveScore >= 5) {
        result.formType = FormFactory::LeaveApplication;
        result.formName = "请假申请表";
        result.confidence = std::min(1.0, leaveScore / 20.0);
    } else if (employeeScore >= 5) {
        result.formType = FormFactory::EmployeeInfo;
        result.formName = "员工信息表";
        result.confidence = std::min(1.0, employeeScore / 20.0);
    } else {
        // 默认识别为员工信息表
        result.formType = FormFactory::EmployeeInfo;
        result.formName = "员工信息表";
        result.confidence = 0.3;
    }
    
    qDebug() << "[NLP] 表单类型识别:" << result.formName 
             << "置信度:" << result.confidence
             << "请假分数:" << leaveScore << "员工分数:" << employeeScore;
    
    return result;
}

QList<ExtractionResult> NLPProcessor::extractFieldValues(const QString& text, FormTemplate* form)
{
    QList<ExtractionResult> results;
    if (!form) return results;
    
    QString cleanedText = cleanText(text);
    QList<FormField> fields = form->getFields();
    
    for (const FormField& field : fields) {
        QList<ExtractionResult> fieldResults;
        
        switch (field.type) {
        case FieldType::Text:
        case FieldType::Email:
        case FieldType::Phone:
            fieldResults = extractTextFields(cleanedText, field);
            break;
        case FieldType::Date:
            fieldResults = extractDateFields(cleanedText, field);
            break;
        case FieldType::Selection:
            fieldResults = extractSelectionFields(cleanedText, field);
            break;
        case FieldType::Number:
            fieldResults = extractNumberFields(cleanedText, field);
            break;
        default:
            break;
        }
        
        // 选择置信度最高的结果
        if (!fieldResults.isEmpty()) {
            auto maxResult = std::max_element(fieldResults.begin(), fieldResults.end(),
                [](const ExtractionResult& a, const ExtractionResult& b) {
                    return a.confidence < b.confidence;
                });
            if (maxResult->confidence >= MIN_CONFIDENCE) {
                results.append(*maxResult);
                qDebug() << "[NLP] 提取字段:" << field.label 
                         << "值:" << maxResult->value.toString()
                         << "置信度:" << maxResult->confidence;
            }
        }
    }
    
    return results;
}

FormTemplate* NLPProcessor::autoFillForm(const QString& text)
{
    // 识别表单类型
    FormRecognitionResult recognition = recognizeFormType(text);
    if (recognition.confidence < MIN_CONFIDENCE) {
        return nullptr;
    }
    
    // 创建表单
    FormTemplate* form = FormFactory::createForm(recognition.formType);
    if (!form) return nullptr;
    
    // 提取字段值
    QList<ExtractionResult> extractions = extractFieldValues(text, form);
    
    qDebug() << "[AutoFormFiller] 提取到" << extractions.size() << "个字段";
    
    // 填入表单
    for (const ExtractionResult& extraction : extractions) {
        qDebug() << "[AutoFormFiller] 填入字段:" << extraction.fieldId << "值:" << extraction.value.toString();
        form->setFieldValue(extraction.fieldId, extraction.value);
    }

    // 如果是请假申请表，且存在开始/结束日期但未填充天数，则自动计算
    if (recognition.formType == FormFactory::LeaveApplication) {
        QMap<QString, QVariant> vals = form->getAllValues();
        if (vals.contains("startDate") && vals.contains("endDate") && !vals.contains("totalDays")) {
            QDate s = vals.value("startDate").toDate();
            QDate e = vals.value("endDate").toDate();
            if (s.isValid() && e.isValid()) {
                int days = s.daysTo(e) + 1;
                if (days < 0) days = 0;
                form->setFieldValue("totalDays", days);
            }
        }
    }
    
    // 调试：输出表单当前状态
    QMap<QString, QVariant> allValues = form->getAllValues();
    qDebug() << "[AutoFormFiller] 表单当前值个数:" << allValues.size();
    for (auto it = allValues.begin(); it != allValues.end(); ++it) {
        qDebug() << "[AutoFormFiller] 字段" << it.key() << "=" << it.value().toString();
    }
    
    return form;
}

QList<ExtractionResult> NLPProcessor::extractTextFields(const QString& text, const FormField& field)
{
    QList<ExtractionResult> results;
    
    // 调试：输出字段提取过程
    qDebug() << "[NLP提取] 开始提取字段:" << field.id << "(" << field.label << ")";
    qDebug() << "[NLP提取] 输入文本:" << text;
    
    // 根据字段ID使用特定的提取方法
    if (field.id == "name" || field.id == "applicantName") {
        qDebug() << "[NLP提取] 识别为姓名字段";
        QString name = extractName(text);
        qDebug() << "[NLP提取] 姓名提取结果:" << name;
        if (!name.isEmpty()) {
            double confidence = calculateConfidence(text, field.keywords, name);
            results.append(ExtractionResult(field.id, name, name, confidence));
            qDebug() << "[NLP提取] 姓名字段提取成功:" << name << "置信度:" << confidence;
        } else {
            qDebug() << "[NLP提取] 姓名字段提取失败";
        }
    }
    else if (field.id == "employeeId") {
        // 工号提取
        auto match = patterns_["employeeId"].match(text);
        if (match.hasMatch()) {
            QString id = match.captured(1);
            double confidence = calculateConfidence(text, field.keywords, id);
            results.append(ExtractionResult(field.id, id, match.captured(0), confidence + 0.2));
        }
        // 尝试简单模式
        match = patterns_["employeeId_simple"].match(text);
        if (match.hasMatch()) {
            QString id = match.captured(1);
            results.append(ExtractionResult(field.id, id, match.captured(0), 0.6));
        }
    }
    else if (field.id == "phone") {
        QString phone = extractPhone(text);
        if (!phone.isEmpty()) {
            double confidence = calculateConfidence(text, field.keywords, phone);
            results.append(ExtractionResult(field.id, phone, phone, confidence + 0.3));
        }
    }
    else if (field.id == "email") {
        QString email = extractEmail(text);
        if (!email.isEmpty()) {
            double confidence = calculateConfidence(text, field.keywords, email);
            results.append(ExtractionResult(field.id, email, email, confidence + 0.4));
        }
    }
    else if (field.id == "reason" || field.id == "leaveReason") {
        // 请假原因提取：支持"因为/由于/原因是"等句式，或关键字后的短句
        QRegularExpression reasonRegex(R"((?:因为|由于|因|请假原因(?:是|为)?|原因(?:是|为)?)[:：]?\s*([^，。.!！]{2,60}))");
        QRegularExpressionMatch m = reasonRegex.match(text);
        if (m.hasMatch()) {
            QString reason = m.captured(1).trimmed();
            if (!reason.isEmpty()) {
                double confidence = 0.8; // 句式匹配较高置信度
                results.append(ExtractionResult(field.id, reason, m.captured(0), confidence));
            }
        } else {
            // 关键词附近提取
            for (const QString& keyword : QStringList{"原因", "请假", "说明"}) {
                int pos = text.indexOf(keyword);
                if (pos != -1) {
                    QString tail = text.mid(pos + keyword.length());
                    QRegularExpression r2(R"([:：]?\s*([^，。.!！]{2,60}))");
                    QRegularExpressionMatch m2 = r2.match(tail);
                    if (m2.hasMatch()) {
                        QString reason = m2.captured(1).trimmed();
                        if (!reason.isEmpty()) {
                            results.append(ExtractionResult(field.id, reason, keyword + m2.captured(0), 0.6));
                            break;
                        }
                    }
                }
            }
        }
    }
    else {
        // 通用关键词匹配
        results = extractWithKeywords(text, field);
    }
    
    return results;
}

QList<ExtractionResult> NLPProcessor::extractDateFields(const QString& text, const FormField& field)
{
    QList<ExtractionResult> results;
    
    // 检查特殊日期关键词
    for (auto it = dateFormats_.begin(); it != dateFormats_.end(); ++it) {
        if (text.contains(it.key())) {
            QDate date = QDate::fromString(it.value(), "yyyy-MM-dd");
            if (date.isValid()) {
                double confidence = calculateConfidence(text, field.keywords, it.key());
                results.append(ExtractionResult(field.id, date, it.key(), confidence + 0.3));
            }
        }
    }
    
    // 使用正则表达式提取日期
    for (const QRegularExpression& pattern : datePatterns_) {
        QRegularExpressionMatchIterator it = pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString dateStr = match.captured(0);
            QDate date = extractDate(dateStr);
            if (date.isValid()) {
                double confidence = calculateConfidence(text, field.keywords, dateStr);
                results.append(ExtractionResult(field.id, date, dateStr, confidence));
            }
        }
    }
    
    return results;
}

QList<ExtractionResult> NLPProcessor::extractSelectionFields(const QString& text, const FormField& field)
{
    QList<ExtractionResult> results;
    
    if (field.id == "department") {
        QString dept = extractDepartment(text);
        if (!dept.isEmpty()) {
            double confidence = calculateConfidence(text, field.keywords, dept);
            results.append(ExtractionResult(field.id, dept, dept, confidence + 0.2));
        }
    }
    else if (field.id == "leaveType") {
        QString leaveType = extractLeaveType(text);
        if (!leaveType.isEmpty()) {
            double confidence = calculateConfidence(text, field.keywords, leaveType);
            results.append(ExtractionResult(field.id, leaveType, leaveType, confidence + 0.3));
        }
    }
    else if (field.id == "gender") {
        if (text.contains("男") && !text.contains("男性")) {
            results.append(ExtractionResult(field.id, "男", "男", 0.8));
        } else if (text.contains("女") && !text.contains("女性")) {
            results.append(ExtractionResult(field.id, "女", "女", 0.8));
        }
    }
    else {
        // 检查选项列表
        for (const QString& option : field.options) {
            if (text.contains(option)) {
                double confidence = calculateConfidence(text, field.keywords, option);
                results.append(ExtractionResult(field.id, option, option, confidence));
            }
        }
    }
    
    return results;
}

QList<ExtractionResult> NLPProcessor::extractNumberFields(const QString& text, const FormField& field)
{
    QList<ExtractionResult> results;
    
    // 查找关键词附近的数字
    for (const QString& keyword : field.keywords) {
        int keywordPos = text.indexOf(keyword);
        if (keywordPos != -1) {
            // 在关键词前后搜索数字
            QString searchText = text.mid(std::max(0, keywordPos - MAX_EXTRACTION_DISTANCE), 
                                        MAX_EXTRACTION_DISTANCE * 2);
            
            QRegularExpressionMatchIterator it = patterns_["number"].globalMatch(searchText);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                bool ok;
                int number = match.captured(1).toInt(&ok);
                if (ok) {
                    double confidence = calculateConfidence(text, field.keywords, match.captured(1));
                    results.append(ExtractionResult(field.id, number, match.captured(0), confidence));
                }
            }
        }
    }
    
    return results;
}

QList<ExtractionResult> NLPProcessor::extractWithKeywords(const QString& text, const FormField& field)
{
    QList<ExtractionResult> results;
    
    // 在关键词附近查找值
    for (const QString& keyword : field.keywords) {
        int keywordPos = text.indexOf(keyword);
        if (keywordPos != -1) {
            // 查找关键词后的值（支持冒号、等号等分隔符）
            QRegularExpression valueRegex(QString("%1[:：=]\\s*([^\\s,，。！!]{1,50})").arg(QRegularExpression::escape(keyword)));
            QRegularExpressionMatch match = valueRegex.match(text);
            if (match.hasMatch()) {
                QString value = match.captured(1).trimmed();
                if (!value.isEmpty()) {
                    double confidence = calculateConfidence(text, field.keywords, value);
                    results.append(ExtractionResult(field.id, value, match.captured(0), confidence));
                }
            }
            
            // 查找关键词前的值（如"张三申请请假"）
            if (keywordPos > 0) {
                QString beforeText = text.left(keywordPos).trimmed();
                QStringList words = beforeText.split(QRegularExpression("\\s+"));
                if (!words.isEmpty()) {
                    QString lastWord = words.last();
                    if (lastWord.length() >= 2 && lastWord.length() <= 20) {
                        double confidence = calculateConfidence(text, field.keywords, lastWord);
                        results.append(ExtractionResult(field.id, lastWord, lastWord + keyword, confidence * 0.8));
                    }
                }
            }
        }
    }
    
    return results;
}

QList<ExtractionResult> NLPProcessor::extractWithPatterns(const QString& text, const FormField& field)
{
    QList<ExtractionResult> results;
    
    // 根据字段类型使用相应的模式
    QString patternKey = field.id;
    if (patterns_.contains(patternKey)) {
        QRegularExpressionMatchIterator it = patterns_[patternKey].globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString value = match.captured(1);
            if (!value.isEmpty()) {
                double confidence = calculateConfidence(text, field.keywords, value);
                results.append(ExtractionResult(field.id, value, match.captured(0), confidence));
            }
        }
    }
    
    return results;
}

QString NLPProcessor::extractName(const QString& text)
{
    qDebug() << "[NLP] 提取姓名，输入文本:" << text;
    
    // 1. 优先匹配明确的姓名表达（支持冒号）
    auto match = patterns_["name"].match(text);
    if (match.hasMatch()) {
        QString name = match.captured(1).trimmed();
        qDebug() << "[NLP] 通过姓名模式提取到:" << name;
        return name;
    }
    
    // 2. 尝试简单模式
    match = patterns_["name_simple"].match(text);
    if (match.hasMatch()) {
        QString name = match.captured(1).trimmed();
        qDebug() << "[NLP] 通过简单模式提取到:" << name;
        return name;
    }
    
    // 3. 增强的姓名识别模式
    QStringList namePatterns = {
        R"((?:我是|我叫|姓名是|名字是|叫做|申请人|员工)[:：]?\s*([\u4e00-\u9fff]{2,4}))",
        R"(([\u4e00-\u9fff]{2,4})(?:申请|请假|入职|员工|在|的|工作|来自))",
        R"(名称[:：]?\s*([\u4e00-\u9fff]{2,4}))",
        R"((?:工号|手机|部门)[:：]?\s*[\w\d\s]*[:：]?\s*([\u4e00-\u9fff]{2,4}))",
        R"(“?我是”?([\u4e00-\u9fff]{2,4}))"
    };
    
    for (const QString& pattern : namePatterns) {
        QRegularExpression regex(pattern);
        match = regex.match(text);
        if (match.hasMatch()) {
            QString name = match.captured(1).trimmed();
            if (isValidName(name)) {
                qDebug() << "[NLP] 通过增强模式提取到姓名:" << name;
                return name;
            }
        }
    }
    
    // 4. 兜底：简单的中文姓名识别（排除常见非姓名词汇）
    QRegularExpression chineseName("([\\u4e00-\\u9fff]{2,4})");
    match = chineseName.match(text);
    if (match.hasMatch()) {
        QString name = match.captured(1);
        if (isValidName(name)) {
            qDebug() << "[NLP] 通过中文模式提取到姓名:" << name;
            return name;
        }
    }
    
    // 5. 最终兜底：基于关键短语的手动扫描（容错引号、空格、标点）
    QStringList triggers = {"我是", "我叫", "姓名是", "名字是", "叫做"};
    for (const QString& trigger : triggers) {
        int pos = text.indexOf(trigger);
        if (pos != -1) {
            int start = pos + trigger.length();
            // 跳过空白与引号
            while (start < text.length() && (text[start].isSpace() || text[start] == '"' || text[start] == '\'')) {
                start++;
            }
            // 收集连续中文字符
            QString candidate;
            for (int i = start; i < text.length(); ++i) {
                QChar ch = text[i];
                if (ch.unicode() >= 0x4E00 && ch.unicode() <= 0x9FFF) {
                    candidate.append(ch);
                    if (candidate.length() == 4) break;
                } else {
                    break;
                }
            }
            if (isValidName(candidate)) {
                qDebug() << "[NLP] 通过手动扫描提取到姓名:" << candidate;
                return candidate;
            }
        }
    }
    
    qDebug() << "[NLP] 未能提取到姓名";
    return QString();
}

bool NLPProcessor::isValidName(const QString& name)
{
    if (name.length() < 2 || name.length() > 4) {
        return false;
    }
    
    // 排除常见非姓名词汇
    QStringList excludeWords = {
        "技术", "部门", "请假", "申请", "公司", "岗位", "员工", "经理", "主管", 
        "主任", "部长", "总监", "助理", "专员", "工程师", "设计师", "开发",
        "测试", "运维", "产品", "运营", "销售", "市场", "财务", "人事", "行政",
        "今天", "明天", "后天", "昨天", "前天", "开始", "结束", "时间", "日期",
        "原因", "理由", "说明", "备注", "其他", "等等", "包括", "需要", "想要"
    };
    
    return !excludeWords.contains(name);
}

QString NLPProcessor::extractPhone(const QString& text)
{
    auto match = patterns_["phone"].match(text);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    
    match = patterns_["phone_simple"].match(text);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    
    return QString();
}

QString NLPProcessor::extractEmail(const QString& text)
{
    auto match = patterns_["email"].match(text);
    if (match.hasMatch()) {
        return match.captured(1);
    }
    return QString();
}

QDate NLPProcessor::extractDate(const QString& text)
{
    // 尝试各种日期格式
    QStringList formats = {"yyyy年M月d日", "yyyy-MM-dd", "yyyy/MM/dd", "M月d日", "MM-dd"};
    
    for (const QString& format : formats) {
        QDate date = QDate::fromString(text, format);
        if (date.isValid()) {
            // 如果只有月日，使用当前年份
            if (format.contains("M月d日") || format.contains("MM-dd")) {
                date = QDate(QDate::currentDate().year(), date.month(), date.day());
            }
            return date;
        }
    }
    
    return QDate();
}

QString NLPProcessor::extractDepartment(const QString& text)
{
    for (const QString& dept : departments_) {
        if (text.contains(dept)) {
            return dept;
        }
    }
    return QString();
}

QString NLPProcessor::extractLeaveType(const QString& text)
{
    for (const QString& type : leaveTypes_) {
        if (text.contains(type)) {
            return type;
        }
    }
    return QString();
}

double NLPProcessor::calculateConfidence(const QString& text, const QStringList& keywords, const QString& extracted)
{
    if (extracted.isEmpty()) return 0.0;
    
    double confidence = 0.5; // 基础置信度
    
    // 关键词匹配加分
    for (const QString& keyword : keywords) {
        if (text.contains(keyword)) {
            confidence += 0.2;
        }
    }
    
    // 长度合理性
    if (extracted.length() >= 2 && extracted.length() <= 50) {
        confidence += 0.1;
    }
    
    return std::min(1.0, confidence);
}

QString NLPProcessor::cleanText(const QString& text)
{
    QString cleaned = text;
    // 移除多余的空白字符
    cleaned = cleaned.simplified();
    // 统一标点符号
    cleaned.replace("，", ",")
           .replace("。", ".")
           .replace("：", ":")
           .replace(QChar(0x201C), '"')   // “ -> "
           .replace(QChar(0x201D), '"')   // ” -> "
           .replace(QChar(0x2018), '\'')  // ‘ -> '
           .replace(QChar(0x2019), '\''); // ’ -> '
    return cleaned;
}

// ===== AutoFormFiller 实现 =====

AutoFormFiller::AutoFormFiller()
    : minConfidence_(0.2)  // 降低置信度阈值，提高识别率
{
}

FormTemplate* AutoFormFiller::processUserInput(const QString& userInput, QString& responseMessage)
{
    if (userInput.trimmed().isEmpty()) {
        responseMessage = "请提供具体的信息，我将帮您自动填写表单。\n\n"
                         "例如：\"我叫张三，工号是A001，想申请明天的病假\"";
        return nullptr;
    }
    
    // 使用NLP处理器自动填写表单
    FormTemplate* form = processor_.autoFillForm(userInput);
    
    if (!form) {
        responseMessage = "抱歉，我无法从您的描述中识别出具体的表单类型。\n\n"
                         "请尝试更明确的描述，例如：\n"
                         "• 员工信息：\"我是新员工张三，工号A001，在技术部工作\"\n"
                         "• 请假申请：\"我要请假，从明天开始请3天病假\"";
        return nullptr;
    }
    
    // 生成响应消息
    FormRecognitionResult recognition = processor_.recognizeFormType(userInput);
    responseMessage = generateResponseMessage(recognition, form);
    
    return form;
}

QString AutoFormFiller::generateResponseMessage(const FormRecognitionResult& result, FormTemplate* form)
{
    if (!form) return "表单生成失败";
    
    QString message = QString("已识别为：%1\n\n").arg(result.formName);
    
    // 显示已填写的字段
    message += "已自动填写以下信息：\n";
    QMap<QString, QVariant> values = form->getAllValues();
    QList<FormField> fields = form->getFields();
    
    // 调试：输出表单详细信息
    qDebug() << "[AutoFormFiller] 表单名称:" << form->getName();
    qDebug() << "[AutoFormFiller] 表单字段总数:" << fields.size();
    qDebug() << "[AutoFormFiller] 表单值总数:" << values.size();
    
    int filledCount = 0;
    for (const FormField& field : fields) {
        bool hasValue = values.contains(field.id);
        QString valueStr = hasValue ? values[field.id].toString() : "空";
        bool isEmpty = valueStr.isEmpty();
        
        qDebug() << "[AutoFormFiller] 字段" << field.id << "(" << field.label << "):"
                 << "包含=" << hasValue << ", 值=" << valueStr << ", 为空=" << isEmpty;
        
        if (hasValue && !isEmpty) {
            QString displayValue = formatFieldValue(field, values[field.id]);
            message += QString("• %1：%2\n").arg(field.label, displayValue);
            filledCount++;
        }
    }
    
    if (filledCount == 0) {
        message += "暂无信息被自动识别\n";
        qDebug() << "[AutoFormFiller] 警告：没有任何字段被填充!";
    }
    
    // 显示缺失的必填字段
    QString missingHint = generateMissingFieldsHint(form);
    if (!missingHint.isEmpty()) {
        message += "\n还需要以下必填信息：\n" + missingHint;
        message += "\n您可以继续输入补充信息，我会自动更新表单。";
    } else {
        message += "\n所有必填信息已完整，可以导出表单了！";
    }
    
    message += "\n\n可用操作：\n";
    message += "• 继续输入信息补充表单\n";
    message += "• 输入\"导出表单\"生成文件\n";
    message += "• 输入\"查看表单\"显示完整内容";
    
    return message;
}

QString AutoFormFiller::formatFieldValue(const FormField& field, const QVariant& value)
{
    switch (field.type) {
    case FieldType::Date:
        return value.toDate().toString("yyyy年MM月dd日");
    case FieldType::Number:
        return QString::number(value.toInt());
    default:
        return value.toString();
    }
}

QString AutoFormFiller::generateMissingFieldsHint(FormTemplate* form)
{
    QStringList missing;
    QMap<QString, QVariant> values = form->getAllValues();
    QList<FormField> fields = form->getFields();
    
    for (const FormField& field : fields) {
        if (field.required && (!values.contains(field.id) || values[field.id].toString().isEmpty())) {
            missing.append(QString("• %1").arg(field.label));
        }
    }
    
    return missing.join("\n");
}
