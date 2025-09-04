#ifndef USERDIALOG_H
#define USERDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "user.h"

class Database;

class UserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserDialog(Database* database, QWidget* parent = nullptr);
    explicit UserDialog(Database* database, const User& user, QWidget* parent = nullptr);

    User getUser() const;

private slots:
    void onAccept();

private:
    void setupUi(bool isEdit);
    void fillFromUser(const User& user);
    bool validateInput();

private:
    Database* database_;
    bool isEditMode_;
    User editingUser_;

    QLineEdit* employeeIdEdit_;
    QLineEdit* nameEdit_;
    QLineEdit* departmentEdit_;
    QComboBox* roleCombo_;
    QDialogButtonBox* buttonBox_;
};

#endif // USERDIALOG_H
