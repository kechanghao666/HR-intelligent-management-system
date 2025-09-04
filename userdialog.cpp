#include "userdialog.h"
#include "database.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QMessageBox>

UserDialog::UserDialog(Database* database, QWidget* parent)
    : QDialog(parent)
    , database_(database)
    , isEditMode_(false)
    , employeeIdEdit_(nullptr)
    , nameEdit_(nullptr)
    , departmentEdit_(nullptr)
    , roleCombo_(nullptr)
    , buttonBox_(nullptr)
{
    setupUi(false);
    setWindowTitle("添加用户");
}

UserDialog::UserDialog(Database* database, const User& user, QWidget* parent)
    : QDialog(parent)
    , database_(database)
    , isEditMode_(true)
    , editingUser_(user)
    , employeeIdEdit_(nullptr)
    , nameEdit_(nullptr)
    , departmentEdit_(nullptr)
    , roleCombo_(nullptr)
    , buttonBox_(nullptr)
{
    setupUi(true);
    fillFromUser(user);
    setWindowTitle("编辑用户");
}

void UserDialog::setupUi(bool isEdit)
{
    setModal(true);
    resize(400, 200);

    // 创建表单控件
    employeeIdEdit_ = new QLineEdit(this);
    nameEdit_ = new QLineEdit(this);
    departmentEdit_ = new QLineEdit(this);
    roleCombo_ = new QComboBox(this);
    
    // 设置角色选项
    roleCombo_->addItem("普通员工", static_cast<int>(UserRole::Employee));
    roleCombo_->addItem("HR管理员", static_cast<int>(UserRole::HRAdmin));

    // 编辑模式下工号不可修改
    if (isEdit) {
        employeeIdEdit_->setEnabled(false);
    }

    // 创建表单布局
    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow("工号:", employeeIdEdit_);
    formLayout->addRow("姓名:", nameEdit_);
    formLayout->addRow("部门:", departmentEdit_);
    formLayout->addRow("角色:", roleCombo_);

    // 创建按钮
    buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    
    // 连接信号
    connect(buttonBox_, &QDialogButtonBox::accepted, this, &UserDialog::onAccept);
    connect(buttonBox_, &QDialogButtonBox::rejected, this, &QDialog::reject);

    // 主布局
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox_);
    
    setLayout(mainLayout);
}

void UserDialog::fillFromUser(const User& user)
{
    if (employeeIdEdit_) employeeIdEdit_->setText(user.getEmployeeId());
    if (nameEdit_) nameEdit_->setText(user.getName());
    if (departmentEdit_) departmentEdit_->setText(user.getDepartment());
    if (roleCombo_) {
        int index = user.isHRAdmin() ? 1 : 0;
        roleCombo_->setCurrentIndex(index);
    }
}

User UserDialog::getUser() const
{
    User user = editingUser_;
    
    if (employeeIdEdit_) user.setEmployeeId(employeeIdEdit_->text().trimmed());
    if (nameEdit_) user.setName(nameEdit_->text().trimmed());
    if (departmentEdit_) user.setDepartment(departmentEdit_->text().trimmed());
    if (roleCombo_) {
        UserRole role = (roleCombo_->currentIndex() == 1) ? UserRole::HRAdmin : UserRole::Employee;
        user.setRole(role);
    }
    
    return user;
}

bool UserDialog::validateInput()
{
    if (!employeeIdEdit_ || !nameEdit_ || !departmentEdit_) {
        return false;
    }

    if (employeeIdEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入工号");
        employeeIdEdit_->setFocus();
        return false;
    }

    if (nameEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入姓名");
        nameEdit_->setFocus();
        return false;
    }

    if (departmentEdit_->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入部门");
        departmentEdit_->setFocus();
        return false;
    }

    return true;
}

void UserDialog::onAccept()
{
    if (validateInput()) {
        accept();
    }
}
