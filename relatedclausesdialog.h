#ifndef RELATEDCLAUSESDIALOG_H
#define RELATEDCLAUSESDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTextBrowser>
#include <QSplitter>
#include <QGroupBox>
#include <QProgressBar>
#include <QDateTime>
#include "documentprocessor.h"

struct RelatedClause {
    QString id;
    QString title;
    QString content;
    QString source;
    double relevanceScore;
    QString category;
    QDateTime lastModified;
};

class RelatedClausesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RelatedClausesDialog(const QString& question, DocumentProcessor* processor, QWidget *parent = nullptr);
    ~RelatedClausesDialog();

private slots:
    void onClauseSelected();
    void onViewFullDocument();
    void onCopyClause();
    void onSearchCompleted();

private:
    void setupUI();
    void setupClauseList();
    void setupContentView();
    void setupButtons();
    void searchRelatedClauses();
    void displayClauses(const QList<RelatedClause>& clauses);
    void formatClauseContent(const RelatedClause& clause);
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QHBoxLayout* buttonLayout_;
    QSplitter* splitter_;
    
    // 左侧条款列表
    QGroupBox* clauseListGroup_;
    QListWidget* clauseList_;
    QLabel* statusLabel_;
    QProgressBar* searchProgress_;
    
    // 右侧详细内容
    QGroupBox* contentGroup_;
    QTextBrowser* contentBrowser_;
    QLabel* sourceLabel_;
    QLabel* relevanceLabel_;
    
    // 按钮
    QPushButton* viewFullButton_;
    QPushButton* copyButton_;
    QPushButton* closeButton_;
    
    // 数据
    QString question_;
    DocumentProcessor* processor_;
    QList<RelatedClause> clauses_;
    RelatedClause currentClause_;
};

#endif // RELATEDCLAUSESDIALOG_H
