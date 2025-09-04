#include "reportgenerator.h"
#include <QTextStream>
#include <QStringConverter>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

ReportGenerator::ReportGenerator(Database* database, QObject* parent)
    : QObject(parent)
    , database_(database)
{
}

bool ReportGenerator::generateUserReport(const QString& filePath, const QDate& startDate, const QDate& endDate)
{
    QString html = generateUserReportHTML(startDate, endDate);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << html;
    file.close();
    
    return true;
}

bool ReportGenerator::generateDocumentReport(const QString& filePath, const QDate& startDate, const QDate& endDate)
{
    QString html = generateDocumentReportHTML(startDate, endDate);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << html;
    file.close();
    
    return true;
}

bool ReportGenerator::generateQAReport(const QString& filePath, const QDate& startDate, const QDate& endDate)
{
    QString html = generateQAReportHTML(startDate, endDate);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << html;
    file.close();
    
    return true;
}

bool ReportGenerator::generateStatisticsReport(const QString& filePath, const QDate& startDate, const QDate& endDate)
{
    QString html = generateStatisticsReportHTML(startDate, endDate);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << html;
    file.close();
    
    return true;
}

bool ReportGenerator::generateComprehensiveReport(const QString& filePath, const QDate& startDate, const QDate& endDate)
{
    QString html = generateComprehensiveReportHTML(startDate, endDate);
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    stream << html;
    file.close();
    
    return true;
}

QString ReportGenerator::generateUserReportHTML(const QDate& startDate, const QDate& endDate)
{
    QString html = getReportHeader("用户管理报表", startDate, endDate);
    
    QList<User> users = database_->getAllUsers();
    
    html += "<h2>用户统计</h2>";
    html += "<p>总用户数: " + QString::number(users.size()) + "</p>";
    
    int activeUsers = 0, pendingUsers = 0, inactiveUsers = 0;
    int employeeCount = 0, adminCount = 0;
    
    for (const User& user : users) {
        switch (user.getStatus()) {
        case UserStatus::Active: activeUsers++; break;
        case UserStatus::Pending: pendingUsers++; break;
        case UserStatus::Inactive: inactiveUsers++; break;
        }
        
        if (user.isHRAdmin()) {
            adminCount++;
        } else {
            employeeCount++;
        }
    }
    
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'><th>统计项目</th><th>数量</th></tr>";
    html += "<tr><td>已激活用户</td><td>" + QString::number(activeUsers) + "</td></tr>";
    html += "<tr><td>待审核用户</td><td>" + QString::number(pendingUsers) + "</td></tr>";
    html += "<tr><td>已禁用用户</td><td>" + QString::number(inactiveUsers) + "</td></tr>";
    html += "<tr><td>普通员工</td><td>" + QString::number(employeeCount) + "</td></tr>";
    html += "<tr><td>HR管理员</td><td>" + QString::number(adminCount) + "</td></tr>";
    html += "</table>";
    
    html += "<h2>用户详细列表</h2>";
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'>";
    html += "<th>工号</th><th>姓名</th><th>部门</th><th>角色</th><th>状态</th><th>创建时间</th><th>最后登录</th>";
    html += "</tr>";
    
    for (const User& user : users) {
        html += "<tr>";
        html += "<td>" + user.getEmployeeId() + "</td>";
        html += "<td>" + user.getName() + "</td>";
        html += "<td>" + user.getDepartment() + "</td>";
        html += "<td>" + QString(user.isHRAdmin() ? "HR管理员" : "普通员工") + "</td>";
        
        QString status;
        switch (user.getStatus()) {
        case UserStatus::Active: status = "已激活"; break;
        case UserStatus::Pending: status = "待审核"; break;
        case UserStatus::Inactive: status = "已禁用"; break;
        }
        html += "<td>" + status + "</td>";
        
        html += "<td>" + formatDate(user.getCreateTime().date()) + "</td>";
        html += "<td>" + formatDateTime(user.getLastLoginTime()) + "</td>";
        html += "</tr>";
    }
    
    html += "</table>";
    html += getReportFooter();
    
    return html;
}

QString ReportGenerator::generateDocumentReportHTML(const QDate& startDate, const QDate& endDate)
{
    QString html = getReportHeader("文档管理报表", startDate, endDate);
    
    QList<Document> documents = database_->getAllDocuments();
    
    html += "<h2>文档统计</h2>";
    html += "<p>总文档数: " + QString::number(documents.size()) + "</p>";
    
    QMap<QString, int> categoryStats;
    for (const Document& doc : documents) {
        categoryStats[doc.getCategory()]++;
    }
    
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'><th>文档类别</th><th>数量</th></tr>";
    
    for (auto it = categoryStats.begin(); it != categoryStats.end(); ++it) {
        html += "<tr><td>" + it.key() + "</td><td>" + QString::number(it.value()) + "</td></tr>";
    }
    
    html += "</table>";
    
    html += "<h2>文档详细列表</h2>";
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'>";
    html += "<th>ID</th><th>标题</th><th>类别</th><th>创建时间</th><th>更新时间</th>";
    html += "</tr>";
    
    for (const Document& doc : documents) {
        html += "<tr>";
        html += "<td>" + QString::number(doc.getId()) + "</td>";
        html += "<td>" + doc.getTitle() + "</td>";
        html += "<td>" + doc.getCategory() + "</td>";
        html += "<td>" + formatDateTime(doc.getCreateTime()) + "</td>";
        html += "<td>" + formatDateTime(doc.getUpdateTime()) + "</td>";
        html += "</tr>";
    }
    
    html += "</table>";
    html += getReportFooter();
    
    return html;
}

QString ReportGenerator::generateQAReportHTML(const QDate& startDate, const QDate& endDate)
{
    QString html = getReportHeader("问答管理报表", startDate, endDate);
    
    QList<QuestionRecord> qaRecords = database_->getQuestionRecords();
    
    html += "<h2>问答统计</h2>";
    html += "<p>总问答数: " + QString::number(qaRecords.size()) + "</p>";
    
    QMap<QString, int> categoryStats;
    for (const QuestionRecord& qa : qaRecords) {
        categoryStats[qa.getCategory()]++;
    }
    
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'><th>问题类别</th><th>数量</th></tr>";
    
    for (auto it = categoryStats.begin(); it != categoryStats.end(); ++it) {
        html += "<tr><td>" + it.key() + "</td><td>" + QString::number(it.value()) + "</td></tr>";
    }
    
    html += "</table>";
    
    html += "<h2>问答详细列表</h2>";
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'>";
    html += "<th>ID</th><th>员工工号</th><th>问题</th><th>答案</th><th>类别</th><th>创建时间</th>";
    html += "</tr>";
    
    for (const QuestionRecord& qa : qaRecords) {
        html += "<tr>";
        html += "<td>" + QString::number(qa.getId()) + "</td>";
        html += "<td>" + qa.getEmployeeId() + "</td>";
        html += "<td>" + qa.getQuestion() + "</td>";
        html += "<td>" + qa.getAnswer().left(100) + (qa.getAnswer().length() > 100 ? "..." : "") + "</td>";
        html += "<td>" + qa.getCategory() + "</td>";
        html += "<td>" + formatDateTime(qa.getCreateTime()) + "</td>";
        html += "</tr>";
    }
    
    html += "</table>";
    html += getReportFooter();
    
    return html;
}

QString ReportGenerator::generateStatisticsReportHTML(const QDate& startDate, const QDate& endDate)
{
    QString html = getReportHeader("数据统计报表", startDate, endDate);
    
    int totalUsers = database_->getUserCount();
    int totalDocs = database_->getDocumentCount();
    int totalQuestions = database_->getQuestionCount();
    
    // 修复：正确计算活跃用户数（状态为Active的用户）
    QList<User> allUsers = database_->getAllUsers();
    int activeUsers = 0;
    for (const User& user : allUsers) {
        if (user.getStatus() == UserStatus::Active) {
            activeUsers++;
        }
    }
    
    int pendingUsers = database_->getPendingUsers().size();
    
    html += "<h2>系统总体统计</h2>";
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'><th>统计项目</th><th>数量</th></tr>";
    html += "<tr><td>总用户数</td><td>" + QString::number(totalUsers) + "</td></tr>";
    html += "<tr><td>总文档数</td><td>" + QString::number(totalDocs) + "</td></tr>";
    html += "<tr><td>总问题数</td><td>" + QString::number(totalQuestions) + "</td></tr>";
    html += "<tr><td>活跃用户</td><td>" + QString::number(activeUsers) + "</td></tr>";
    html += "<tr><td>待审核用户</td><td>" + QString::number(pendingUsers) + "</td></tr>";
    html += "</table>";
    
    html += getReportFooter();
    
    return html;
}

QString ReportGenerator::generateComprehensiveReportHTML(const QDate& startDate, const QDate& endDate)
{
    QString html = getReportHeader("综合统计报表", startDate, endDate);
    
    // 用户统计
    html += "<h2>用户统计</h2>";
    QList<User> users = database_->getAllUsers();
    int activeUsers = 0, pendingUsers = 0, inactiveUsers = 0;
    int employeeCount = 0, adminCount = 0;
    
    for (const User& user : users) {
        switch (user.getStatus()) {
        case UserStatus::Active: activeUsers++; break;
        case UserStatus::Pending: pendingUsers++; break;
        case UserStatus::Inactive: inactiveUsers++; break;
        }
        
        if (user.isHRAdmin()) {
            adminCount++;
        } else {
            employeeCount++;
        }
    }
    
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'><th>用户类型</th><th>数量</th></tr>";
    html += "<tr><td>已激活用户</td><td>" + QString::number(activeUsers) + "</td></tr>";
    html += "<tr><td>待审核用户</td><td>" + QString::number(pendingUsers) + "</td></tr>";
    html += "<tr><td>已禁用用户</td><td>" + QString::number(inactiveUsers) + "</td></tr>";
    html += "<tr><td>普通员工</td><td>" + QString::number(employeeCount) + "</td></tr>";
    html += "<tr><td>HR管理员</td><td>" + QString::number(adminCount) + "</td></tr>";
    html += "</table>";
    
    // 文档统计
    html += "<h2>文档统计</h2>";
    QList<Document> documents = database_->getAllDocuments();
    QMap<QString, int> docCategoryStats;
    for (const Document& doc : documents) {
        docCategoryStats[doc.getCategory()]++;
    }
    
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'><th>文档类别</th><th>数量</th></tr>";
    
    for (auto it = docCategoryStats.begin(); it != docCategoryStats.end(); ++it) {
        html += "<tr><td>" + it.key() + "</td><td>" + QString::number(it.value()) + "</td></tr>";
    }
    
    html += "</table>";
    
    // 问答统计
    html += "<h2>问答统计</h2>";
    QList<QuestionRecord> qaRecords = database_->getQuestionRecords();
    QMap<QString, int> qaCategoryStats;
    for (const QuestionRecord& qa : qaRecords) {
        qaCategoryStats[qa.getCategory()]++;
    }
    
    html += "<table border='1' style='border-collapse: collapse; width: 100%; margin: 20px 0;'>";
    html += "<tr style='background-color: #f0f0f0;'><th>问题类别</th><th>数量</th></tr>";
    
    for (auto it = qaCategoryStats.begin(); it != qaCategoryStats.end(); ++it) {
        html += "<tr><td>" + it.key() + "</td><td>" + QString::number(it.value()) + "</td></tr>";
    }
    
    html += "</table>";
    
    html += getReportFooter();
    
    return html;
}

QString ReportGenerator::getReportHeader(const QString& title, const QDate& startDate, const QDate& endDate)
{
    QString html = "<!DOCTYPE html><html><head>";
    html += "<meta charset='UTF-8'>";
    html += "<title>" + title + "</title>";
    html += "<style>";
    html += "body { font-family: 'Microsoft YaHei', Arial, sans-serif; margin: 20px; }";
    html += "h1 { color: #2c3e50; text-align: center; border-bottom: 2px solid #3498db; padding-bottom: 10px; }";
    html += "h2 { color: #34495e; margin-top: 30px; }";
    html += "table { border-collapse: collapse; width: 100%; margin: 20px 0; }";
    html += "th, td { padding: 8px; text-align: left; border: 1px solid #ddd; }";
    html += "th { background-color: #f0f0f0; font-weight: bold; }";
    html += "tr:nth-child(even) { background-color: #f9f9f9; }";
    html += ".header-info { text-align: center; margin: 20px 0; color: #7f8c8d; }";
    html += "</style>";
    html += "</head><body>";
    
    html += "<h1>" + title + "</h1>";
    html += "<div class='header-info'>";
    html += "<p>生成时间: " + formatDateTime(QDateTime::currentDateTime()) + "</p>";
    if (startDate.isValid() && endDate.isValid()) {
        html += "<p>统计时间范围: " + formatDate(startDate) + " 至 " + formatDate(endDate) + "</p>";
    }
    html += "</div>";
    
    return html;
}

QString ReportGenerator::getReportFooter()
{
    QString html = "<div style='margin-top: 50px; text-align: center; color: #7f8c8d; border-top: 1px solid #ddd; padding-top: 20px;'>";
    html += "<p>本报表由HR智能问答系统自动生成</p>";
    html += "<p>生成时间: " + formatDateTime(QDateTime::currentDateTime()) + "</p>";
    html += "</div>";
    html += "</body></html>";
    
    return html;
}

QString ReportGenerator::formatDate(const QDate& date)
{
    return date.toString("yyyy-MM-dd");
}

QString ReportGenerator::formatDateTime(const QDateTime& dateTime)
{
    return dateTime.toString("yyyy-MM-dd hh:mm:ss");
}

bool ReportGenerator::exportQAToExcel(const QString& filePath, const QList<QuestionRecord>& records)
{
    // 由于Qt没有内置的Excel导出功能，我们创建一个CSV格式的文件
    // 用户可以用Excel打开CSV文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8);
    
    // 写入BOM以确保Excel正确识别UTF-8编码
    stream << "\xEF\xBB\xBF";
    
    // 写入表头
    stream << "ID,员工工号,问题,答案,类别,创建时间\n";
    
    // 写入数据
    for (const QuestionRecord& record : records) {
        QString line = QString("%1,%2,%3,%4,%5,%6\n")
            .arg(record.getId())
            .arg(record.getEmployeeId())
            .arg(record.getQuestion().replace("\"", "\"\""))  // 转义双引号
            .arg(record.getAnswer().replace("\"", "\"\""))    // 转义双引号
            .arg(record.getCategory())
            .arg(record.getCreateTime().toString("yyyy-MM-dd hh:mm:ss"));
        
        stream << line;
    }
    
    file.close();
    return true;
}
