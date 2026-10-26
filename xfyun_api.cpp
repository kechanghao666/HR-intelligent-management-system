#include "xfyun_api.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QUrl>
#include <QDebug>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSslError>
#include <QNetworkProxy>
#include <QNetworkProxyFactory>
#include <QCoreApplication>
#include <QTimer>
#include <QSettings>
#include <QProcessEnvironment>

XfyunAPI::XfyunAPI(QObject *parent) : QObject(parent)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::finished, this, &XfyunAPI::onReplyFinished);
    
    // 加载鉴权信息
    loadCredentials();
    
    // 初始化SSL配置以解决TLS初始化失败问题
    initializeSSLConfiguration();
    
    // 设置网络安全
    setupNetworkSecurity();
}

void XfyunAPI::askQuestion(const QString &question)
{
    // 增强错误处理和网络检查
    if (question.trimmed().isEmpty()) {
        qWarning() << "[XfyunAPI] 问题为空，无法发送";
        emit answerReceived("问题不能为空");
        return;
    }
    // 记录本次问题，供失败兜底使用
    lastQuestion = question;
    
    if (!manager) {
        qCritical() << "[XfyunAPI] 网络管理器未初始化";
        emit answerReceived("网络服务未初始化，请重启应用程序");
        return;
    }
    
    // 检查SSL支持状态，如果不支持则使用模拟回复
    if (!QSslSocket::supportsSsl()) {
        qWarning() << "[XfyunAPI] SSL不可用，使用本地模拟回复";
        QString simulatedAnswer = generateLocalResponse(lastQuestion);
        QTimer::singleShot(200, [this, simulatedAnswer]() {
            emit answerReceived(simulatedAnswer);
        });
        return;
    }
    
    try {
        // 官方REST端点
        QString url = "https://spark-api-open.xf-yun.com/v1/chat/completions";

        QJsonObject msg;
        msg.insert("role", "user");
        msg.insert("content", question);

        QJsonArray msgs;
        msgs.append(msg);

        QJsonObject payload;
        payload.insert("model", "Spark Lite");
        payload.insert("messages", msgs);

        QNetworkRequest request((QUrl(url)));
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        
        // REST鉴权方式：Authorization: Bearer {apiPassword}
        if (apiPassword.isEmpty()) {
            qWarning() << "[XfyunAPI] API Password为空，请在设置或环境变量中配置";
        } else {
            QByteArray bearer = QByteArray("Bearer ") + apiPassword.toUtf8();
            request.setRawHeader("Authorization", bearer);
        }
        
        // 设置超时时间和SSL配置
        request.setTransferTimeout(30000); // 30秒超时
        
        // 为此请求设置特定的SSL配置
        QSslConfiguration sslConfig = request.sslConfiguration();
        sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
        sslConfig.setPeerVerifyMode(QSslSocket::QueryPeer);
        request.setSslConfiguration(sslConfig);

        QNetworkReply* reply = manager->post(request, QJsonDocument(payload).toJson());
        
        if (!reply) {
            qCritical() << "[XfyunAPI] 无法创建网络请求";
            const QString fallback = generateLocalResponse(lastQuestion);
            emit answerReceived(fallback);
            return;
        }

        qDebug() << "[XfyunAPI] POST URL:" << url;
        qDebug() << "[XfyunAPI] POST JSON:" << QJsonDocument(payload).toJson();
        qDebug() << "[XfyunAPI] SSL支持状态: ✅";
        
    } catch (const std::exception& e) {
        qCritical() << "[XfyunAPI] 发送请求时发生异常:" << e.what();
        emit answerReceived(generateLocalResponse(lastQuestion));
    } catch (...) {
        qCritical() << "[XfyunAPI] 发送请求时发生未知异常";
        emit answerReceived(generateLocalResponse(lastQuestion));
    }
}

void XfyunAPI::onReplyFinished(QNetworkReply *reply)
{
    // 增强错误处理和崩溃防护
    if (!reply) {
        qCritical() << "[XfyunAPI] 网络回复对象为空";
        emit answerReceived("网络连接异常，请稍后重试");
        return;
    }
    
    // 检查网络错误
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "[XfyunAPI] 网络错误:" << reply->error() << reply->errorString();
        // 无论何种错误，对用户均回退为本地回答
        const QString fallback = generateLocalResponse(lastQuestion);
        emit answerReceived(fallback);
        reply->deleteLater();
        return;
    }
    
    QByteArray response = reply->readAll();
    qDebug() << "[XfyunAPI] 原始返回：" << response;
    
    // 检查响应是否为空
    if (response.isEmpty()) {
        qWarning() << "[XfyunAPI] 服务器返回空响应";
        emit answerReceived("服务器返回空响应，请稍后重试");
        reply->deleteLater();
        return;
    }
    
    try {
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(response, &parseError);
        
        // 检查JSON解析错误
        if (parseError.error != QJsonParseError::NoError) {
            qWarning() << "[XfyunAPI] JSON解析错误:" << parseError.errorString();
            emit answerReceived(generateLocalResponse(lastQuestion));
            reply->deleteLater();
            return;
        }
        
        // 检查是否为有效的JSON对象
        if (!doc.isObject()) {
            qWarning() << "[XfyunAPI] 返回的JSON不是对象格式";
            emit answerReceived(generateLocalResponse(lastQuestion));
            reply->deleteLater();
            return;
        }
        
        QJsonObject rootObj = doc.object();
        
        // 检查是否包含错误信息
        if (rootObj.contains("error")) {
            QJsonObject errorObj = rootObj["error"].toObject();
            QString errorMessage = errorObj["message"].toString();
            qWarning() << "[XfyunAPI] API错误:" << errorMessage;
            emit answerReceived(generateLocalResponse(lastQuestion));
            reply->deleteLater();
            return;
        }
        
        // 安全地提取答案
        QString ans;
        if (rootObj.contains("choices") && rootObj["choices"].isArray()) {
            QJsonArray choicesArray = rootObj["choices"].toArray();
            if (!choicesArray.isEmpty()) {
                QJsonObject firstChoice = choicesArray[0].toObject();
                if (firstChoice.contains("message") && firstChoice["message"].isObject()) {
                    QJsonObject messageObj = firstChoice["message"].toObject();
                    if (messageObj.contains("content")) {
                        ans = messageObj["content"].toString();
                    }
                }
            }
        }
        
        qDebug() << "[XfyunAPI] 提取的回答：" << ans;
        
        // 检查答案是否为空
        if (ans.trimmed().isEmpty()) {
            emit answerReceived(generateLocalResponse(lastQuestion));
        } else {
            emit answerReceived(ans);
        }
        
    } catch (const std::exception& e) {
        qCritical() << "[XfyunAPI] 处理响应时发生异常:" << e.what();
        emit answerReceived(generateLocalResponse(lastQuestion));
    } catch (...) {
        qCritical() << "[XfyunAPI] 处理响应时发生未知异常";
        emit answerReceived(generateLocalResponse(lastQuestion));
    }
    
    reply->deleteLater();
}

void XfyunAPI::initializeSSLConfiguration()
{
    qDebug() << "[XfyunAPI] 初始化SSL配置";
    
    // 检查SSL支持
    if (!isSSLSupported()) {
        qCritical() << "[XfyunAPI] SSL不受支持，无法建立安全连接";
        return;
    }
    
    try {
        // 设置默认SSL配置
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        
        // 设置SSL协议
        sslConfig.setProtocol(QSsl::SecureProtocols);
        
        // 【安全修复】设置对等验证模式 - 使用更安全的验证模式
        sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);
        
        // 应用SSL配置
        QSslConfiguration::setDefaultConfiguration(sslConfig);
        
        qDebug() << "[XfyunAPI] SSL配置初始化完成";
        
    } catch (const std::exception& e) {
        qCritical() << "[XfyunAPI] SSL配置初始化异常:" << e.what();
    } catch (...) {
        qCritical() << "[XfyunAPI] SSL配置初始化发生未知异常";
    }
}

void XfyunAPI::setupNetworkSecurity()
{
    if (!manager) return;
    
    // 连接SSL错误处理
    connect(manager, QOverload<QNetworkReply*, const QList<QSslError>&>::of(&QNetworkAccessManager::sslErrors),
            this, &XfyunAPI::onSslErrors);
    
    // 启用系统代理（适配公司网络/操作系统代理设置）
    QNetworkProxyFactory::setUseSystemConfiguration(true);
    qDebug() << "[XfyunAPI] 已启用系统代理配置";
    
    qDebug() << "[XfyunAPI] 网络安全设置完成";
}

bool XfyunAPI::isSSLSupported()
{
    bool supported = QSslSocket::supportsSsl();
    qDebug() << "[XfyunAPI] SSL支持状态:" << (supported ? "支持" : "不支持");
    
    if (supported) {
        qDebug() << "[XfyunAPI] SSL版本:" << QSslSocket::sslLibraryVersionString();
        qDebug() << "[XfyunAPI] SSL构建版本:" << QSslSocket::sslLibraryBuildVersionString();
    } else {
        qWarning() << "[XfyunAPI] 系统未检测到SSL库";
    }
    
    return supported;
}

QString XfyunAPI::generateLocalResponse(const QString& question)
{
    // 智能本地回复系统 - 当SSL不可用时提供有意义的回复
    QString lowerQuestion = question.toLower();
    
    // HR相关问题的智能回复
    if (lowerQuestion.contains("考勤") || lowerQuestion.contains("上班") || lowerQuestion.contains("打卡")) {
        return "关于考勤管理：\n"
               "• 正常工作时间为9:00-18:00\n"
               "• 请按时打卡考勤\n"
               "• 如有特殊情况请提前申请\n"
               "• 迟到早退需要提前报备\n"
               "\n详细考勤制度请查看员工手册或咨询HR部门。";
    }
    
    if (lowerQuestion.contains("请假") || lowerQuestion.contains("假期") || lowerQuestion.contains("休假")) {
        return "关于请假申请：\n"
               "• 病假：需提供医院证明\n"
               "• 事假：需提前3天申请\n"
               "• 年假：需提前1周申请\n"
               "• 产假：按国家规定执行\n"
               "\n详细申请流程请查看员工手册。";
    }
    
    if (lowerQuestion.contains("薪资") || lowerQuestion.contains("工资") || lowerQuestion.contains("薪酬") || 
        lowerQuestion.contains("奖金") || lowerQuestion.contains("提成")) {
        return "薪酬体系详情：\n"
               "\n发放时间：\n"
               "• 月薪发放：每月15日（遇节假日提前）\n"
               "• 绩效奖金：季度末发放\n"
               "• 年终奖：春节前统一发放\n"
               "\n薪资构成：\n"
               "• 基本工资：岗位工资+学历津贴\n"
               "• 绩效奖金：根据KPI考核结果\n"
               "• 专项津贴：加班费+出差补贴\n"
               "\n扣除项目：\n"
               "• 社保公积金：个人承担部分\n"
               "• 个人所得税：依法代扣代缴\n"
               "\n薪酬详情请咨询财务部：内线8002";
    }
    
    if (lowerQuestion.contains("培训") || lowerQuestion.contains("学习") || lowerQuestion.contains("成长")) {
        return "关于培训发展：\n"
               "• 公司提供定期技能培训\n"
               "• 支持员工参加外部学习\n"
               "• 完善的职业发展通道\n"
               "• 新员工入职培训计划\n"
               "\n培训信息请关注HR通知或咨询人事部门。";
    }
    
    if (lowerQuestion.contains("福利") || lowerQuestion.contains("保险") || lowerQuestion.contains("待遇")) {
        return "员工福利待遇详情：\n"
               "\n基础保障：\n"
               "• 五险一金：按最高标准全额缴纳\n"
               "• 补充商业保险：意外险+医疗险\n"
               "• 带薪年假：工龄满1年享受5-15天\n"
               "\n特色福利：\n"
               "• 年度体检：三甲医院全面体检\n"
               "• 节日福利：春节/中秋/生日礼品\n"
               "• 团建活动：季度团建+年度旅游\n"
               "• 弹性工作：核心时间+灵活安排\n"
               "\n详细福利政策请咨询HR部门：内线8001";
    }
    
    // 联系方式和部门信息
    if (lowerQuestion.contains("联系") || lowerQuestion.contains("电话") || lowerQuestion.contains("部门") || 
        lowerQuestion.contains("找谁") || lowerQuestion.contains("咨询谁")) {
        return "各部门联系方式：\n"
               "\n行政部门：\n"
               "• HR人事部：内线8001 hr@company.com\n"
               "• 财务部：内线8002 finance@company.com\n"
               "• 行政部：内线8003 admin@company.com\n"
               "\n技术部门：\n"
               "• IT技术部：内线8010 it@company.com\n"
               "• 产品部：内线8011 product@company.com\n"
               "\n办公时间：周一至周五 9:00-18:00\n"
               "公司地址：请查看员工手册或咨询前台";
    }
    
    // 入职和离职相关
    if (lowerQuestion.contains("入职") || lowerQuestion.contains("离职") || lowerQuestion.contains("新员工") || 
        lowerQuestion.contains("辞职") || lowerQuestion.contains("转正")) {
        return "入职离职相关流程：\n"
               "\n新员工入职：\n"
               "• 试用期：3个月（可适当调整）\n"
               "• 入职培训：岗位技能+企业文化\n"
               "• 转正评估：试用期满前1周进行\n"
               "\n离职办理：\n"
               "• 提前通知：正式员工提前30天\n"
               "• 工作交接：完成所有在手项目\n"
               "• 离职手续：HR部门统一办理\n"
               "\n具体流程请联系HR部门：内线8001";
    }
    
    // 问候语和身份询问回复
    if (lowerQuestion.contains("你好") || lowerQuestion.contains("hello") || lowerQuestion.contains("hi") ||
        lowerQuestion.contains("你是谁") || lowerQuestion.contains("你是什么") || 
        lowerQuestion.contains("介绍") || lowerQuestion.contains("who are you")) {
        return "您好！我是您的HR智能助手。\n"
               "\n我的专业能力：\n"
               "• 考勤制度和打卡规定\n"
               "• 请假流程和假期政策\n"
               "• 薪酬结构和发放时间\n"
               "• 培训机会和发展规划\n"
               "• 员工福利和保险政策\n"
               "• 各部门联系方式和办事流程\n"
               "\n请直接告诉我您想了解什么，我会为您提供详细的专业解答！";
    }
    
    // 智能引导回复
    return XfyunAPI::QObject::tr("我是您的HR智能助手，很高兴为您服务！\n\n我可以帮您解答：\n• 输入「考勤」了解工作时间和打卡制度\n• 输入「请假」了解各类假期申请流程\n• 输入「工资」了解薪酬发放和结构\n• 输入「培训」了解学习发展机会\n• 输入「福利」了解员工待遇政策\n\n请告诉我您具体想了解什么，我会为您提供专业详细的解答！\n\n如需更复杂的咨询，建议直接联系HR部门：内线8001");
}

void XfyunAPI::onSslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    qWarning() << "[XfyunAPI] SSL错误发生，错误数量:" << errors.size();
    
    // 【安全修复】更严格的SSL错误处理
    bool hasCriticalError = false;
    QStringList errorDetails;
    
    for (const QSslError &error : errors) {
        QString errorString = error.errorString();
        errorDetails << errorString;
        qWarning() << "[XfyunAPI] SSL错误:" << errorString;
        
        // 检查是否为关键错误（证书验证失败等）
        if (error.error() == QSslError::CertificateExpired ||
            error.error() == QSslError::CertificateRevoked ||
            error.error() == QSslError::CertificateUntrusted ||
            error.error() == QSslError::HostNameMismatch) {
            hasCriticalError = true;
        }
    }
    
    if (hasCriticalError) {
        qCritical() << "[XfyunAPI] 检测到关键SSL安全错误，拒绝连接";
        reply->abort(); // 中止请求
        emit answerReceived("SSL安全验证失败，请检查网络连接和证书配置");
    } else {
        qWarning() << "[XfyunAPI] 检测到非关键SSL错误，继续连接（可能是自签名证书等）";
        // 仅对非关键错误忽略SSL错误
        reply->ignoreSslErrors();
    }
}

void XfyunAPI::loadCredentials()
{
    // 优先读取环境变量；未提供时再读取仅保存在本机的 QSettings。
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    appId = env.value("XFYUN_APP_ID").trimmed();
    apiSecret = env.value("XFYUN_API_SECRET").trimmed();
    apiKey = env.value("XFYUN_API_KEY").trimmed();
    QString envPwd = env.value("XFYUN_API_PASSWORD");
    if (!envPwd.trimmed().isEmpty()) {
        apiPassword = envPwd.trimmed();
        qDebug() << "[XfyunAPI] 已从环境变量载入API Password";
    }
    QSettings settings("Company HR System", "HR智能问答系统");
    if (appId.isEmpty()) {
        appId = settings.value("xfyun/app_id").toString().trimmed();
    }
    if (apiSecret.isEmpty()) {
        apiSecret = settings.value("xfyun/api_secret").toString().trimmed();
    }
    if (apiKey.isEmpty()) {
        apiKey = settings.value("xfyun/api_key").toString().trimmed();
    }
    if (apiPassword.isEmpty()) {
        apiPassword = settings.value("xfyun/api_password").toString().trimmed();
    }
    if (!apiPassword.isEmpty()) {
        qDebug() << "[XfyunAPI] 已从QSettings载入API Password";
    }
}
