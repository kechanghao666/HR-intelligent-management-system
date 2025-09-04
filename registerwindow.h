#ifndef REGISTERWINDOW_H
#define REGISTERWINDOW_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QFrame>
#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include "database.h"
#include "user.h"

class RegisterWindow : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterWindow(Database* database, QWidget *parent = nullptr);
    ~RegisterWindow();

signals:
    void registrationSuccessful(const User& user);
    void showLoginWindow();

private slots:
    void onRegisterClicked();
    void onBackToLoginClicked();
    void onRoleChanged(const QString& role);
    void onEmployeeIdChanged(const QString& employeeId);
    void onDepartmentChanged(const QString& department);
    void onPasswordChanged(const QString& password);
    void onConfirmPasswordChanged(const QString& confirmPassword);

private:
    Database* database_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QFrame* registerFrame_;
    QLabel* titleLabel_;
    QLabel* subtitleLabel_;
    
    // 表单组件
    QLineEdit* employeeIdEdit_;
    QLineEdit* nameEdit_;
    QComboBox* departmentCombo_;
    QComboBox* roleCombo_;
    QLineEdit* passwordEdit_;
    QLineEdit* confirmPasswordEdit_;
    
    // 按钮
    QPushButton* registerButton_;
    QPushButton* backButton_;
    
    // 状态标签
    QLabel* statusLabel_;
    QLabel* employeeIdStatusLabel_;
    QLabel* passwordStatusLabel_;
    QLabel* confirmPasswordStatusLabel_;
    
    // 验证器
    QRegularExpressionValidator* employeeIdValidator_;
    QRegularExpressionValidator* nameValidator_;
    
    // 私有方法
    void setupUI();
    void setupConnections();
    void centerWindow();
    void applyStyles();
    void setupValidators();
    void loadDepartments();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    void clearFields();
    bool validateInput();
    bool validateEmployeeId();
    bool validatePassword();
    bool validateName();
    bool validateDepartment();
    void updateStatusLabels();
    void animateRegistration();
    
    // 样式相关
    void createGradientBackground();
    void setupAnimations();
    void updateButtonStates();
};

#endif // REGISTERWINDOW_H
