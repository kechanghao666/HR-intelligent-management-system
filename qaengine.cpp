#include "qaengine.h"
#include "documentprocessor.h"
#include <QDebug>
#include <QRegularExpression>
#include <QStringList>
#include <QDateTime>
#include <QtMath>

QAEngine::QAEngine(Database* database, QObject *parent)
    : QObject(parent)
    , database_(database)
    , documentProcessor_(nullptr)
    , aiEnabled_(true) // 默认启用AI
{
    initializeKeywords();
    loadPresetQA();
    
    // 初始化AI API
    xfyunAPI_ = new XfyunAPI(this);
    connect(xfyunAPI_, &XfyunAPI::answerReceived, this, &QAEngine::onAIAnswerReceived);

}

QAEngine::QAEngine(Database* database, DocumentProcessor* docProcessor, QObject *parent)
    : QObject(parent)
    , database_(database)
    , documentProcessor_(docProcessor)
    , aiEnabled_(true) // 默认启用AI
{
    initializeKeywords();
    loadPresetQA();
    
    // 初始化AI API
    xfyunAPI_ = new XfyunAPI(this);
    connect(xfyunAPI_, &XfyunAPI::answerReceived, this, &QAEngine::onAIAnswerReceived);

}

void QAEngine::setDocumentProcessor(DocumentProcessor* docProcessor)
{
    documentProcessor_ = docProcessor;
}

QAResult QAEngine::askQuestion(const QString& question)
{
    // 验证输入问题
    if (question.trimmed().isEmpty()) {
        QAResult result;
        result.answer = "请输入您的问题。";
        result.category = "系统提示";
        result.confidence = "高";
        return result;
    }
    
    QString trimmedQuestion = question.trimmed();
    QString category = categorizeQuestion(trimmedQuestion);
    QAResult result = searchInKnowledgeBase(trimmedQuestion, category);
    
    // 记录问答历史（只记录有效问题）
    if (!result.answer.isEmpty() && result.category != "系统提示") {
        QuestionRecord record("", trimmedQuestion, result.answer, result.category);
        database_->addQuestionRecord(record);
    }
    
    return result;
}

QAResult QAEngine::askAttendanceQuestion(const QString& question)
{
    return searchInKnowledgeBase(question, "考勤");
}

QAResult QAEngine::askSalaryQuestion(const QString& question)
{
    return searchInKnowledgeBase(question, "薪酬");
}

QAResult QAEngine::askPerformanceQuestion(const QString& question)
{
    return searchInKnowledgeBase(question, "绩效");
}

QAResult QAEngine::askOnboardingQuestion(const QString& question)
{
    return searchInKnowledgeBase(question, "入职离职");
}

QAResult QAEngine::askBenefitsQuestion(const QString& question)
{
    return searchInKnowledgeBase(question, "员工福利");
}

QList<Document> QAEngine::searchDocuments(const QString& keyword)
{
    QList<Document> allDocs = database_->getAllDocuments();
    QList<Document> results;
    
    for (const Document& doc : allDocs) {
        if (doc.getTitle().contains(keyword, Qt::CaseInsensitive) ||
            doc.getContent().contains(keyword, Qt::CaseInsensitive)) {
            results.append(doc);
        }
    }
    
    return results;
}

Document QAEngine::findRelatedDocument(const QString& question, const QString& category)
{
    QList<Document> docs = database_->getDocumentsByCategory(category);
    
    if (!docs.isEmpty()) {
        // 简单的关键词匹配
        QString keywords = extractKeywords(question);
        for (const Document& doc : docs) {
            if (doc.getContent().contains(keywords, Qt::CaseInsensitive)) {
                return doc;
            }
        }
        // 如果没有精确匹配，返回该分类的第一个文档
        return docs.first();
    }
    
    return Document();
}

void QAEngine::updateKnowledgeBase()
{
    // 从数据库加载最新的问答记录
    QList<QuestionRecord> records = database_->getQuestionRecords();
    
    for (const QuestionRecord& record : records) {
        addCustomQA(record.getQuestion(), record.getAnswer(), record.getCategory());
    }
}

void QAEngine::addCustomQA(const QString& question, const QString& answer, const QString& category)
{
    if (!knowledgeBase_.contains(category)) {
        knowledgeBase_[category] = QList<QPair<QString, QString>>();
    }
    
    knowledgeBase_[category].append(qMakePair(question, answer));
}

void QAEngine::initializeKeywords()
{
    // 考勤关键词
    attendanceKeywords_["考勤"] = QStringList{"考勤", "打卡", "上班", "下班", "工作时间", "迟到", "早退"};
    attendanceKeywords_["请假"] = QStringList{"请假", "病假", "事假", "年假", "休假", "假期"};
    attendanceKeywords_["加班"] = QStringList{"加班", "超时", "加班费", "调休"};
    
    // 薪酬关键词
    salaryKeywords_["薪资"] = QStringList{"薪资", "工资", "薪水", "收入", "基本工资"};
    salaryKeywords_["社保"] = QStringList{"社保", "社会保险", "医保", "养老保险", "失业保险", "工伤保险"};
    salaryKeywords_["公积金"] = QStringList{"公积金", "住房公积金", "公积金贷款"};
    salaryKeywords_["个税"] = QStringList{"个税", "个人所得税", "税收", "扣税"};
    
    // 绩效关键词
    performanceKeywords_["考核"] = QStringList{"考核", "绩效", "评估", "考评", "表现"};
    performanceKeywords_["等级"] = QStringList{"等级", "评分", "优秀", "良好", "合格", "不合格"};
    performanceKeywords_["面谈"] = QStringList{"面谈", "沟通", "反馈", "改进"};
    
    // 入职离职关键词
    onboardingKeywords_["入职"] = QStringList{"入职", "新员工", "入职材料", "入职流程"};
    onboardingKeywords_["试用期"] = QStringList{"试用期", "转正", "试用", "实习"};
    onboardingKeywords_["离职"] = QStringList{"离职", "辞职", "手续", "交接"};
    
    // 员工福利关键词
    benefitsKeywords_["年假"] = QStringList{"年假", "年休假", "带薪假期"};
    benefitsKeywords_["体检"] = QStringList{"体检", "健康检查", "医疗"};
    benefitsKeywords_["福利"] = QStringList{"福利", "补贴", "津贴", "奖金", "节日"};
}

QString QAEngine::categorizeQuestion(const QString& question)
{
    QString lowerQuestion = question.toLower();
    
    // 检查考勤相关关键词
    for (auto it = attendanceKeywords_.begin(); it != attendanceKeywords_.end(); ++it) {
        for (const QString& keyword : it.value()) {
            if (lowerQuestion.contains(keyword)) {
                return "考勤";
            }
        }
    }
    
    // 检查薪酬相关关键词
    for (auto it = salaryKeywords_.begin(); it != salaryKeywords_.end(); ++it) {
        for (const QString& keyword : it.value()) {
            if (lowerQuestion.contains(keyword)) {
                return "薪酬";
            }
        }
    }
    
    // 检查绩效相关关键词
    for (auto it = performanceKeywords_.begin(); it != performanceKeywords_.end(); ++it) {
        for (const QString& keyword : it.value()) {
            if (lowerQuestion.contains(keyword)) {
                return "绩效";
            }
        }
    }
    
    // 检查入职离职相关关键词
    for (auto it = onboardingKeywords_.begin(); it != onboardingKeywords_.end(); ++it) {
        for (const QString& keyword : it.value()) {
            if (lowerQuestion.contains(keyword)) {
                return "入职离职";
            }
        }
    }
    
    // 检查员工福利相关关键词
    for (auto it = benefitsKeywords_.begin(); it != benefitsKeywords_.end(); ++it) {
        for (const QString& keyword : it.value()) {
            if (lowerQuestion.contains(keyword)) {
                return "员工福利";
            }
        }
    }
    
    return "通用";
}

QAResult QAEngine::searchInKnowledgeBase(const QString& question, const QString& category)
{
    QAResult result;
    result.category = category;
    result.confidence = "中等";
    
    if (knowledgeBase_.contains(category)) {
        QString bestAnswer = findBestMatch(question, knowledgeBase_[category]);
        if (!bestAnswer.isEmpty()) {
            result.answer = bestAnswer;
            result.confidence = "高";
        }
    }
    
    // 如果没有找到匹配的答案，尝试从文档中提取答案
    if (result.answer.isEmpty()) {
        // 首先尝试使用DocumentProcessor查找相关文档块
        if (documentProcessor_) {
            QList<DocumentChunk> relevantChunks = documentProcessor_->findRelevantChunks(question, 3);
            if (!relevantChunks.isEmpty()) {
                QString bestAnswer;
                double bestScore = 0.0;
                
                for (const DocumentChunk& chunk : relevantChunks) {
                    double similarity = calculateSimilarity(question, chunk.content);
                    if (similarity > bestScore) {
                        bestScore = similarity;
                        bestAnswer = chunk.content;
                        result.sourceDocument = chunk.title;
                    }
                }
                
                if (bestScore > 0.3) {  // 相似度阈值
                    result.answer = QString("根据相关文档：\n\n%1").arg(bestAnswer);
                    result.confidence = bestScore > 0.7 ? "高" : "中";
                } else {
                    result.answer = "根据已上传的文档，我找到了一些相关信息，但可能不完全匹配您的问题。请查看相关制度文档获取详细信息。";
                    result.confidence = "低";
                }
            } else {
                // 如果DocumentProcessor没有找到，回退到原来的方法
                QList<Document> relatedDocs = findRelatedDocuments(question, category);
                if (!relatedDocs.isEmpty()) {
                    QString extractedAnswer = extractAnswerFromDocuments(question, relatedDocs);
                    if (!extractedAnswer.isEmpty()) {
                        result.answer = extractedAnswer;
                        result.confidence = "中";
                    } else {
                        result.answer = "抱歉，我暂时无法回答这个问题。请联系HR部门获取更详细的信息，或查看相关制度文档。";
                        result.confidence = "低";
                    }
                } else {
                    result.answer = "抱歉，我暂时无法回答这个问题。请联系HR部门获取更详细的信息，或查看相关制度文档。";
                    result.confidence = "低";
                }
            }
        } else {
            // DocumentProcessor不可用时的回退逻辑
            QList<Document> relatedDocs = findRelatedDocuments(question, category);
            if (!relatedDocs.isEmpty()) {
                QString extractedAnswer = extractAnswerFromDocuments(question, relatedDocs);
                if (!extractedAnswer.isEmpty()) {
                    result.answer = extractedAnswer;
                    result.confidence = "中";
                } else {
                    result.answer = "抱歉，我暂时无法回答这个问题。请联系HR部门获取更详细的信息，或查看相关制度文档。";
                    result.confidence = "低";
                }
            } else {
                result.answer = "抱歉，我暂时无法回答这个问题。请联系HR部门获取更详细的信息，或查看相关制度文档。";
                result.confidence = "低";
            }
        }
    }
    
    // 查找相关文档
    result.relatedDocuments = findRelatedDocuments(question, category);
    
    return result;
}

QString QAEngine::findBestMatch(const QString& question, const QList<QPair<QString, QString>>& qaList)
{
    double bestSimilarity = 0.0;
    QString bestAnswer = "";
    
    for (const auto& qa : qaList) {
        double similarity = calculateSimilarity(question, qa.first);
        if (similarity > bestSimilarity) {
            bestSimilarity = similarity;
            bestAnswer = qa.second;
        }
    }
    
    // 只有相似度超过阈值才返回答案
    if (bestSimilarity > 0.2) {
        return bestAnswer;
    }
    
    return "";
}

double QAEngine::calculateDocumentRelevance(const QString& question, const Document& document)
{
    double score = 0.0;
    QString questionLower = question.toLower();
    QString titleLower = document.getTitle().toLower();
    QString contentLower = document.getContent().toLower();
    
    // 标题匹配得分（权重更高）
    QStringList questionWords = questionLower.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    for (const QString& word : questionWords) {
        if (word.length() > 1) {
            if (titleLower.contains(word)) {
                score += 3.0; // 标题匹配权重更高
            }
            int contentCount = contentLower.count(word);
            score += contentCount * 0.1; // 内容匹配权重
        }
    }
    
    // 特殊关键词匹配
    QStringList specialKeywords = {"工作时间", "请假", "加班", "工资", "社保", "公积金", "年假", "病假", "事假", "入职", "离职", "试用期", "转正", "绩效", "考核"};
    for (const QString& keyword : specialKeywords) {
        if (questionLower.contains(keyword) && contentLower.contains(keyword)) {
            score += 2.0; // 特殊关键词匹配加分
        }
    }
    
    return score;
}

double QAEngine::calculateSimilarity(const QString& str1, const QString& str2)
{
    // 简单的字符串相似度计算（基于共同字符数）
    QStringList words1 = str1.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QStringList words2 = str2.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    
    int commonWords = 0;
    for (const QString& word1 : words1) {
        for (const QString& word2 : words2) {
            if (word1.contains(word2, Qt::CaseInsensitive) || 
                word2.contains(word1, Qt::CaseInsensitive)) {
                commonWords++;
                break;
            }
        }
    }
    
    int totalWords = qMax(words1.size(), words2.size());
    return totalWords > 0 ? static_cast<double>(commonWords) / totalWords : 0.0;
}

QString QAEngine::extractKeywords(const QString& text)
{
    // 简单的关键词提取（去除常用词）
    QStringList stopWords = {"的", "是", "在", "有", "和", "与", "或", "但", "如何", "什么", "怎么", "为什么"};
    QStringList words = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
    QStringList keywords;
    
    for (const QString& word : words) {
        if (!stopWords.contains(word, Qt::CaseInsensitive) && word.length() > 1) {
            keywords.append(word);
        }
    }
    
    return keywords.join(" ");
}

QList<Document> QAEngine::findRelatedDocuments(const QString& question, const QString& category)
{
    QList<Document> documents;
    
    // 首先查找同类别的文档
    QList<Document> categoryDocs = database_->getDocumentsByCategory(category);
    documents.append(categoryDocs);
    
    // 然后基于关键词查找其他相关文档
    QString keywords = extractKeywords(question);
    QList<Document> keywordDocs = searchDocuments(keywords);
    
    for (const Document& doc : keywordDocs) {
        if (!documents.contains(doc)) {
            documents.append(doc);
        }
    }
    
    // 如果没有找到相关文档，尝试查找通用类别的文档（如HR制度文档）
    if (documents.isEmpty()) {
        QList<Document> generalDocs = database_->getDocumentsByCategory("通用");
        for (const Document& doc : generalDocs) {
            // 检查文档内容是否包含问题中的关键词
            QString questionLower = question.toLower();
            QString contentLower = doc.getContent().toLower();
            
            QStringList questionWords = questionLower.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);
            bool hasMatch = false;
            
            for (const QString& word : questionWords) {
                if (word.length() > 1 && contentLower.contains(word)) {
                    hasMatch = true;
                    break;
                }
            }
            
            if (hasMatch) {
                documents.append(doc);
            }
        }
    }
    
    // 如果仍然没有找到，返回所有文档（作为最后的备选）
    if (documents.isEmpty()) {
        documents = database_->getAllDocuments();
    }
    
    // 按相关性排序文档
    QList<QPair<Document, double>> scoredDocs;
    for (const Document& doc : documents) {
        double score = calculateDocumentRelevance(question, doc);
        scoredDocs.append(qMakePair(doc, score));
    }
    
    // 按分数降序排序
    std::sort(scoredDocs.begin(), scoredDocs.end(), 
        [](const QPair<Document, double>& a, const QPair<Document, double>& b) {
            return a.second > b.second;
        });
    
    // 返回排序后的文档
    documents.clear();
    for (const auto& pair : scoredDocs) {
        documents.append(pair.first);
    }
    
    return documents;
}

void QAEngine::loadPresetQA()
{
    loadAttendanceQA();
    loadSalaryQA();
    loadPerformanceQA();
    loadOnboardingQA();
    loadBenefitsQA();
}

void QAEngine::loadAttendanceQA()
{
    QList<QPair<QString, QString>> attendanceQA;
    
    attendanceQA.append(qMakePair("公司的工作时间是什么？", 
        "公司实行标准工时制：核心工作时间为每日10:00-16:00，弹性工作时间为8:30-10:00及16:00-19:30，员工每日累计工作时间不超过8小时，每周不超过44小时。公司实行'3+2'混合办公制度，即每周一、二、四为公司办公日，周三、周五为灵活办公日。"));
    
    attendanceQA.append(qMakePair("如何请假？", 
        "员工请假需提前申请，经主管批准后方可休假。病假需提供医院证明，事假需说明具体原因。"));
    
    attendanceQA.append(qMakePair("请假需要提前多久申请？", 
        "年假需提前3个工作日提交申请，当年未休年假可结转至次年3月31日前使用。事假需提前申请，每月不超过3天，全年累计不超过15天。病假需提供二级以上医院开具的诊断证明。"));
    
    attendanceQA.append(qMakePair("迟到早退怎么处理？", 
        "迟到/早退15分钟以内，每月累计不超过3次的，不予处罚；迟到/早退15-30分钟，每次扣减当日半小时工资；迟到/早退30分钟以上，视为旷工半天。"));
    
    attendanceQA.append(qMakePair("加班时间如何计算？", 
        "员工因工作需要确需加班的，需提前在系统提交加班申请，经部门负责人批准后方可认定为加班。工作日加班：按不低于工资的150%支付加班费；休息日加班：优先安排补休，不能补休的按不低于工资的200%支付加班费；法定节假日加班：按不低于工资的300%支付加班费。"));
    
    attendanceQA.append(qMakePair("加班费如何计算？", 
        "加班费计算基数：以劳动合同约定的月工资为基准，无约定的按正常出勤工资的70%确定，但不得低于秦皇岛市最低工资标准。日工资计算：月工资收入÷21.75天；小时工资计算：日工资÷8小时。"));
    
    attendanceQA.append(qMakePair("考勤制度有哪些？", 
        "公司实行打卡考勤制度，员工需按时上下班打卡。迟到、早退、旷工等行为将影响绩效考核和薪资待遇。"));
    
    knowledgeBase_["考勤"] = attendanceQA;
}

void QAEngine::loadSalaryQA()
{
    QList<QPair<QString, QString>> salaryQA;
    
    salaryQA.append(qMakePair("薪资构成包括什么？", 
        "员工薪酬由基本工资、绩效工资、津贴补贴等组成。基本工资根据岗位等级和员工能力确定。"));
    
    salaryQA.append(qMakePair("社保缴费比例", 
        "公司按国家规定为员工缴纳养老保险、医疗保险、失业保险、工伤保险和生育保险。具体缴费比例按照国家最新规定执行。"));
    
    salaryQA.append(qMakePair("社保缴费比例是多少？", 
        "公司按国家规定为员工缴纳养老保险、医疗保险、失业保险、工伤保险和生育保险。具体缴费比例按照国家最新规定执行。"));
    
    salaryQA.append(qMakePair("公积金缴费比例是多少？", 
        "住房公积金：按秦皇岛市规定的比例为员工缴纳住房公积金。"));
    
    salaryQA.append(qMakePair("个人所得税如何计算？", 
        "个人所得税按照国家最新规定计算和缴纳，公司代扣代缴个人所得税。"));
    
    salaryQA.append(qMakePair("工资什么时候发放？", 
        "公司每月10日发放上月工资，遇节假日提前至最近的工作日。"));
    
    salaryQA.append(qMakePair("绩效工资如何计算？", 
        "绩效工资占薪酬总额的30%，根据绩效考核结果发放。绩效工资发放比例：S级150%，A级130%，B级120%，C级110%，D级100%，E级80%，F级60%，G级50%。"));
    
    knowledgeBase_["薪酬"] = salaryQA;
}

void QAEngine::loadPerformanceQA()
{
    QList<QPair<QString, QString>> performanceQA;
    
    performanceQA.append(qMakePair("绩效考核周期是什么？", 
        "绩效考核分为月度考核、季度考核和年度考核。月度考核于次月5日前完成，季度考核于次季度首月10日前完成。"));
    
    performanceQA.append(qMakePair("绩效等级如何划分？", 
        "绩效等级分为：优秀(A)、良好(B)、合格(C)、不合格(D)。优秀比例不超过20%，良好比例不超过40%。"));
    
    performanceQA.append(qMakePair("绩效面谈包括什么内容？", 
        "考核结果需与员工进行面谈沟通。面谈内容包括成绩总结、问题分析、改进计划等。"));
    
    performanceQA.append(qMakePair("绩效结果如何运用？", 
        "绩效结果与员工晋升、薪资调整、培训机会等直接挂钩，是人才发展的重要依据。"));
    
    knowledgeBase_["绩效"] = performanceQA;
}

void QAEngine::loadOnboardingQA()
{
    QList<QPair<QString, QString>> onboardingQA;
    
    onboardingQA.append(qMakePair("入职需要准备什么材料？", 
        "新员工需提供：1.身份证原件及复印件 2.学历证书原件及复印件 3.离职证明 4.体检报告 5.一寸免冠照片4张"));
    
    onboardingQA.append(qMakePair("入职需要什么材料？", 
        "新员工需提供：1.身份证原件及复印件 2.学历证书原件及复印件 3.离职证明 4.体检报告 5.一寸免冠照片4张"));
    
    onboardingQA.append(qMakePair("试用期多长时间？", 
        "新员工试用期为3个月。试用期内表现优秀者可提前转正。"));
    
    onboardingQA.append(qMakePair("入职流程是什么？", 
        "入职流程包括：1.材料审核 2.签订劳动合同 3.办理工作证 4.安排工位 5.入职培训"));
    
    onboardingQA.append(qMakePair("离职手续如何办理？", 
        "离职需要提前30天提交书面申请，完成工作交接，办理各项手续，归还公司物品。"));
    
    onboardingQA.append(qMakePair("转正流程是什么？", 
        "试用期满后，员工需提交转正申请，经部门主管评估、HR审核、公司领导审批后正式转正。"));
    
    knowledgeBase_["入职离职"] = onboardingQA;
}

void QAEngine::loadBenefitsQA()
{
    QList<QPair<QString, QString>> benefitsQA;
    
    benefitsQA.append(qMakePair("年假天数如何计算？", 
        "员工工作满1年可享受5天年假。工作满10年可享受10天年假，满20年可享受15天年假。"));
    
    benefitsQA.append(qMakePair("公司提供哪些体检服务？", 
        "公司每年为员工安排一次免费体检。体检项目包括常规体检和专项检查。"));
    
    benefitsQA.append(qMakePair("有什么节日福利？", 
        "公司逢重大节日发放节日福利。福利标准根据公司经营状况确定。"));
    
    benefitsQA.append(qMakePair("其他福利待遇有哪些？", 
        "员工可享受生日福利、结婚福利、生育福利等。具体标准详见员工福利制度。"));
    
    knowledgeBase_["员工福利"] = benefitsQA;
}

QString QAEngine::extractAnswerFromDocuments(const QString& question, const QList<Document>& documents)
{
    if (documents.isEmpty()) {
        return "";
    }
    
    // 提取问题中的关键词
    QStringList questionKeywords = extractKeywords(question).split(" ", Qt::SkipEmptyParts);
    
    QString bestAnswer = "";
    double bestScore = 0.0;
    
    for (const Document& doc : documents) {
        QString content = doc.getContent();
        QString title = doc.getTitle();
        
        // 计算文档与问题的相关性得分
        double score = 0.0;
        
        // 标题匹配得分
        for (const QString& keyword : questionKeywords) {
            if (title.contains(keyword, Qt::CaseInsensitive)) {
                score += 2.0; // 标题匹配权重更高
            }
        }
        
        // 内容匹配得分
        for (const QString& keyword : questionKeywords) {
            int count = content.count(keyword, Qt::CaseInsensitive);
            score += count * 0.1;
        }
        
        // 如果得分更高，更新最佳答案
        if (score > bestScore) {
            bestScore = score;
            
            // 从文档内容中提取相关段落作为答案
            QString extractedAnswer = extractRelevantParagraph(content, questionKeywords);
            if (!extractedAnswer.isEmpty()) {
                bestAnswer = extractedAnswer;
            } else {
                // 如果没有找到相关段落，使用文档标题作为答案
                bestAnswer = QString("根据《%1》的相关规定，请查看具体内容。").arg(title);
            }
        }
    }
    
    return bestAnswer;
}

QString QAEngine::extractRelevantParagraph(const QString& content, const QStringList& keywords)
{
    // 按段落分割内容
    QStringList paragraphs = content.split(QRegularExpression("\\n\\s*\\n"), Qt::SkipEmptyParts);
    
    QString bestParagraph = "";
    double bestScore = 0.0;
    
    for (const QString& paragraph : paragraphs) {
        if (paragraph.trimmed().isEmpty()) {
            continue;
        }
        
        double score = 0.0;
        for (const QString& keyword : keywords) {
            int count = paragraph.count(keyword, Qt::CaseInsensitive);
            score += count;
        }
        
        if (score > bestScore && paragraph.length() > 20) { // 确保段落有足够长度
            bestScore = score;
            bestParagraph = paragraph.trimmed();
        }
    }
    
    // 如果找到相关段落，限制长度并返回
    if (!bestParagraph.isEmpty()) {
        if (bestParagraph.length() > 300) {
            bestParagraph = bestParagraph.left(300) + "...";
        }
        return bestParagraph;
    }
    
    return "";
}

// AI相关方法实现
void QAEngine::askQuestionWithAI(const QString& question)
{
    if (!aiEnabled_ || !xfyunAPI_) {
        // 如果AI未启用，只返回传统答案
        QAResult result = askQuestion(question);
        result.isAIGenerated = false;
        emit aiAnswerReceived(question, result);
        return;
    }
    
    // 保存当前问题和传统答案结果
    currentQuestion_ = question.trimmed();
    currentTraditionalResult_ = askQuestion(question);
    
    // 构建AI上下文
    QString aiContext = buildContextForAI(question, currentTraditionalResult_);
    
    // 发送给AI
    xfyunAPI_->askQuestion(aiContext);
}

void QAEngine::enableAI(bool enable)
{
    aiEnabled_ = enable;
}

bool QAEngine::isAIEnabled() const
{
    return aiEnabled_;
}

void QAEngine::onAIAnswerReceived(const QString& answer)
{
    if (currentQuestion_.isEmpty()) {
        qWarning() << "Received AI answer but no current question set";
        return;
    }
    
    QAResult finalResult;
    
    // 检查AI答案是否有效
    if (answer.trimmed().isEmpty()) {
        // AI答案为空，使用传统答案
        finalResult = currentTraditionalResult_;
        finalResult.isAIGenerated = false;
        finalResult.aiAnswer = "";
        finalResult.traditionalAnswer = finalResult.answer;
        finalResult.answer = QString("AI服务暂时不可用，为您提供传统答案：\n\n%1").arg(finalResult.answer);
    } else {
        // AI答案有效，合并传统和AI答案
        finalResult = combineTraditionalAndAIAnswers(currentTraditionalResult_, answer);
    }
    
    emit aiAnswerReceived(currentQuestion_, finalResult);
    
    // 清空当前状态
    currentQuestion_.clear();
    currentTraditionalResult_ = QAResult(); // 重置为默认状态
}



QAResult QAEngine::combineTraditionalAndAIAnswers(const QAResult& traditionalResult, const QString& aiAnswer)
{
    QAResult result = traditionalResult;
    result.isAIGenerated = true;
    result.aiAnswer = aiAnswer.trimmed();
    result.traditionalAnswer = traditionalResult.answer;
    
    // 决定主要答案来源
    if (!aiAnswer.trimmed().isEmpty()) {
        // 如果有AI答案，优先使用AI答案
        result.answer = QString("AI智能回答：\n%1\n\n系统知识库：\n%2")
                       .arg(aiAnswer.trimmed())
                       .arg(traditionalResult.answer);
        result.confidence = "AI增强";
    } else {
        // 如果AI答案为空，使用传统答案
        result.answer = traditionalResult.answer;
        result.isAIGenerated = false;
    }
    
    return result;
}

QString QAEngine::buildContextForAI(const QString& question, const QAResult& traditionalResult)
{
    QString context = QString("请回答以下HR相关问题，要求答案专业、准确、具体：\n\n问题：%1\n\n").arg(question);
    
    // 如果有传统答案，提供作为参考
    if (!traditionalResult.answer.isEmpty() && 
        traditionalResult.answer != "抱歉，我暂时无法回答这个问题。请联系HR部门获取更详细的信息，或查看相关制度文档。") {
        context += QString("参考信息：%1\n\n").arg(traditionalResult.answer);
    }
    
    // 如果有相关文档，提供文档信息
    if (!traditionalResult.relatedDocuments.isEmpty()) {
        context += "相关制度文档：\n";
        for (int i = 0; i < qMin(3, traditionalResult.relatedDocuments.size()); ++i) {
            const Document& doc = traditionalResult.relatedDocuments[i];
            context += QString("- %1\n").arg(doc.getTitle());
        }
        context += "\n";
    }
    
    context += "请基于以上信息给出专业的HR回答，如果参考信息不足，请基于常见的HR管理最佳实践给出建议。";
    
    return context;
}
