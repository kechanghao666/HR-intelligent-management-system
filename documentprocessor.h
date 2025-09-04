#ifndef DOCUMENTPROCESSOR_H
#define DOCUMENTPROCESSOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QStringConverter>
#include "database.h"

struct DocumentContent {
    QString fileName;
    QString filePath;
    QString fileType;
    QString title;
    QString content;
    QStringList keywords;
    QString category;
    QDateTime uploadTime;
    QString checksum;
};

struct DocumentChunk {
    QString id;
    QString documentId;
    QString content;
    QString title;
    int chunkIndex;
    QStringList keywords;
};

class DocumentProcessor : public QObject
{
    Q_OBJECT

public:
    explicit DocumentProcessor(Database* database, QObject *parent = nullptr);
    ~DocumentProcessor();

    // 文档处理主接口
    bool processDocument(const QString& filePath, const QString& category = "未分类");
    bool processDocumentBatch(const QStringList& filePaths, const QString& category = "未分类");
    
    // 内容提取
    QString extractTextFromPdf(const QString& filePath);
    QString extractTextFromWord(const QString& filePath);
    QString extractTextFromTxt(const QString& filePath);
    QString extractTextFromXml(const QString& filePath);
    QString extractTextFromJson(const QString& filePath);
    
    // 内容处理
    QStringList extractKeywords(const QString& content);
    QStringList splitIntoChunks(const QString& content, int maxLength = 500);
    QString generateDocumentId(const QString& filePath);
    
    // 数据库操作
    bool saveDocumentToDatabase(const DocumentContent& doc);
    bool saveDocumentChunks(const QString& documentId, const QStringList& chunks, const QString& title);
    QList<DocumentChunk> searchDocuments(const QString& query, const QString& category = "");
    
    // 相关性检索
    QList<DocumentChunk> findRelevantChunks(const QString& question, int maxResults = 5);
    double calculateSimilarity(const QString& text1, const QString& text2);
    
    // 支持的文件类型
    bool isSupportedFormat(const QString& filePath);
    QStringList getSupportedFormats();

signals:
    void documentProcessed(const QString& fileName, bool success);
    void processingProgress(int current, int total);
    void documentIndexed(const QString& documentId, const QString& title);

private:
    Database* database_;
    
    // 文本处理
    QString cleanText(const QString& text);
    QString normalizeText(const QString& text);
    QStringList tokenizeText(const QString& text);
    
    // 关键词提取
    QStringList extractKeywordsFromText(const QString& text);
    QStringList getStopWords();
    
    // 文件处理辅助
    QString calculateFileChecksum(const QString& filePath);
    bool isDocumentExists(const QString& checksum);
    QString detectDocumentType(const QString& content);
    
    // 支持的格式列表
    QStringList supportedFormats_;
    QStringList stopWords_;
};

#endif // DOCUMENTPROCESSOR_H
