#include "formexporter.h"
#include <QTextDocument>
#include <QPrinter>
#include <QPagedPaintDevice>
#include <QPageLayout>
#include <QPageSize>
#include <QTextStream>
#include <QFileInfo>
#include <QDateTime>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <QThread>
#include <QRegularExpression>
#include <functional>

// 样式常量定义
const QString FormExporter::DEFAULT_CSS_STYLE = R"(
    body {
        font-family: 'Microsoft YaHei', 'SimSun', Arial, sans-serif;
        margin: 0; padding: 32px;
        background: #f8fafc;
        color: #111827;
        text-align: center;
    }
    .form-header {
        text-align: center;
        margin-bottom: 24px;
        border-bottom: 2px solid #e5e7eb;
        padding-bottom: 16px;
    }
    .form-title {
        font-size: 28px;
        font-weight: 800;
        color: #1d4ed8;
        margin: 0 0 8px 0;
    }
    .form-description {
        font-size: 14px;
        color: #6b7280;
        margin: 0;
    }
    .form-content {
        margin-top: 20px;
    }
    .form-table {
        width: 90%;
        margin: 16px auto;
        border-collapse: collapse;
        background: #ffffff;
        border: 1px solid #e5e7eb;
        table-layout: fixed;
    }
    .form-table th, .form-table td {
        border: 1px solid #e5e7eb;
        padding: 12px 14px;
        text-align: left;
        word-break: break-word;
    }
    .form-table th {
        background: #f3f4f6;
        color: #111827;
        font-weight: 700;
        font-size: 14px;
    }
    .form-table tr:nth-child(even) {
        background-color: #fbfdff;
    }
    .form-table tr:hover {
        background-color: #f1f5f9;
    }
    .label-cell {
        background: #f9fafb !important;
        font-weight: 600;
        width: 28%;
        color: #374151;
        text-align: right;
        padding-right: 14px;
    }
    .value-cell {
        background: white !important;
        color: #111827;
    }
    .footer {
        margin-top: 28px;
        padding-top: 14px;
        border-top: 1px solid #e5e7eb;
        text-align: center;
        font-size: 12px;
        color: #6b7280;
    }
    .watermark {
        position: fixed;
        bottom: 10px;
        right: 10px;
        opacity: 0.3;
        font-size: 10px;
        color: #9ca3af;
    }
)";

const QString FormExporter::PDF_CSS_STYLE = R"(
    @page {
        margin: 10mm;
        size: A4;
    }
    body {
        font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', 'Microsoft YaHei', Arial, sans-serif;
        font-size: 12pt;
        line-height: 1.65;
        color: #111827;
        text-align: center;
        margin: 0;
        padding: 0;
    }
    .container {
        display: inline-block;
        width: 100%;
        text-align: left;
    }
    .form-header {
        background: #1f2937;
        color: #ffffff;
        padding: 22pt 16pt;
        text-align: center;
        margin-bottom: 16pt;
    }
    .form-title {
        font-size: 21pt;
        font-weight: 700;
        margin: 0 0 6pt 0;
        letter-spacing: 0.5pt;
    }
    .form-description {
        font-size: 13pt;
        opacity: 0.95;
        margin: 0;
        font-weight: 300;
    }
    .form-table {
        width: 80%;
        border-collapse: separate;
        border-spacing: 0;
        font-size: 12pt;
        margin: 0 auto;
    }
    .form-table tr:nth-child(even) {
        background-color: #f8f9fa;
    }
    .form-table td {
        padding: 12pt 14pt;
        border-bottom: 1pt solid #e5e7eb;
        vertical-align: middle;
    }
    .label-cell {
        background: #111827;
        color: #ffffff;
        font-weight: 600;
        width: 30%;
        text-align: right;
        border-right: 2pt solid #667eea;
    }
    .value-cell {
        background: #ffffff;
        color: #111827;
        width: 70%;
        text-align: left;
    }
    .empty-value {
        color: #6b7280;
        font-style: italic;
        opacity: 0.85;
    }
    .footer {
        background: #f8f9fa;
        padding: 12pt 16pt;
        text-align: center;
        font-size: 10pt;
        color: #6b7280;
        border-top: 1pt solid #e5e7eb;
        margin-top: 16pt;
    }
)";

// ===== FormExporter 实现 =====

FormExporter::FormExporter()
    : outputDir_(QStandardPaths::writableLocation(QStandardPaths::DesktopLocation))
    , autoOpenFile_(true)
{
}

ExportResult FormExporter::exportForm(FormTemplate* form, ExportFormat format, const QString& fileName)
{
    if (!form) {
        return ExportResult(false, "", "表单对象为空");
    }
    
    switch (format) {
    case ExportFormat::PDF:
        return exportToPDF(form, fileName);
    case ExportFormat::HTML:
        return exportToHTML(form, fileName);
    case ExportFormat::CSV:
        return exportToCSV(form, fileName);
    case ExportFormat::TXT:
        return exportToTXT(form, fileName);
    default:
        return ExportResult(false, "", "不支持的导出格式");
    }
}

ExportResult FormExporter::exportToPDF(FormTemplate* form, const QString& fileName)
{
    try {
        QString outputPath = ensureOutputDirectory();
        QString fullFileName = fileName.isEmpty() ? generateFileName(form, ExportFormat::PDF) : fileName;
        if (!fullFileName.endsWith(".pdf")) {
            fullFileName += ".pdf";
        }
        
        QString filePath = QDir(outputPath).absoluteFilePath(fullFileName);
        
        // 生成HTML内容
        QString htmlContent = form->toHtml();
        
        // 应用PDF样式，并添加容器包裹，避免页面缩小
        htmlContent.replace("<style>", QString("<style>%1").arg(FormExporter::PDF_CSS_STYLE));
        htmlContent.replace("<body>", "<body><div class='container'>");
        htmlContent.replace("</body>", "</div></body>");
        
        // 创建PDF
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filePath);
        printer.setPageSize(QPageSize::A4);
        // 稍微缩小页边距，给内容更大可视区
        printer.setPageMargins(QMarginsF(10, 10, 10, 10), QPageLayout::Millimeter);
        
        QTextDocument document;
        document.setHtml(htmlContent);
        document.print(&printer);
        
        // 验证文件是否创建成功
        QFileInfo fileInfo(filePath);
        if (!fileInfo.exists() || fileInfo.size() == 0) {
            return ExportResult(false, "", "PDF文件创建失败");
        }
        
        ExportResult result(true, filePath, "");
        result.fileSize = formatFileSize(fileInfo.size());
        
        if (autoOpenFile_) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
        
        qDebug() << "[表单导出] PDF导出成功:" << filePath;
        return result;
        
    } catch (const std::exception& e) {
        return ExportResult(false, "", QString("PDF导出异常: %1").arg(e.what()));
    } catch (...) {
        return ExportResult(false, "", "PDF导出时发生未知错误");
    }
}

ExportResult FormExporter::exportToHTML(FormTemplate* form, const QString& fileName)
{
    try {
        QString outputPath = ensureOutputDirectory();
        QString fullFileName = fileName.isEmpty() ? generateFileName(form, ExportFormat::HTML) : fileName;
        if (!fullFileName.endsWith(".html")) {
            fullFileName += ".html";
        }
        
        QString filePath = QDir(outputPath).absoluteFilePath(fullFileName);
        
        // 生成HTML内容
        QString htmlContent = form->toHtml();
        
        // 写入文件
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return ExportResult(false, "", "无法创建HTML文件");
        }
        
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << htmlContent;
        file.close();
        
        QFileInfo fileInfo(filePath);
        ExportResult result(true, filePath, "");
        result.fileSize = formatFileSize(fileInfo.size());
        
        if (autoOpenFile_) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
        
        qDebug() << "[表单导出] HTML导出成功:" << filePath;
        return result;
        
    } catch (const std::exception& e) {
        return ExportResult(false, "", QString("HTML导出异常: %1").arg(e.what()));
    } catch (...) {
        return ExportResult(false, "", "HTML导出时发生未知错误");
    }
}

ExportResult FormExporter::exportToCSV(FormTemplate* form, const QString& fileName)
{
    try {
        QString outputPath = ensureOutputDirectory();
        QString fullFileName = fileName.isEmpty() ? generateFileName(form, ExportFormat::CSV) : fileName;
        if (!fullFileName.endsWith(".csv")) {
            fullFileName += ".csv";
        }
        
        QString filePath = QDir(outputPath).absoluteFilePath(fullFileName);
        
        // 生成CSV内容
        QString csvContent = form->toCsv();
        
        // 写入文件
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return ExportResult(false, "", "无法创建CSV文件");
        }
        
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << "\uFEFF"; // BOM for UTF-8
        out << csvContent;
        file.close();
        
        QFileInfo fileInfo(filePath);
        ExportResult result(true, filePath, "");
        result.fileSize = formatFileSize(fileInfo.size());
        
        if (autoOpenFile_) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
        
        qDebug() << "[表单导出] CSV导出成功:" << filePath;
        return result;
        
    } catch (const std::exception& e) {
        return ExportResult(false, "", QString("CSV导出异常: %1").arg(e.what()));
    } catch (...) {
        return ExportResult(false, "", "CSV导出时发生未知错误");
    }
}

ExportResult FormExporter::exportToTXT(FormTemplate* form, const QString& fileName)
{
    try {
        QString outputPath = ensureOutputDirectory();
        QString fullFileName = fileName.isEmpty() ? generateFileName(form, ExportFormat::TXT) : fileName;
        if (!fullFileName.endsWith(".txt")) {
            fullFileName += ".txt";
        }
        
        QString filePath = QDir(outputPath).absoluteFilePath(fullFileName);
        
        // 生成文本内容
        QString txtContent = QString("%1\n%2\n").arg(form->getName(), form->getDescription());
        txtContent += QString("生成时间：%1\n").arg(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm"));
        txtContent += QString("=").repeated(50) + "\n\n";
        
        QMap<QString, QVariant> values = form->getAllValues();
        QList<FormField> fields = form->getFields();
        
        for (const FormField& field : fields) {
            if (values.contains(field.id) && !values[field.id].toString().isEmpty()) {
                QString value = values[field.id].toString();
                if (field.type == FieldType::Date) {
                    value = values[field.id].toDate().toString("yyyy年MM月dd日");
                }
                txtContent += QString("%1：%2\n").arg(field.label, value);
            }
        }
        
        txtContent += "\n" + QString("=").repeated(50) + "\n";
        txtContent += "此文件由HR智能问答系统自动生成\n";
        
        // 写入文件
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return ExportResult(false, "", "无法创建TXT文件");
        }
        
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << txtContent;
        file.close();
        
        QFileInfo fileInfo(filePath);
        ExportResult result(true, filePath, "");
        result.fileSize = formatFileSize(fileInfo.size());
        
        if (autoOpenFile_) {
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
        
        qDebug() << "[表单导出] TXT导出成功:" << filePath;
        return result;
        
    } catch (const std::exception& e) {
        return ExportResult(false, "", QString("TXT导出异常: %1").arg(e.what()));
    } catch (...) {
        return ExportResult(false, "", "TXT导出时发生未知错误");
    }
}

QString FormExporter::generateFileName(FormTemplate* form, ExportFormat format)
{
    if (!form) return QString("form_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
    
    QString baseName = form->getName();
    baseName.replace(QRegularExpression(R"([^\w\u4e00-\u9fff])"), "_"); // 只保留字母、数字、中文
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString extension = getFormatExtension(format);
    
    return QString("%1_%2%3").arg(baseName, timestamp, extension);
}

QString FormExporter::ensureOutputDirectory()
{
    QDir dir(outputDir_);
    if (!dir.exists()) {
        dir.mkpath(outputDir_);
    }
    return outputDir_;
}

QString FormExporter::formatFileSize(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    
    if (bytes < KB) {
        return QString("%1 B").arg(bytes);
    } else if (bytes < MB) {
        return QString("%1 KB").arg(bytes / KB);
    } else {
        return QString("%.1f MB").arg(static_cast<double>(bytes) / MB);
    }
}

QString FormExporter::getDefaultFileName(FormTemplate* form, ExportFormat format)
{
    FormExporter exporter;
    return exporter.generateFileName(form, format);
}

QString FormExporter::getFormatExtension(ExportFormat format)
{
    switch (format) {
    case ExportFormat::PDF: return ".pdf";
    case ExportFormat::HTML: return ".html";
    case ExportFormat::CSV: return ".csv";
    case ExportFormat::TXT: return ".txt";
    default: return ".txt";
    }
}

QString FormExporter::getFormatName(ExportFormat format)
{
    switch (format) {
    case ExportFormat::PDF: return "PDF文档";
    case ExportFormat::HTML: return "HTML网页";
    case ExportFormat::CSV: return "CSV表格";
    case ExportFormat::TXT: return "文本文件";
    default: return "未知格式";
    }
}

QStringList FormExporter::getSupportedFormats()
{
    return {"PDF文档", "HTML网页", "CSV表格", "文本文件"};
}

// ===== BatchFormExporter 实现 =====

BatchFormExporter::BatchFormExporter()
{
}

QList<ExportResult> BatchFormExporter::exportForms(const QList<FormTemplate*>& forms, 
                                                   ExportFormat format, 
                                                   const QString& outputDir)
{
    QList<ExportResult> results;
    
    if (!outputDir.isEmpty()) {
        exporter_.setOutputDirectory(outputDir);
    }
    
    for (int i = 0; i < forms.size(); ++i) {
        FormTemplate* form = forms[i];
        if (!form) continue;
        
        if (progressCallback_) {
            progressCallback_(i + 1, forms.size(), form->getName());
        }
        
        ExportResult result = exporter_.exportForm(form, format);
        results.append(result);
        
        // 小延迟以避免文件名冲突
        QThread::currentThread()->msleep(10);
    }
    
    return results;
}

ExportResult BatchFormExporter::generateSummaryReport(const QList<FormTemplate*>& forms, 
                                                     const QString& title)
{
    try {
        QString htmlContent = generateSummaryHTML(forms, title);
        
        QString outputPath = exporter_.getOutputDirectory();
        QString fileName = QString("%1_%2.html")
                          .arg(title)
                          .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        QString filePath = QDir(outputPath).absoluteFilePath(fileName);
        
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return ExportResult(false, "", "无法创建汇总报告文件");
        }
        
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << htmlContent;
        file.close();
        
        QFileInfo fileInfo(filePath);
        ExportResult result(true, filePath, "");
        result.fileSize = FormExporter::formatFileSize(fileInfo.size());
        
        return result;
        
    } catch (const std::exception& e) {
        return ExportResult(false, "", QString("汇总报告生成异常: %1").arg(e.what()));
    } catch (...) {
        return ExportResult(false, "", "汇总报告生成时发生未知错误");
    }
}

QString BatchFormExporter::generateSummaryHTML(const QList<FormTemplate*>& forms, const QString& title)
{
    QString html = QString(
        "<!DOCTYPE html>"
        "<html><head>"
        "<meta charset='UTF-8'>"
        "<title>%1</title>"
        "<style>%2</style>"
        "</head><body>"
    ).arg(title, FormExporter::DEFAULT_CSS_STYLE);
    
    html += QString(
        "<div class='form-header'>"
        "<h1 class='form-title'>%1</h1>"
        "<p class='form-description'>共包含 %2 个表单</p>"
        "</div>"
    ).arg(title).arg(forms.size());
    
    html += "<div class='form-content'>";
    
    for (int i = 0; i < forms.size(); ++i) {
        FormTemplate* form = forms[i];
        if (!form) continue;
        
        html += QString("<h2>%1. %2</h2>").arg(i + 1).arg(form->getName());
        html += QString("<p><em>%1</em></p>").arg(form->getDescription());
        
        html += "<table class='form-table'>";
        html += "<tr><th>字段</th><th>值</th></tr>";
        
        QMap<QString, QVariant> values = form->getAllValues();
        QList<FormField> fields = form->getFields();
        
        for (const FormField& field : fields) {
            if (values.contains(field.id) && !values[field.id].toString().isEmpty()) {
                QString value = values[field.id].toString();
                if (field.type == FieldType::Date) {
                    value = values[field.id].toDate().toString("yyyy年MM月dd日");
                }
                html += QString(
                    "<tr>"
                    "<td class='label-cell'>%1</td>"
                    "<td class='value-cell'>%2</td>"
                    "</tr>"
                ).arg(field.label, value);
            }
        }
        
        html += "</table><br>";
    }
    
    html += "</div>";
    html += QString("<div class='footer'>生成时间：%1</div>")
           .arg(QDateTime::currentDateTime().toString("yyyy年MM月dd日 hh:mm"));
    html += "</body></html>";
    
    return html;
}

// ===== FormPreviewer 实现 =====

QString FormPreviewer::generatePreviewHTML(FormTemplate* form)
{
    if (!form) return "<p>无效的表单</p>";
    
    QString style = previewStyle_.isEmpty() ? generatePreviewStyle() : previewStyle_;
    QString html = QString(
        "<html><head>"
        "<meta charset='UTF-8'>"
        "<style>%1</style>"
        "</head><body>"
    ).arg(style);
    
    html += form->toHtml();
    html += "</body></html>";
    
    return html;
}

QString FormPreviewer::generatePreviewText(FormTemplate* form)
{
    if (!form) return "无效的表单";
    
    QString text = QString("%1\n%2\n").arg(form->getName(), form->getDescription());
    text += QString("=").repeated(40) + "\n";
    
    QMap<QString, QVariant> values = form->getAllValues();
    QList<FormField> fields = form->getFields();
    
    for (const FormField& field : fields) {
        if (values.contains(field.id) && !values[field.id].toString().isEmpty()) {
            QString value = values[field.id].toString();
            if (field.type == FieldType::Date) {
                value = values[field.id].toDate().toString("yyyy年MM月dd日");
            }
            text += QString("%1: %2\n").arg(field.label, value);
        }
    }
    
    return text;
}

QString FormPreviewer::generatePreviewStyle()
{
    return FormExporter::DEFAULT_CSS_STYLE + R"(
        body { margin: 10px; }
        .form-table { font-size: 13px; }
        .form-header { margin-bottom: 20px; }
        .form-title { font-size: 22px; }
    )";
}
