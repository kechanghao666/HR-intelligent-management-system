#ifndef FORMEXPORTER_H
#define FORMEXPORTER_H

#include <QString>
#include <QTextDocument>
#include <QPrinter>
#include <QStandardPaths>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QStringConverter>
#include <functional>
#include "formtemplate.h"

/**
 * @brief 导出格式枚举
 */
enum class ExportFormat {
    PDF,
    HTML,
    CSV,
    TXT
};

/**
 * @brief 导出结果结构
 */
struct ExportResult {
    bool success;
    QString filePath;
    QString errorMessage;
    QString fileSize;
    
    ExportResult(bool success = false, const QString& path = "", const QString& error = "")
        : success(success), filePath(path), errorMessage(error) {}
};

/**
 * @brief 表单导出器 - 负责将表单导出为各种格式
 */
class FormExporter
{
public:
    FormExporter();
    ~FormExporter() = default;

    // 主要导出接口
    ExportResult exportForm(FormTemplate* form, ExportFormat format, const QString& fileName = "");
    ExportResult exportToPDF(FormTemplate* form, const QString& fileName = "");
    ExportResult exportToHTML(FormTemplate* form, const QString& fileName = "");
    ExportResult exportToCSV(FormTemplate* form, const QString& fileName = "");
    ExportResult exportToTXT(FormTemplate* form, const QString& fileName = "");
    
    // 配置方法
    void setOutputDirectory(const QString& dir) { outputDir_ = dir; }
    QString getOutputDirectory() const { return outputDir_; }
    
    void setAutoOpenFile(bool autoOpen) { autoOpenFile_ = autoOpen; }
    bool getAutoOpenFile() const { return autoOpenFile_; }
    
    // 静态便捷方法
    static QString getDefaultFileName(FormTemplate* form, ExportFormat format);
    static QString getFormatExtension(ExportFormat format);
    static QString getFormatName(ExportFormat format);
    static QStringList getSupportedFormats();

    // 静态便捷方法
    static QString formatFileSize(qint64 bytes);

private:
    // 内部方法
    QString generateFileName(FormTemplate* form, ExportFormat format);
    QString ensureOutputDirectory();
    
    // HTML样式生成
    QString generateHTMLStyle();
    QString generatePDFStyle();
    
    // 数据成员
    QString outputDir_;
    bool autoOpenFile_;
    
public:
    // 样式常量
    static const QString DEFAULT_CSS_STYLE;
    static const QString PDF_CSS_STYLE;
};

/**
 * @brief 批量导出器 - 支持导出多个表单
 */
class BatchFormExporter
{
public:
    BatchFormExporter();
    ~BatchFormExporter() = default;
    
    // 批量导出
    QList<ExportResult> exportForms(const QList<FormTemplate*>& forms, 
                                   ExportFormat format, 
                                   const QString& outputDir = "");
    
    // 生成汇总报告
    ExportResult generateSummaryReport(const QList<FormTemplate*>& forms, 
                                      const QString& title = "表单汇总报告");
    
    // 进度回调
    using ProgressCallback = std::function<void(int current, int total, const QString& currentFile)>;
    void setProgressCallback(ProgressCallback callback) { progressCallback_ = callback; }

private:
    FormExporter exporter_;
    ProgressCallback progressCallback_;
    
    QString generateSummaryHTML(const QList<FormTemplate*>& forms, const QString& title);
};

/**
 * @brief 表单预览器 - 在导出前预览表单
 */
class FormPreviewer
{
public:
    FormPreviewer() = default;
    ~FormPreviewer() = default;
    
    // 生成预览内容
    QString generatePreviewHTML(FormTemplate* form);
    QString generatePreviewText(FormTemplate* form);
    
    // 预览配置
    void setPreviewStyle(const QString& cssStyle) { previewStyle_ = cssStyle; }
    QString getPreviewStyle() const { return previewStyle_; }

private:
    QString previewStyle_;
    QString generatePreviewStyle();
};

#endif // FORMEXPORTER_H
