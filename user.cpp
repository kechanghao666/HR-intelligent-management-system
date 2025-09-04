#include "user.h"
#include <QRegularExpression>
#include <QRegularExpressionValidator>

User::User()
    : employeeId_("")
    , name_("")
    , department_("")
    , role_(UserRole::Employee)
    , status_(UserStatus::Pending)
    , createTime_(QDateTime::currentDateTime())
    , lastLoginTime_(QDateTime::currentDateTime())
{
}

User::User(const QString& employeeId, const QString& name, const QString& department, 
           UserRole role, UserStatus status)
    : employeeId_(employeeId)
    , name_(name)
    , department_(department)
    , role_(role)
    , status_(status)
    , createTime_(QDateTime::currentDateTime())
    , lastLoginTime_(QDateTime::currentDateTime())
{
}

bool User::isValidEmployeeId() const
{
    // 工号格式：字母+数字，长度5-10位（修改为5位起）
    QRegularExpression regex("^[A-Za-z][A-Za-z0-9]{4,9}$");
    return regex.match(employeeId_).hasMatch();
}

bool User::isValidDepartment() const
{
    // 部门名称不能为空且长度合理
    return !department_.isEmpty() && department_.length() <= 50;
}

bool User::canLogin() const
{
    return isValidEmployeeId() && isValidDepartment() && isActive();
}
