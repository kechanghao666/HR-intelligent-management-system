#ifndef ATTACHMENTDIALOG_H
#define ATTACHMENTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QGroupBox>
#include <QScrollArea>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QPixmap>
#include <QTimer>
class DocumentProcessor;
class Database;

struct AttachmentInfo {
    QString fileName;
    QString filePath;
    QString fileType;
    qint64 fileSize;
    QPixmap preview;
    QString description;
};

class AttachmentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AttachmentDialog(Database* database, DocumentProcessor* processor, QWidget *parent = nullptr);
    ~AttachmentDialog();
    
    QList<AttachmentInfo> getSelectedAttachments() const;

private slots:
    void onBrowseClicked();
    void onRemoveClicked();
    void onPreviewClicked();
    void onUploadClicked();
    void onCancelClicked();
    void onAttachmentSelectionChanged();
    void updateUploadProgress();
    void onDocumentProcessed(const QString& fileName, bool success);
    void onProcessingProgress(int current, int total);
    void onDocumentIndexed(const QString& documentId, const QString& title);

private:
    void setupUI();
    void setupFileArea();
    void setupPreviewArea();
    void setupButtonArea();
    void addAttachment(const QString& filePath);
    void updateFileInfo();
    void updatePreview();
    bool isImageFile(const QString& filePath);
    bool isDocumentFile(const QString& filePath);
    QString formatFileSize(qint64 bytes);
    QPixmap generatePreviewIcon(const QString& filePath);
    void startUploadSimulation();
    void startRealUpload();
    
    // UI组件
    QVBoxLayout* mainLayout_;
    
    // 文件区域
    QGroupBox* fileGroup_;
    QListWidget* fileList_;
    QPushButton* browseBtn_;
    QPushButton* removeBtn_;
    QPushButton* previewBtn_;
    QLabel* fileSummaryLabel_;
    
    // 预览区域
    QGroupBox* previewGroup_;
    QScrollArea* previewArea_;
    QLabel* previewLabel_;
    QTextEdit* descriptionEdit_;
    
    // 进度区域
    QGroupBox* progressGroup_;
    QProgressBar* uploadProgress_;
    QLabel* progressLabel_;
    
    // 按钮区域
    QPushButton* uploadBtn_;
    QPushButton* cancelBtn_;
    
    // 数据
    QList<AttachmentInfo> attachments_;
    QMimeDatabase mimeDb_;
    Database* database_;
    DocumentProcessor* documentProcessor_;
    
    // 上传模拟
    QTimer* uploadTimer_;
    int currentProgress_;
    
    // 支持的文件类型
    QStringList supportedImageTypes_;
    QStringList supportedDocTypes_;
    QStringList supportedOtherTypes_;
};

#endif // ATTACHMENTDIALOG_H
