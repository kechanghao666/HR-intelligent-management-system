#ifndef DOCUMENTVIEWER_H
#define DOCUMENTVIEWER_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextBrowser>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextCharFormat>
#include <QTextBlockFormat>
#include <QFont>
#include <QColor>
#include <QUrl>
#include <QDesktopServices>
#include "database.h"

class DocumentViewer : public QWidget
{
    Q_OBJECT

public:
    explicit DocumentViewer(Database* database, QWidget *parent = nullptr);
    ~DocumentViewer();

public slots:
    void loadDocument(const Document& document);
    void loadDocumentById(int documentId);
    void searchInDocument(const QString& keyword);
    void highlightSection(int section);
    void exportDocument();
    void printDocument();

private slots:
    void onSearchTextChanged(const QString& text);
    void onSearchButtonClicked();
    void onNextButtonClicked();
    void onPreviousButtonClicked();
    void onZoomInClicked();
    void onZoomOutClicked();
    void onZoomResetClicked();
    void onPrintClicked();
    void onExportClicked();
    void onOutlineItemClicked(QTreeWidgetItem* item, int column);
    void onOutlineContextMenu(const QPoint& pos);
    void onDocumentLinkClicked(const QUrl& url);
    void onTextSelectionChanged();

signals:
    void documentLoaded(const Document& document);
    void sectionHighlighted(int section);
    void searchResultFound(const QString& keyword, int position);

private:
    Database* database_;
    Document currentDocument_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QSplitter* mainSplitter_;
    
    // 左侧大纲面板
    QWidget* outlinePanel_;
    QVBoxLayout* outlineLayout_;
    QLabel* outlineTitleLabel_;
    QTreeWidget* outlineTree_;
    QPushButton* expandAllButton_;
    QPushButton* collapseAllButton_;
    
    // 右侧文档显示区域
    QWidget* documentPanel_;
    QVBoxLayout* documentLayout_;
    
    // 工具栏
    QHBoxLayout* toolbarLayout_;
    QLabel* searchLabel_;
    QLineEdit* searchEdit_;
    QPushButton* searchButton_;
    QPushButton* nextButton_;
    QPushButton* previousButton_;
    QLabel* zoomLabel_;
    QPushButton* zoomInButton_;
    QPushButton* zoomOutButton_;
    QPushButton* zoomResetButton_;
    QPushButton* printButton_;
    QPushButton* exportButton_;
    
    // 文档内容
    QTextBrowser* documentBrowser_;
    QLabel* documentInfoLabel_;
    
    // 搜索相关
    QList<int> searchPositions_;
    int currentSearchIndex_;
    QTextCharFormat originalFormat_;
    QTextCharFormat highlightFormat_;
    QTextCharFormat searchHighlightFormat_;
    
    // 缩放相关
    double zoomLevel_;
    QFont originalFont_;
    
    // 文档结构
    QList<QPair<QString, int>> documentSections_;
    QMap<QString, int> sectionPositions_;

    // 私有方法
    void setupUI();
    void setupConnections();
    void applyStyles();
    void setupToolbar();
    void setupOutline();
    void setupDocumentBrowser();
    void loadDocumentContent();
    void parseDocumentStructure();
    void buildOutlineTree();
    void highlightSearchResults();
    void clearSearchHighlights();
    void highlightSectionInText(int section);
    void updateDocumentInfo();
    void updateSearchButtons();
    void updateZoomButtons();
    void applyZoom();
    void findNextSearchResult();
    void findPreviousSearchResult();
    void scrollToPosition(int position);
    void exportToPDF();
    void exportToHTML();
    void exportToText();
    void printDocumentContent();
    
    // 文档处理方法
    QString formatDocumentContent();
    QString extractSectionContent(int section);
    QList<QString> extractDocumentSections();
    void processDocumentLinks();
    void addDocumentNavigation();
    
    // 样式方法
    void setupTextFormats();
    void applyDocumentStyle();
    void setupHighlightColors();
    void createCustomStyles();
    
    // 辅助方法
    void showMessage(const QString& title, const QString& message, QMessageBox::Icon icon = QMessageBox::Information);
    QString formatFileSize(qint64 size);
    QString formatDateTime(const QDateTime& dateTime);
    bool confirmAction(const QString& title, const QString& message);
    void logDocumentAction(const QString& action, const QString& details);
};

#endif // DOCUMENTVIEWER_H
