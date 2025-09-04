#ifndef ADMINPANEL_H
#define ADMINPANEL_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QTableWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QDateEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QDateTime>
#include <QList>
#include <QMap>
#include "database.h"
#include "user.h"
#include "documentdialog.h"
#include "userdialog.h"
#include "qadialog.h"
#include "reportgenerator.h"
#include "batchuploaddialog.h"

class AdminPanel : public QWidget
{
    Q_OBJECT

public:
    explicit AdminPanel(Database* database, const User& currentUser, QWidget *parent = nullptr);
    ~AdminPanel();

public slots:
    void refreshData();
    void exportData();
    void showStatistics();
    void forceRefreshUI();

private slots:
    // 用户管理
    void onAddUserClicked();
    void onEditUserClicked();
    void onDeleteUserClicked();
    void onActivateUserClicked();
    void onUserTableContextMenu(const QPoint& pos);
    void onUserTableSelectionChanged();
    void onUserSearchTextChanged(const QString& text);
    void onUserRoleFilterChanged(const QString& role);
    void onUserStatusFilterChanged(const QString& status);
    
    // 文档管理
    void onAddDocumentClicked();
    void onEditDocumentClicked();
    void onDeleteDocumentClicked();
    void onUploadDocumentClicked();
    void onDocumentTableContextMenu(const QPoint& pos);
    void onDocumentTableSelectionChanged();
    void onDocumentSearchTextChanged(const QString& text);
    void onDocumentCategoryFilterChanged(const QString& category);
    
    // 问答管理
    void onOrganizeQAClicked();
    void onExportQAClicked();
    void onQATableContextMenu(const QPoint& pos);
    void onQATableSelectionChanged();
    void onQASearchTextChanged(const QString& text);
    void onQACategoryFilterChanged(const QString& category);
    
    // 统计和报表
    void onGenerateReportClicked();
    void onExportReportClicked();
    void onDateRangeChanged();
    void onRefreshStatisticsClicked();
    
    // 系统设置
    void onSaveSettingsClicked();
    void onResetSettingsClicked();
    void onBackupDataClicked();
    void onRestoreDataClicked();

signals:
    void dataChanged();
    void userStatusChanged(const QString& employeeId, UserStatus status);

private:
    Database* database_;
    User currentUser_;
    
    // 主布局
    QVBoxLayout* mainLayout_;
    QTabWidget* tabWidget_;
    
    // 用户管理标签页
    QWidget* userTab_;
    QVBoxLayout* userLayout_;
    QGroupBox* userSearchGroup_;
    QHBoxLayout* userSearchLayout_;
    QLineEdit* userSearchEdit_;
    QComboBox* userRoleFilter_;
    QComboBox* userStatusFilter_;
    QPushButton* userRefreshButton_;
    QTableWidget* userTable_;
    QGroupBox* userActionGroup_;
    QHBoxLayout* userActionLayout_;
    QPushButton* addUserButton_;
    QPushButton* editUserButton_;
    QPushButton* deleteUserButton_;
    QPushButton* activateUserButton_;
    
    // 文档管理标签页
    QWidget* documentTab_;
    QVBoxLayout* documentLayout_;
    QGroupBox* documentSearchGroup_;
    QHBoxLayout* documentSearchLayout_;
    QLineEdit* documentSearchEdit_;
    QComboBox* documentCategoryFilter_;
    QPushButton* documentRefreshButton_;
    QTableWidget* documentTable_;
    QGroupBox* documentActionGroup_;
    QHBoxLayout* documentActionLayout_;
    QPushButton* addDocumentButton_;
    QPushButton* editDocumentButton_;
    QPushButton* deleteDocumentButton_;
    QPushButton* uploadDocumentButton_;
    
    // 问答管理标签页
    QWidget* qaTab_;
    QVBoxLayout* qaLayout_;
    QGroupBox* qaSearchGroup_;
    QHBoxLayout* qaSearchLayout_;
    QLineEdit* qaSearchEdit_;
    QComboBox* qaCategoryFilter_;
    QPushButton* qaRefreshButton_;
    QTableWidget* qaTable_;
    QGroupBox* qaActionGroup_;
    QHBoxLayout* qaActionLayout_;
    QPushButton* organizeQAButton_;
    QPushButton* exportQAButton_;
    
    // 统计报表标签页
    QWidget* statisticsTab_;
    QVBoxLayout* statisticsLayout_;
    QGroupBox* statisticsFilterGroup_;
    QHBoxLayout* statisticsFilterLayout_;
    QLabel* dateRangeLabel_;
    QDateEdit* startDateEdit_;
    QDateEdit* endDateEdit_;
    QPushButton* refreshStatisticsButton_;
    QPushButton* generateReportButton_;
    QPushButton* exportReportButton_;
    QGroupBox* statisticsDisplayGroup_;
    QVBoxLayout* statisticsDisplayLayout_;
    QLabel* totalUsersLabel_;
    QLabel* totalDocumentsLabel_;
    QLabel* totalQuestionsLabel_;
    QLabel* activeUsersLabel_;
    QLabel* pendingUsersLabel_;
    
    // 系统设置标签页
    QWidget* settingsTab_;
    QVBoxLayout* settingsLayout_;
    QGroupBox* generalSettingsGroup_;
    QGridLayout* generalSettingsLayout_;
    QLabel* autoBackupLabel_;
    QCheckBox* autoBackupCheck_;
    QLabel* backupIntervalLabel_;
    QSpinBox* backupIntervalSpin_;
    QLabel* sessionTimeoutLabel_;
    QSpinBox* sessionTimeoutSpin_;
    QGroupBox* dataManagementGroup_;
    QHBoxLayout* dataManagementLayout_;
    QPushButton* backupDataButton_;
    QPushButton* restoreDataButton_;
    QPushButton* saveSettingsButton_;
    QPushButton* resetSettingsButton_;
    
    // 数据
    QList<User> userList_;            // filtered
    QList<User> allUsers_;            // full
    QList<Document> documentList_;    // filtered
    QList<Document> allDocuments_;    // full
    QList<QuestionRecord> qaList_;    // filtered
    QList<QuestionRecord> allQAs_;    // full
    
    // 状态
    QTimer* refreshTimer_;
    bool dataChanged_;
    ReportGenerator* reportGenerator_;

    // 私有方法
    void setupUI();
    void setupConnections();
    void applyStyles();
    void setupUserTab();
    void setupDocumentTab();
    void setupQATab();
    void setupStatisticsTab();
    void setupSettingsTab();
    void setupTables();
    void setupContextMenus();
    
    // 用户管理方法
    void loadUsers();
    void refreshUserTable();
    void filterUsers();
    void showUserDialog(const User& user = User(), bool isEdit = false);
    void deleteUser(const QString& employeeId);
    void activateUser(const QString& employeeId);
    
    // 文档管理方法
    void loadDocuments();
    void refreshDocumentTable();
    void filterDocuments();
    void showDocumentDialog(const Document& document = Document(), bool isEdit = false);
    void deleteDocument(int documentId);
    void uploadDocument();
    
    // 问答管理方法
    void loadQAs();
    void refreshQATable();
    void filterQAs();
    void showQADialog(const QuestionRecord& qa = QuestionRecord(), bool isEdit = false);
    void deleteQA(int qaId);
    
    // 统计方法
    void loadStatistics();
    void generateReport();
    void exportReport();
    void updateStatisticsDisplay();
    QWidget* createStatsCard(QLabel* numberLabel, QLabel* textLabel, const QString& color);
    
    // 设置方法
    void loadSettings();
    void saveSettings();
    void backupData();
    void restoreData();
    
    // 辅助方法
    void showMessage(const QString& title, const QString& message, QMessageBox::Icon icon = QMessageBox::Information);
    bool confirmAction(const QString& title, const QString& message);
    QString formatDateTime(const QDateTime& dateTime);
    void logAdminAction(const QString& action, const QString& details);
};

#endif // ADMINPANEL_H
