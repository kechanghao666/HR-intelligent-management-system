#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QObject>
#include <QString>
#include <QDate>
#include <QTextDocument>
#include <QPrinter>
#include "database.h"

class ReportGenerator : public QObject
{
    Q_OBJECT

public:
    explicit ReportGenerator(Database* database, QObject* parent = nullptr);
    
    // 生成不同类型的报表
    bool generateUserReport(const QString& filePath, const QDate& startDate = QDate(), const QDate& endDate = QDate());
    bool generateDocumentReport(const QString& filePath, const QDate& startDate = QDate(), const QDate& endDate = QDate());
    bool generateQAReport(const QString& filePath, const QDate& startDate = QDate(), const QDate& endDate = QDate());
    bool generateStatisticsReport(const QString& filePath, const QDate& startDate = QDate(), const QDate& endDate = QDate());
    bool generateComprehensiveReport(const QString& filePath, const QDate& startDate = QDate(), const QDate& endDate = QDate());
    
    // Excel导出功能
    bool exportQAToExcel(const QString& filePath, const QList<QuestionRecord>& records);

private:
    QString generateUserReportHTML(const QDate& startDate, const QDate& endDate);
    QString generateDocumentReportHTML(const QDate& startDate, const QDate& endDate);
    QString generateQAReportHTML(const QDate& startDate, const QDate& endDate);
    QString generateStatisticsReportHTML(const QDate& startDate, const QDate& endDate);
    QString generateComprehensiveReportHTML(const QDate& startDate, const QDate& endDate);
    
    QString getReportHeader(const QString& title, const QDate& startDate, const QDate& endDate);
    QString getReportFooter();
    QString formatDate(const QDate& date);
    QString formatDateTime(const QDateTime& dateTime);
    
    Database* database_;
};

#endif // REPORTGENERATOR_H
