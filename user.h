#ifndef USER_H
#define USER_H

#include <QString>
#include <QDateTime>

enum class UserRole {
    Employee,    // 普通员工
    HRAdmin      // HR管理员
};

enum class UserStatus {
    Pending,     // 待审核
    Active,      // 已激活
    Inactive     // 未激活
};

class User {
public:
    User();
    User(const QString& employeeId, const QString& name, const QString& department, 
         UserRole role = UserRole::Employee, UserStatus status = UserStatus::Pending);

    // Getters
    QString getEmployeeId() const { return employeeId_; }
    QString getName() const { return name_; }
    QString getDepartment() const { return department_; }
    UserRole getRole() const { return role_; }
    UserStatus getStatus() const { return status_; }
    QDateTime getCreateTime() const { return createTime_; }
    QDateTime getLastLoginTime() const { return lastLoginTime_; }

    // Setters
    void setEmployeeId(const QString& employeeId) { employeeId_ = employeeId; }
    void setName(const QString& name) { name_ = name; }
    void setDepartment(const QString& department) { department_ = department; }
    void setRole(UserRole role) { role_ = role; }
    void setStatus(UserStatus status) { status_ = status; }
    void setLastLoginTime(const QDateTime& time) { lastLoginTime_ = time; }
    void setCreateTime(const QDateTime& time) { createTime_ = time; }

    // 验证方法
    bool isValidEmployeeId() const;
    bool isValidDepartment() const;
    bool canLogin() const;

    // 角色检查
    bool isEmployee() const { return role_ == UserRole::Employee; }
    bool isHRAdmin() const { return role_ == UserRole::HRAdmin; }

    // 状态检查
    bool isActive() const { return status_ == UserStatus::Active; }
    bool isPending() const { return status_ == UserStatus::Pending; }

private:
    QString employeeId_;      // 工号
    QString name_;           // 姓名
    QString department_;     // 部门
    UserRole role_;          // 角色
    UserStatus status_;      // 状态
    QDateTime createTime_;   // 创建时间
    QDateTime lastLoginTime_; // 最后登录时间
};

#endif // USER_H
