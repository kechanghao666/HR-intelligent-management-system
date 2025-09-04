#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QList>
#include <QDateTime>
#include "user.h"

class Document;
class QuestionRecord;

class Database : public QObject
{
    Q_OBJECT

public:
    explicit Database(QObject *parent = nullptr);
    ~Database();

    // 数据库初始化
    bool initialize();
    bool createTables();
    bool isConnected() const;

    // 用户管理
    bool addUser(const User& user, const QString& password);
    bool updateUser(const User& user);
    bool deleteUser(const QString& employeeId);
    User getUser(const QString& employeeId);
    QList<User> getAllUsers();
    QList<User> getUsersByRole(UserRole role);
    QList<User> getPendingUsers();
    bool activateUser(const QString& employeeId);
    bool updateUserStatus(const QString& employeeId, UserStatus status);
    bool updateLastLoginTime(const QString& employeeId);

    // 密码管理
    bool setPassword(const QString& employeeId, const QString& password);
    bool verifyPassword(const QString& employeeId, const QString& password);
    bool resetPassword(const QString& employeeId, const QString& newPassword);

    // 文档管理
    bool addDocument(const Document& document);
    bool updateDocument(const Document& document);
    bool deleteDocument(int documentId);
    Document getDocument(int documentId);
    QList<Document> getAllDocuments();
    QList<Document> getDocumentsByCategory(const QString& category);

    // 问答记录管理
    bool addQuestionRecord(const QuestionRecord& record);
    bool updateQuestionRecord(const QuestionRecord& record);
    bool deleteQuestionRecord(int recordId);
    QList<QuestionRecord> getQuestionRecords(const QString& employeeId = QString());
    QList<QuestionRecord> getQuestionRecordsByDateRange(const QDateTime& start, const QDateTime& end);
    QList<QuestionRecord> getQuestionRecordsByCategory(const QString& category);

    // 统计查询
    int getUserCount(UserRole role = UserRole::Employee);
    int getDocumentCount(const QString& category = QString());
    int getQuestionCount(const QString& employeeId = QString());
    QMap<QString, int> getQuestionCategoryStats();
    QMap<QString, int> getUserActivityStats();
    
    // 重新加载示例数据（用于调试）
    void reloadSampleData();

private:
    QSqlDatabase db_;
    QString dbPath_;

    // 私有辅助方法
    bool executeQuery(const QString& sql, const QVariantMap& params = QVariantMap());
    QSqlQuery prepareQuery(const QString& sql);
    void loadInitialData();
    void logError(const QString& operation, const QSqlError& error);
};

// 文档类
class Document {
public:
    Document();
    Document(const QString& title, const QString& content, const QString& category);

    int getId() const { return id_; }
    QString getTitle() const { return title_; }
    QString getContent() const { return content_; }
    QString getCategory() const { return category_; }
    QString getFileName() const { return fileName_; }
    QString getFilePath() const { return filePath_; }
    QString getFileType() const { return fileType_; }
    QString getKeywords() const { return keywords_; }
    QString getChecksum() const { return checksum_; }
    QDateTime getCreateTime() const { return createTime_; }
    QDateTime getUpdateTime() const { return updateTime_; }

    void setId(int id) { id_ = id; }
    void setTitle(const QString& title) { title_ = title; }
    void setContent(const QString& content) { content_ = content; }
    void setCategory(const QString& category) { category_ = category; }
    void setFileName(const QString& fileName) { fileName_ = fileName; }
    void setFilePath(const QString& filePath) { filePath_ = filePath; }
    void setFileType(const QString& fileType) { fileType_ = fileType; }
    void setKeywords(const QString& keywords) { keywords_ = keywords; }
    void setChecksum(const QString& checksum) { checksum_ = checksum; }
    void setCreateTime(const QDateTime& time) { createTime_ = time; }
    void setUpdateTime(const QDateTime& time) { updateTime_ = time; }
    
    // 比较操作符
    bool operator==(const Document& other) const { return id_ == other.id_; }
    bool operator!=(const Document& other) const { return id_ != other.id_; }

private:
    int id_;
    QString title_;
    QString content_;
    QString category_;
    QString fileName_;
    QString filePath_;
    QString fileType_;
    QString keywords_;
    QString checksum_;
    QDateTime createTime_;
    QDateTime updateTime_;
};

// 问答记录类
class QuestionRecord {
public:
    QuestionRecord();
    QuestionRecord(const QString& employeeId, const QString& question, 
                   const QString& answer, const QString& category);

    int getId() const { return id_; }
    QString getEmployeeId() const { return employeeId_; }
    QString getQuestion() const { return question_; }
    QString getAnswer() const { return answer_; }
    QString getCategory() const { return category_; }
    QDateTime getCreateTime() const { return createTime_; }

    void setId(int id) { id_ = id; }
    void setEmployeeId(const QString& employeeId) { employeeId_ = employeeId; }
    void setQuestion(const QString& question) { question_ = question; }
    void setAnswer(const QString& answer) { answer_ = answer; }
    void setCategory(const QString& category) { category_ = category; }
    void setCreateTime(const QDateTime& time) { createTime_ = time; }

private:
    int id_;
    QString employeeId_;
    QString question_;
    QString answer_;
    QString category_;
    QDateTime createTime_;
};

#endif // DATABASE_H
