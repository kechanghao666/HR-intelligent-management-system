#ifndef BATCHUPLOADDIALOG_H
#define BATCHUPLOADDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QListWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include "database.h"

class BatchUploadDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BatchUploadDialog(Database* database, QWidget* parent = nullptr);
    ~BatchUploadDialog();

private slots:
    void onSelectFilesClicked();
    void onSelectFolderClicked();
    void onUploadClicked();
    void onCancelClicked();
    void onPreviewClicked();
    void onClearListClicked();

private:
    void setupUI();
    void setupConnections();
    void applyStyles();
    void loadFilesFromFolder(const QString& folderPath);
    void processFiles();
    void addFileToList(const QString& filePath);
    void updateProgress(int current, int total);
    QString extractTitleFromFileName(const QString& fileName);
    QString determineCategoryFromContent(const QString& content);
    QString readFileContent(const QString& filePath);
    QString readDocxContent(const QString& filePath);
    bool isValidFile(const QString& filePath);

    Database* database_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QHBoxLayout* buttonLayout_;
    QFormLayout* formLayout_;
    
    // 文件选择
    QLineEdit* folderPathEdit_;
    QPushButton* selectFolderButton_;
    QPushButton* selectFilesButton_;
    
    // 设置选项
    QComboBox* defaultCategoryCombo_;
    QCheckBox* autoDetectCategoryCheck_;
    QCheckBox* overwriteExistingCheck_;
    QSpinBox* maxFileSizeSpin_;
    
    // 文件列表
    QListWidget* fileListWidget_;
    QPushButton* previewButton_;
    QPushButton* clearListButton_;
    
    // 进度显示
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    
    // 操作按钮
    QPushButton* uploadButton_;
    QPushButton* cancelButton_;
    
    // 数据
    QStringList selectedFiles_;
    QList<Document> documentsToUpload_;
    int totalFiles_;
    int processedFiles_;
};

#endif // BATCHUPLOADDIALOG_H
