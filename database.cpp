#include "database.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QDateTime>

Database::Database(QObject *parent)
    : QObject(parent)
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    dbPath_ = dataDir + "/hrsystem.db";
}

Database::~Database()
{
    if (db_.isOpen()) {
        db_.close();
    }
}

bool Database::initialize()
{
    try {
        // 【异常处理修复】确保数据目录存在
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir;
        if (!dir.mkpath(dataDir)) {
            qCritical() << "[数据库初始化] 无法创建数据目录:" << dataDir;
            return false;
        }
        
        db_ = QSqlDatabase::addDatabase("QSQLITE");
        db_.setDatabaseName(dbPath_);
        
        if (!db_.open()) {
            qCritical() << "[数据库初始化] 无法打开数据库:" << db_.lastError().text();
            return false;
        }
        
        // 设置SQLite编码为UTF-8和其他配置
        QSqlQuery query;
        if (!query.exec("PRAGMA encoding = 'UTF-8'")) {
            qWarning() << "[数据库配置] 无法设置数据库编码:" << query.lastError().text();
        }
        if (!query.exec("PRAGMA foreign_keys = ON")) {
            qWarning() << "[数据库配置] 无法启用外键约束:" << query.lastError().text();
        }
        
        // 增加连接超时和缓存设置
        if (!query.exec("PRAGMA synchronous = NORMAL")) {
            qWarning() << "[数据库配置] 设置同步模式失败:" << query.lastError().text();
        }
        if (!query.exec("PRAGMA cache_size = 10000")) {
            qWarning() << "[数据库配置] 设置缓存大小失败:" << query.lastError().text();
        }
        if (!query.exec("PRAGMA temp_store = MEMORY")) {
            qWarning() << "[数据库配置] 设置临时存储失败:" << query.lastError().text();
        }
        
        bool result = createTables();
        if (result) {
            loadInitialData();
        } else {
            qCritical() << "[数据库初始化] 创建表失败";
        }
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "[数据库初始化] 发生异常:" << e.what();
        return false;
    } catch (...) {
        qCritical() << "[数据库初始化] 发生未知异常";
        return false;
    }
}

bool Database::createTables()
{
    QSqlQuery query;
    
    // 创建用户表
    if (!query.exec("CREATE TABLE IF NOT EXISTS users ("
                   "employee_id TEXT PRIMARY KEY,"
                   "name TEXT NOT NULL,"
                   "department TEXT NOT NULL,"
                   "role INTEGER NOT NULL,"
                   "status INTEGER NOT NULL,"
                   "create_time DATETIME NOT NULL,"
                   "last_login_time DATETIME"
                   ")")) {
        logError("创建用户表", query.lastError());
        return false;
    }
    
    // 创建密码表
    if (!query.exec("CREATE TABLE IF NOT EXISTS passwords ("
                   "employee_id TEXT PRIMARY KEY,"
                   "password_hash TEXT NOT NULL,"
                   "FOREIGN KEY(employee_id) REFERENCES users(employee_id)"
                   ")")) {
        logError("创建密码表", query.lastError());
        return false;
    }
    
    // 创建文档表
    if (!query.exec("CREATE TABLE IF NOT EXISTS documents ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "title TEXT NOT NULL,"
                   "content TEXT NOT NULL,"
                   "category TEXT NOT NULL,"
                   "file_name TEXT,"
                   "file_path TEXT,"
                   "file_type TEXT,"
                   "keywords TEXT,"
                   "checksum TEXT,"
                   "create_time DATETIME NOT NULL,"
                   "update_time DATETIME NOT NULL"
                   ")")) {
        logError("创建文档表", query.lastError());
        return false;
    }
    
    // 创建问答记录表
    if (!query.exec("CREATE TABLE IF NOT EXISTS question_records ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "employee_id TEXT NOT NULL,"
                   "question TEXT NOT NULL,"
                   "answer TEXT NOT NULL,"
                   "category TEXT NOT NULL,"
                   "create_time DATETIME NOT NULL,"
                   "FOREIGN KEY(employee_id) REFERENCES users(employee_id)"
                   ")")) {
        logError("创建问答记录表", query.lastError());
        return false;
    }
    
    // 创建系统设置表
    if (!query.exec("CREATE TABLE IF NOT EXISTS settings ("
                   "key TEXT PRIMARY KEY,"
                   "value TEXT NOT NULL"
                   ")")) {
        logError("创建设置表", query.lastError());
        return false;
    }
    
    return true;
}

bool Database::updateQuestionRecord(const QuestionRecord& record)
{
    QSqlQuery query = prepareQuery(
        "UPDATE question_records SET employee_id = ?, question = ?, answer = ?, category = ? WHERE id = ?"
    );
    
    query.addBindValue(record.getEmployeeId());
    query.addBindValue(record.getQuestion());
    query.addBindValue(record.getAnswer());
    query.addBindValue(record.getCategory());
    query.addBindValue(record.getId());
    
    if (!query.exec()) {
        logError("更新问答记录", query.lastError());
        return false;
    }
    
    return true;
}

bool Database::deleteQuestionRecord(int recordId)
{
    QSqlQuery query = prepareQuery("DELETE FROM question_records WHERE id = ?");
    query.addBindValue(recordId);
    
    if (!query.exec()) {
        logError("删除问答记录", query.lastError());
        return false;
    }
    
    return true;
}

bool Database::isConnected() const
{
    return db_.isOpen();
}

// 用户管理方法
bool Database::addUser(const User& user, const QString& password)
{
    // 检查数据库连接状态
    if (!db_.isOpen()) {
        qDebug() << "❌ 数据库未连接，无法添加用户";
        return false;
    }
    
    // 开始事务
    if (!db_.transaction()) {
        qDebug() << "❌ 无法开始事务";
        return false;
    }
    
    QSqlQuery query;
    query.prepare("INSERT INTO users (employee_id, name, department, role, status, create_time) "
                 "VALUES (?, ?, ?, ?, ?, ?)");
    query.addBindValue(user.getEmployeeId());
    query.addBindValue(user.getName());
    query.addBindValue(user.getDepartment());
    query.addBindValue(static_cast<int>(user.getRole()));
    query.addBindValue(static_cast<int>(user.getStatus()));
    query.addBindValue(user.getCreateTime());
    
    if (!query.exec()) {
        logError("添加用户", query.lastError());
        db_.rollback();
        return false;
    }
    
    // 添加密码
    if (!setPassword(user.getEmployeeId(), password)) {
        logError("添加密码", QSqlError());
        db_.rollback();
        return false;
    }
    
    // 提交事务
    if (!db_.commit()) {
        logError("提交事务", db_.lastError());
        db_.rollback();
        return false;
    }
    
    qDebug() << "✅ 用户添加成功:" << user.getEmployeeId();
    return true;
}

bool Database::updateUser(const User& user)
{
    // 检查数据库连接状态
    if (!db_.isOpen()) {
        qDebug() << "❌ 数据库未连接，无法更新用户";
        return false;
    }
    
    // 验证输入参数
    if (user.getEmployeeId().trimmed().isEmpty()) {
        qDebug() << "❌ 用户ID为空，无法更新";
        return false;
    }
    
    QSqlQuery query;
    query.prepare("UPDATE users SET name=?, department=?, role=?, status=?, last_login_time=? "
                 "WHERE employee_id=?");
    query.addBindValue(user.getName());
    query.addBindValue(user.getDepartment());
    query.addBindValue(static_cast<int>(user.getRole()));
    query.addBindValue(static_cast<int>(user.getStatus()));
    query.addBindValue(user.getLastLoginTime());
    query.addBindValue(user.getEmployeeId());
    
    bool result = query.exec();
    if (!result) {
        logError("更新用户", query.lastError());
        return false;
    }
    
    // 检查是否有行被更新
    if (query.numRowsAffected() == 0) {
        qDebug() << "⚠️ 未找到要更新的用户:" << user.getEmployeeId();
        return false;
    }
    
    qDebug() << "✅ 用户更新成功:" << user.getEmployeeId();
    return true;
}

bool Database::deleteUser(const QString& employeeId)
{
    // 开始事务
    if (!db_.transaction()) {
        qDebug() << "❌ 无法开始事务";
        return false;
    }
    
    // 删除密码记录
    QSqlQuery passwordQuery;
    passwordQuery.prepare("DELETE FROM passwords WHERE employee_id=?");
    passwordQuery.addBindValue(employeeId);
    
    if (!passwordQuery.exec()) {
        logError("删除用户密码", passwordQuery.lastError());
        db_.rollback();
        return false;
    }
    
    // 删除用户记录
    QSqlQuery userQuery;
    userQuery.prepare("DELETE FROM users WHERE employee_id=?");
    userQuery.addBindValue(employeeId);
    
    if (!userQuery.exec()) {
        logError("删除用户", userQuery.lastError());
        db_.rollback();
        return false;
    }
    
    // 提交事务
    if (!db_.commit()) {
        logError("提交删除用户事务", db_.lastError());
        db_.rollback();
        return false;
    }
    
    qDebug() << "✅ 用户删除成功:" << employeeId;
    return true;
}

User Database::getUser(const QString& employeeId)
{
    qDebug() << "获取用户信息:" << employeeId;
    
    // 检查数据库连接状态
    if (!db_.isOpen()) {
        qDebug() << "❌ 数据库未连接，无法获取用户";
        return User();
    }
    
    // 验证输入参数
    if (employeeId.trimmed().isEmpty()) {
        qDebug() << "❌ 用户ID为空";
        return User();
    }
    
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE employee_id=?");
    query.addBindValue(employeeId.trimmed());
    
    if (query.exec()) {
        if (query.next()) {
            User user;
            user.setEmployeeId(query.value("employee_id").toString());
            user.setName(query.value("name").toString());
            user.setDepartment(query.value("department").toString());
            user.setRole(static_cast<UserRole>(query.value("role").toInt()));
            user.setStatus(static_cast<UserStatus>(query.value("status").toInt()));
            user.setCreateTime(query.value("create_time").toDateTime());
            user.setLastLoginTime(query.value("last_login_time").toDateTime());
            
            qDebug() << "✅ 找到用户:" << user.getEmployeeId() << user.getName() 
                     << "角色:" << static_cast<int>(user.getRole()) 
                     << "状态:" << static_cast<int>(user.getStatus())
                     << "可登录:" << user.canLogin();
            
            return user;
        } else {
            qDebug() << "❌ 未找到用户:" << employeeId;
            return User();
        }
    } else {
        logError("获取用户", query.lastError());
        return User();
    }
}

QList<User> Database::getAllUsers()
{
    QList<User> users;
    QSqlQuery query("SELECT * FROM users ORDER BY create_time DESC");
    
    if (query.exec()) {
        while (query.next()) {
            User user;
            user.setEmployeeId(query.value("employee_id").toString());
            user.setName(query.value("name").toString());
            user.setDepartment(query.value("department").toString());
            user.setRole(static_cast<UserRole>(query.value("role").toInt()));
            user.setStatus(static_cast<UserStatus>(query.value("status").toInt()));
            user.setCreateTime(query.value("create_time").toDateTime());
            user.setLastLoginTime(query.value("last_login_time").toDateTime());
            users.append(user);
        }
    } else {
        logError("getAllUsers", query.lastError());
    }
    
    return users;
}

QList<User> Database::getUsersByRole(UserRole role)
{
    QList<User> users;
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE role=? ORDER BY create_time DESC");
    query.addBindValue(static_cast<int>(role));
    
    if (query.exec()) {
        while (query.next()) {
            User user;
            user.setEmployeeId(query.value("employee_id").toString());
            user.setName(query.value("name").toString());
            user.setDepartment(query.value("department").toString());
            user.setRole(static_cast<UserRole>(query.value("role").toInt()));
            user.setStatus(static_cast<UserStatus>(query.value("status").toInt()));
            user.setCreateTime(query.value("create_time").toDateTime());
            user.setLastLoginTime(query.value("last_login_time").toDateTime());
            users.append(user);
        }
    } else {
        logError("getUsersByRole", query.lastError());
    }
    
    return users;
}

QList<User> Database::getPendingUsers()
{
    QList<User> users;
    QSqlQuery query;
    query.prepare("SELECT * FROM users WHERE status=? ORDER BY create_time DESC");
    query.addBindValue(static_cast<int>(UserStatus::Pending));
    
    if (query.exec()) {
        while (query.next()) {
            User user;
            user.setEmployeeId(query.value("employee_id").toString());
            user.setName(query.value("name").toString());
            user.setDepartment(query.value("department").toString());
            user.setRole(static_cast<UserRole>(query.value("role").toInt()));
            user.setStatus(static_cast<UserStatus>(query.value("status").toInt()));
            user.setCreateTime(query.value("create_time").toDateTime());
            user.setLastLoginTime(query.value("last_login_time").toDateTime());
            users.append(user);
        }
    } else {
        logError("getPendingUsers", query.lastError());
    }
    
    return users;
}

bool Database::activateUser(const QString& employeeId)
{
    return updateUserStatus(employeeId, UserStatus::Active);
}

bool Database::updateUserStatus(const QString& employeeId, UserStatus status)
{
    // 检查数据库连接状态
    if (!db_.isOpen()) {
        qDebug() << "❌ 数据库未连接，无法更新用户状态";
        return false;
    }
    
    QSqlQuery query;
    query.prepare("UPDATE users SET status=? WHERE employee_id=?");
    query.addBindValue(static_cast<int>(status));
    query.addBindValue(employeeId);
    
    if (!query.exec()) {
        logError("更新用户状态", query.lastError());
        return false;
    }
    
    qDebug() << "✅ 用户状态更新成功:" << employeeId << "新状态:" << static_cast<int>(status);
    return true;
}

bool Database::updateLastLoginTime(const QString& employeeId)
{
    QSqlQuery query;
    query.prepare("UPDATE users SET last_login_time=? WHERE employee_id=?");
    query.addBindValue(QDateTime::currentDateTime());
    query.addBindValue(employeeId);
    
    return query.exec();
}

// 密码管理方法
bool Database::setPassword(const QString& employeeId, const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashString = hash.toHex();
    
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO passwords (employee_id, password_hash) VALUES (?, ?)");
    query.addBindValue(employeeId);
    query.addBindValue(hashString);
    
    return query.exec();
}

bool Database::verifyPassword(const QString& employeeId, const QString& password)
{
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    QString hashString = hash.toHex();
    
    // 注意：不要在生产环境中记录密码信息
    qDebug() << "🔐 验证密码:" << employeeId << "（密码已隐藏）";
    
    QSqlQuery query;
    query.prepare("SELECT password_hash FROM passwords WHERE employee_id=?");
    query.addBindValue(employeeId);
    
    if (query.exec() && query.next()) {
        QString storedHash = query.value("password_hash").toString();
        bool match = (storedHash == hashString);
        qDebug() << "存储哈希:" << storedHash << "匹配结果:" << (match ? "成功" : "失败");
        return match;
    } else {
        qDebug() << "❌ 未找到用户密码记录:" << employeeId;
        return false;
    }
}

bool Database::resetPassword(const QString& employeeId, const QString& newPassword)
{
    return setPassword(employeeId, newPassword);
}

// 文档管理方法
bool Database::addDocument(const Document& document)
{
    if (!db_.isOpen()) {
        qWarning() << "数据库未打开，无法添加文档:" << document.getTitle();
        return false;
    }
    
    qDebug() << "准备添加文档到数据库:" << document.getTitle();
    qDebug() << "文档内容长度:" << document.getContent().length();
    qDebug() << "文档类别:" << document.getCategory();
    
    QSqlQuery query;
    query.prepare("INSERT INTO documents (title, content, category, file_name, file_path, file_type, keywords, checksum, create_time, update_time) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(document.getTitle());
    query.addBindValue(document.getContent());
    query.addBindValue(document.getCategory());
    query.addBindValue(document.getFileName());
    query.addBindValue(document.getFilePath());
    query.addBindValue(document.getFileType());
    query.addBindValue(document.getKeywords());
    query.addBindValue(document.getChecksum());
    query.addBindValue(document.getCreateTime());
    query.addBindValue(document.getUpdateTime());
    
    bool result = query.exec();
    if (!result) {
        qWarning() << "数据库添加文档失败 - SQL错误:" << query.lastError().text();
        qWarning() << "执行的SQL:" << query.executedQuery();
        logError("添加文档", query.lastError());
        
        // 兼容旧版数据库：尝试用精简字段再次插入，避免因缺少列导致失败
        QSqlQuery compatQuery;
        compatQuery.prepare("INSERT INTO documents (title, content, category, create_time, update_time) VALUES (?, ?, ?, ?, ?)");
        compatQuery.addBindValue(document.getTitle());
        compatQuery.addBindValue(document.getContent());
        compatQuery.addBindValue(document.getCategory());
        compatQuery.addBindValue(document.getCreateTime());
        compatQuery.addBindValue(document.getUpdateTime());
        
        bool compatResult = compatQuery.exec();
        if (!compatResult) {
            qWarning() << "兼容插入仍失败 - SQL错误:" << compatQuery.lastError().text();
            logError("兼容添加文档", compatQuery.lastError());
            return false;
        } else {
            qInfo() << "采用兼容模式成功添加文档:" << document.getTitle();
            return true;
        }
    } else {
        qInfo() << "文档成功添加到数据库:" << document.getTitle();
    }
    
    return result;
}

bool Database::updateDocument(const Document& document)
{
    QSqlQuery query;
    query.prepare("UPDATE documents SET title=?, content=?, category=?, file_name=?, file_path=?, file_type=?, keywords=?, checksum=?, update_time=? "
                 "WHERE id=?");
    query.addBindValue(document.getTitle());
    query.addBindValue(document.getContent());
    query.addBindValue(document.getCategory());
    query.addBindValue(document.getFileName());
    query.addBindValue(document.getFilePath());
    query.addBindValue(document.getFileType());
    query.addBindValue(document.getKeywords());
    query.addBindValue(document.getChecksum());
    query.addBindValue(document.getUpdateTime());
    query.addBindValue(document.getId());
    
    bool result = query.exec();
    if (!result) {
        logError("更新文档", query.lastError());
    }
    
    return result;
}

bool Database::deleteDocument(int documentId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM documents WHERE id=?");
    query.addBindValue(documentId);
    
    return query.exec();
}

Document Database::getDocument(int documentId)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM documents WHERE id=?");
    query.addBindValue(documentId);
    
    if (query.exec() && query.next()) {
        Document document;
        document.setId(query.value("id").toInt());
        document.setTitle(query.value("title").toString());
        document.setContent(query.value("content").toString());
        document.setCategory(query.value("category").toString());
        document.setFileName(query.value("file_name").toString());
        document.setFilePath(query.value("file_path").toString());
        document.setFileType(query.value("file_type").toString());
        document.setKeywords(query.value("keywords").toString());
        document.setChecksum(query.value("checksum").toString());
        document.setCreateTime(query.value("create_time").toDateTime());
        document.setUpdateTime(query.value("update_time").toDateTime());
        return document;
    } else {
        logError("获取文档", query.lastError());
    }
    
    return Document();
}

QList<Document> Database::getAllDocuments()
{
    QList<Document> documents;
    QSqlQuery query("SELECT * FROM documents ORDER BY update_time DESC");
    
    if (query.exec()) {
        while (query.next()) {
            Document document;
            document.setId(query.value("id").toInt());
            document.setTitle(query.value("title").toString());
            document.setContent(query.value("content").toString());
            document.setCategory(query.value("category").toString());
            document.setFileName(query.value("file_name").toString());
            document.setFilePath(query.value("file_path").toString());
            document.setFileType(query.value("file_type").toString());
            document.setKeywords(query.value("keywords").toString());
            document.setChecksum(query.value("checksum").toString());
            document.setCreateTime(query.value("create_time").toDateTime());
            document.setUpdateTime(query.value("update_time").toDateTime());
            documents.append(document);
        }
    } else {
        logError("getAllDocuments", query.lastError());
    }
    
    return documents;
}

QList<Document> Database::getDocumentsByCategory(const QString& category)
{
    QList<Document> documents;
    QSqlQuery query;
    query.prepare("SELECT * FROM documents WHERE category=? ORDER BY update_time DESC");
    query.addBindValue(category);
    
    if (query.exec()) {
        while (query.next()) {
            Document document;
            document.setId(query.value("id").toInt());
            document.setTitle(query.value("title").toString());
            document.setContent(query.value("content").toString());
            document.setCategory(query.value("category").toString());
            document.setFileName(query.value("file_name").toString());
            document.setFilePath(query.value("file_path").toString());
            document.setFileType(query.value("file_type").toString());
            document.setKeywords(query.value("keywords").toString());
            document.setChecksum(query.value("checksum").toString());
            document.setCreateTime(query.value("create_time").toDateTime());
            document.setUpdateTime(query.value("update_time").toDateTime());
            documents.append(document);
        }
    } else {
        logError("getDocumentsByCategory", query.lastError());
    }
    
    return documents;
}

// 问答记录管理方法
bool Database::addQuestionRecord(const QuestionRecord& record)
{
    QSqlQuery query;
    query.prepare("INSERT INTO question_records (employee_id, question, answer, category, create_time) "
                 "VALUES (?, ?, ?, ?, ?)");
    query.addBindValue(record.getEmployeeId());
    query.addBindValue(record.getQuestion());
    query.addBindValue(record.getAnswer());
    query.addBindValue(record.getCategory());
    query.addBindValue(record.getCreateTime());
    
    return query.exec();
}

QList<QuestionRecord> Database::getQuestionRecords(const QString& employeeId)
{
    QList<QuestionRecord> records;
    QSqlQuery query;
    
    if (employeeId.isEmpty()) {
        query.prepare("SELECT * FROM question_records ORDER BY create_time DESC");
    } else {
        query.prepare("SELECT * FROM question_records WHERE employee_id=? ORDER BY create_time DESC");
        query.addBindValue(employeeId);
    }
    
    if (query.exec()) {
        while (query.next()) {
            QuestionRecord record;
            record.setId(query.value("id").toInt());
            record.setEmployeeId(query.value("employee_id").toString());
            record.setQuestion(query.value("question").toString());
            record.setAnswer(query.value("answer").toString());
            record.setCategory(query.value("category").toString());
            record.setCreateTime(query.value("create_time").toDateTime());
            records.append(record);
        }
    } else {
        logError("getQuestionRecords", query.lastError());
    }
    
    return records;
}

QList<QuestionRecord> Database::getQuestionRecordsByDateRange(const QDateTime& start, const QDateTime& end)
{
    QList<QuestionRecord> records;
    QSqlQuery query;
    query.prepare("SELECT * FROM question_records WHERE create_time BETWEEN ? AND ? ORDER BY create_time DESC");
    query.addBindValue(start);
    query.addBindValue(end);
    
    if (query.exec()) {
        while (query.next()) {
            QuestionRecord record;
            record.setId(query.value("id").toInt());
            record.setEmployeeId(query.value("employee_id").toString());
            record.setQuestion(query.value("question").toString());
            record.setAnswer(query.value("answer").toString());
            record.setCategory(query.value("category").toString());
            record.setCreateTime(query.value("create_time").toDateTime());
            records.append(record);
        }
    } else {
        logError("getQuestionRecordsByDateRange", query.lastError());
    }
    
    return records;
}

QList<QuestionRecord> Database::getQuestionRecordsByCategory(const QString& category)
{
    QList<QuestionRecord> records;
    QSqlQuery query;
    query.prepare("SELECT * FROM question_records WHERE category=? ORDER BY create_time DESC");
    query.addBindValue(category);
    
    if (query.exec()) {
        while (query.next()) {
            QuestionRecord record;
            record.setId(query.value("id").toInt());
            record.setEmployeeId(query.value("employee_id").toString());
            record.setQuestion(query.value("question").toString());
            record.setAnswer(query.value("answer").toString());
            record.setCategory(query.value("category").toString());
            record.setCreateTime(query.value("create_time").toDateTime());
            records.append(record);
        }
    } else {
        logError("getQuestionRecordsByCategory", query.lastError());
    }
    
    return records;
}

// 统计查询方法
int Database::getUserCount(UserRole role)
{
    QSqlQuery query;
    // 修复：当role为Employee时，应该查询所有用户而不是只查询员工
    // 如果需要只查询特定角色，应该使用不同的方法名
    if (role == UserRole::Employee) {
        // 默认参数情况下，返回所有用户数量
        query.prepare("SELECT COUNT(*) FROM users");
    } else {
        // 查询特定角色的用户数量
        query.prepare("SELECT COUNT(*) FROM users WHERE role=?");
        query.addBindValue(static_cast<int>(role));
    }
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    } else {
        logError("getUserCount", query.lastError());
    }
    
    return 0;
}

int Database::getDocumentCount(const QString& category)
{
    QSqlQuery query;
    if (category.isEmpty()) {
        query.prepare("SELECT COUNT(*) FROM documents");
    } else {
        query.prepare("SELECT COUNT(*) FROM documents WHERE category=?");
        query.addBindValue(category);
    }
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    } else {
        logError("getDocumentCount", query.lastError());
    }
    
    return 0;
}

int Database::getQuestionCount(const QString& employeeId)
{
    QSqlQuery query;
    if (employeeId.isEmpty()) {
        query.prepare("SELECT COUNT(*) FROM question_records");
    } else {
        query.prepare("SELECT COUNT(*) FROM question_records WHERE employee_id=?");
        query.addBindValue(employeeId);
    }
    
    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    } else {
        logError("getQuestionCount", query.lastError());
    }
    
    return 0;
}

QMap<QString, int> Database::getQuestionCategoryStats()
{
    QMap<QString, int> stats;
    QSqlQuery query("SELECT category, COUNT(*) FROM question_records GROUP BY category");
    
    if (query.exec()) {
        while (query.next()) {
            QString category = query.value("category").toString();
            int count = query.value("COUNT(*)").toInt();
            stats[category] = count;
        }
    } else {
        logError("getQuestionCategoryStats", query.lastError());
    }
    
    return stats;
}

QMap<QString, int> Database::getUserActivityStats()
{
    QMap<QString, int> stats;
    QSqlQuery query("SELECT employee_id, COUNT(*) FROM question_records GROUP BY employee_id ORDER BY COUNT(*) DESC LIMIT 10");
    
    if (query.exec()) {
        while (query.next()) {
            QString employeeId = query.value("employee_id").toString();
            int count = query.value("COUNT(*)").toInt();
            stats[employeeId] = count;
        }
    } else {
        logError("getUserActivityStats", query.lastError());
    }
    
    return stats;
}

// 私有辅助方法
bool Database::executeQuery(const QString& sql, const QVariantMap& params)
{
    QSqlQuery query;
    query.prepare(sql);
    
    for (auto it = params.begin(); it != params.end(); ++it) {
        query.addBindValue(it.value());
    }
    
    return query.exec();
}

QSqlQuery Database::prepareQuery(const QString& sql)
{
    QSqlQuery query;
    query.prepare(sql);
    return query;
}

void Database::logError(const QString& operation, const QSqlError& error)
{
    qDebug() << "数据库操作错误:" << operation << ":" << error.text();
}

// Document类实现
Document::Document()
    : id_(0)
    , title_("")
    , content_("")
    , category_("")
    , createTime_(QDateTime::currentDateTime())
    , updateTime_(QDateTime::currentDateTime())
{
}

Document::Document(const QString& title, const QString& content, const QString& category)
    : id_(0)
    , title_(title)
    , content_(content)
    , category_(category)
    , createTime_(QDateTime::currentDateTime())
    , updateTime_(QDateTime::currentDateTime())
{
}

// QuestionRecord类实现
QuestionRecord::QuestionRecord()
    : id_(0)
    , employeeId_("")
    , question_("")
    , answer_("")
    , category_("")
    , createTime_(QDateTime::currentDateTime())
{
}

QuestionRecord::QuestionRecord(const QString& employeeId, const QString& question, 
                               const QString& answer, const QString& category)
    : id_(0)
    , employeeId_(employeeId)
    , question_(question)
    , answer_(answer)
    , category_(category)
    , createTime_(QDateTime::currentDateTime())
{
}

void Database::loadInitialData()
{
    qDebug() << "=== 开始数据库初始化 ===";
    
    // 检查是否已有用户数据，如果有则不重复加载
    QSqlQuery checkQuery("SELECT COUNT(*) FROM users");
    if (checkQuery.exec() && checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        qDebug() << "数据库中已有" << checkQuery.value(0).toInt() << "个用户";
        
        // 检查文档和问答数据
        QSqlQuery docQuery("SELECT COUNT(*) FROM documents");
        QSqlQuery qaQuery("SELECT COUNT(*) FROM question_records");
        
        int docCount = 0, qaCount = 0;
        if (docQuery.exec() && docQuery.next()) {
            docCount = docQuery.value(0).toInt();
        }
        if (qaQuery.exec() && qaQuery.next()) {
            qaCount = qaQuery.value(0).toInt();
        }
        
        qDebug() << "数据库状态 - 文档:" << docCount << "个，问答:" << qaCount << "个";
        
        // 如果文档或问答数据为空，则重新添加示例数据
        if (docCount == 0 || qaCount == 0) {
            qDebug() << "检测到缺少示例数据，重新添加...";
        } else {
            qDebug() << "数据完整，跳过初始化";
            return; // 已有完整数据，不需要加载
        }
    }
    
    qDebug() << "数据库为空，开始加载初始数据...";
    
    // 插入示例用户
    QSqlQuery userQuery;
    userQuery.prepare("INSERT OR REPLACE INTO users (employee_id, name, department, role, status, create_time) VALUES (?, ?, ?, ?, ?, ?)");
    
    QStringList employeeIds = {"HR001", "HR002", "EMP001", "EMP002", "EMP003"};
    QStringList names = {"张经理", "李HR", "王员工", "赵员工", "孙员工"};
    QStringList departments = {"人事部", "人事部", "技术部", "市场部", "财务部"};
    QList<int> roles = {1, 1, 0, 0, 0}; // 1=HR管理员，0=普通员工
    
    qDebug() << "正在插入用户数据...";
    for (int i = 0; i < employeeIds.size(); ++i) {
        userQuery.addBindValue(employeeIds[i]);
        userQuery.addBindValue(names[i]);
        userQuery.addBindValue(departments[i]);
        userQuery.addBindValue(roles[i]);
        userQuery.addBindValue(static_cast<int>(UserStatus::Active)); // 明确使用枚举值
        userQuery.addBindValue(QDateTime::currentDateTime());
        
        if (!userQuery.exec()) {
            qDebug() << "❌ 插入用户失败:" << employeeIds[i] << "错误:" << userQuery.lastError().text();
            logError("插入示例用户", userQuery.lastError());
        } else {
            qDebug() << "✅ 成功插入用户:" << employeeIds[i] << "角色:" << roles[i] << "状态:" << static_cast<int>(UserStatus::Active);
        }
    }
    
    // 插入示例密码（密码都是123456的SHA256哈希）
    QSqlQuery passwordQuery;
    passwordQuery.prepare("INSERT OR REPLACE INTO passwords (employee_id, password_hash) VALUES (?, ?)");
    // 默认密码为123456，在生产环境中应该要求管理员修改
    QString passwordHash = "8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92"; // 123456的SHA256
    
    qDebug() << "正在插入密码数据...";
    for (const QString& employeeId : employeeIds) {
        passwordQuery.addBindValue(employeeId);
        passwordQuery.addBindValue(passwordHash);
        
        if (!passwordQuery.exec()) {
            qDebug() << "❌ 插入密码失败:" << employeeId << "错误:" << passwordQuery.lastError().text();
            logError("插入示例密码", passwordQuery.lastError());
        } else {
            qDebug() << "✅ 成功插入密码:" << employeeId;
        }
    }
    
    // 验证数据是否正确插入
    qDebug() << "正在验证数据...";
    QSqlQuery verifyQuery("SELECT u.employee_id, u.name, u.role, u.status, p.password_hash FROM users u LEFT JOIN passwords p ON u.employee_id = p.employee_id");
    if (verifyQuery.exec()) {
        while (verifyQuery.next()) {
            QString empId = verifyQuery.value("employee_id").toString();
            QString name = verifyQuery.value("name").toString();
            int role = verifyQuery.value("role").toInt();
            int status = verifyQuery.value("status").toInt();
            QString pwdHash = verifyQuery.value("password_hash").toString();
            qDebug() << "📋 验证数据:" << empId << name << "角色:" << role << "状态:" << status << "密码:" << (pwdHash.isEmpty() ? "❌" : "✅");
        }
    } else {
        qDebug() << "❌ 验证查询失败:" << verifyQuery.lastError().text();
    }
    
    // 添加示例文档
    qDebug() << "正在添加示例文档...";
    QList<Document> sampleDocs;
    
    sampleDocs.append(Document("员工考勤管理制度", 
        "第一章 总则\n"
        "第一条 为规范公司员工考勤管理，维护正常工作秩序，特制定本制度。\n"
        "第二条 本制度适用于公司所有员工。\n\n"
        "第二章 工作时间\n"
        "第三条 公司实行标准工时制，每周工作5天，每天工作8小时。\n"
        "第四条 工作时间：上午9:00-12:00，下午13:00-18:00。\n\n"
        "第三章 考勤管理\n"
        "第五条 员工应按时上下班打卡。\n"
        "第六条 迟到、早退、旷工等行为将影响绩效考核。", "考勤"));
    
    sampleDocs.append(Document("员工薪酬管理制度", 
        "第一章 总则\n"
        "第一条 为规范公司薪酬管理，建立公平合理的薪酬体系，特制定本制度。\n\n"
        "第二章 薪酬构成\n"
        "第二条 员工薪酬由基本工资、绩效工资、津贴补贴等组成。\n"
        "第三条 基本工资根据岗位等级和员工能力确定。\n\n"
        "第三章 社保公积金\n"
        "第四条 公司按照国家规定为员工缴纳社会保险。\n"
        "第五条 养老保险：公司16%，个人8%。\n"
        "第六条 医疗保险：公司8%，个人2%。", "薪酬"));
    
    sampleDocs.append(Document("员工绩效管理制度", 
        "第一章 总则\n"
        "第一条 为建立科学的绩效管理体系，提高员工工作积极性，特制定本制度。\n\n"
        "第二章 考核周期\n"
        "第二条 绩效考核分为月度考核、季度考核和年度考核。\n\n"
        "第三章 考核等级\n"
        "第三条 绩效等级分为：优秀(A)、良好(B)、合格(C)、不合格(D)。\n"
        "第四条 优秀比例不超过20%，良好比例不超过40%。", "绩效"));
    
    for (const Document& doc : sampleDocs) {
        if (addDocument(doc)) {
            qDebug() << "✅ 成功添加示例文档:" << doc.getTitle();
        } else {
            qDebug() << "❌ 添加示例文档失败:" << doc.getTitle();
        }
    }
    
    // 添加示例问答记录
    qDebug() << "正在添加示例问答记录...";
    QList<QuestionRecord> sampleQAs;
    
    sampleQAs.append(QuestionRecord("EMP001", "公司的工作时间是什么？", "公司实行标准工时制，每周工作5天，每天工作8小时。", "考勤"));
    sampleQAs.append(QuestionRecord("EMP002", "如何请假？", "员工请假需提前申请，经主管批准后方可休假。", "考勤"));
    sampleQAs.append(QuestionRecord("EMP003", "社保缴费比例是多少？", "养老保险：公司16%，个人8%；医疗保险：公司8%，个人2%。", "薪酬"));
    sampleQAs.append(QuestionRecord("HR001", "绩效考核周期是什么？", "绩效考核分为月度考核、季度考核和年度考核。", "绩效"));
    
    for (const QuestionRecord& qa : sampleQAs) {
        if (addQuestionRecord(qa)) {
            qDebug() << "✅ 成功添加示例问答:" << qa.getQuestion();
        } else {
            qDebug() << "❌ 添加示例问答失败:" << qa.getQuestion();
        }
    }
    
    qDebug() << "=== 初始数据加载完成 ===";
}

void Database::reloadSampleData()
{
    qDebug() << "=== 强制重新加载示例数据 ===";
    
    // 清空现有数据
    QSqlQuery query;
    query.exec("DELETE FROM documents");
    query.exec("DELETE FROM question_records");
    
    // 添加示例文档
    qDebug() << "正在添加示例文档...";
    QList<Document> sampleDocs;
    
    sampleDocs.append(Document("员工考勤管理制度", 
        "第一章 总则\n"
        "第一条 为规范公司员工考勤管理，维护正常工作秩序，特制定本制度。\n"
        "第二条 本制度适用于公司所有员工。\n\n"
        "第二章 工作时间\n"
        "第三条 公司实行标准工时制，每周工作5天，每天工作8小时。\n"
        "第四条 工作时间：上午9:00-12:00，下午13:00-18:00。\n\n"
        "第三章 考勤管理\n"
        "第五条 员工应按时上下班打卡。\n"
        "第六条 迟到、早退、旷工等行为将影响绩效考核。", "考勤"));
    
    sampleDocs.append(Document("员工薪酬管理制度", 
        "第一章 总则\n"
        "第一条 为规范公司薪酬管理，建立公平合理的薪酬体系，特制定本制度。\n\n"
        "第二章 薪酬构成\n"
        "第二条 员工薪酬由基本工资、绩效工资、津贴补贴等组成。\n"
        "第三条 基本工资根据岗位等级和员工能力确定。\n\n"
        "第三章 社保公积金\n"
        "第四条 公司按照国家规定为员工缴纳社会保险。\n"
        "第五条 养老保险：公司16%，个人8%。\n"
        "第六条 医疗保险：公司8%，个人2%。", "薪酬"));
    
    sampleDocs.append(Document("员工绩效管理制度", 
        "第一章 总则\n"
        "第一条 为建立科学的绩效管理体系，提高员工工作积极性，特制定本制度。\n\n"
        "第二章 考核周期\n"
        "第二条 绩效考核分为月度考核、季度考核和年度考核。\n\n"
        "第三章 考核等级\n"
        "第三条 绩效等级分为：优秀(A)、良好(B)、合格(C)、不合格(D)。\n"
        "第四条 优秀比例不超过20%，良好比例不超过40%。", "绩效"));
    
    for (const Document& doc : sampleDocs) {
        if (addDocument(doc)) {
            qDebug() << "✅ 成功添加示例文档:" << doc.getTitle();
        } else {
            qDebug() << "❌ 添加示例文档失败:" << doc.getTitle();
        }
    }
    
    // 添加示例问答记录
    qDebug() << "正在添加示例问答记录...";
    QList<QuestionRecord> sampleQAs;
    
    sampleQAs.append(QuestionRecord("EMP001", "公司的工作时间是什么？", "公司实行标准工时制，每周工作5天，每天工作8小时。工作时间：上午9:00-12:00，下午13:00-18:00。", "考勤"));
    sampleQAs.append(QuestionRecord("EMP002", "如何请假？", "员工请假需提前申请，经主管批准后方可休假。病假需提供医院证明。", "考勤"));
    sampleQAs.append(QuestionRecord("EMP003", "社保缴费比例是多少？", "养老保险：公司16%，个人8%；医疗保险：公司8%，个人2%。", "薪酬"));
    sampleQAs.append(QuestionRecord("EMP001", "绩效考核周期是什么？", "绩效考核分为月度考核、季度考核和年度考核。", "绩效"));
    sampleQAs.append(QuestionRecord("EMP002", "绩效等级有哪些？", "绩效等级分为：优秀(A)、良好(B)、合格(C)、不合格(D)。", "绩效"));
    
    for (const QuestionRecord& qa : sampleQAs) {
        if (addQuestionRecord(qa)) {
            qDebug() << "✅ 成功添加示例问答:" << qa.getQuestion().left(15) + "...";
        } else {
            qDebug() << "❌ 添加示例问答失败:" << qa.getQuestion().left(15) + "...";
        }
    }
    
    qDebug() << "=== 强制重新加载完成 ===";
}

