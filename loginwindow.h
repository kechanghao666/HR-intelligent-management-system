#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QFrame>
#include <QApplication>
#include <QScreen>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QRadioButton>
#include <QButtonGroup>
#include "database.h"
#include "user.h"

class LoginWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoginWindow(Database* database, QWidget *parent = nullptr);
    ~LoginWindow();
    void clearFields();

signals:
    void loginSuccessful(const User& user);
    void showRegisterWindow();

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void onForgotPasswordClicked();
    void onRememberPasswordChanged(bool checked);
    void onAutoLoginChanged(bool checked);

private:
    Database* database_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QFrame* loginFrame_;
    QLabel* titleLabel_;
    QLineEdit* employeeIdEdit_;
    QLineEdit* passwordEdit_;
    QPushButton* loginButton_;
    QPushButton* registerButton_;
    QPushButton* forgotPasswordButton_;
    QCheckBox* rememberPasswordCheck_;
    QCheckBox* autoLoginCheck_;
    QLabel* statusLabel_;
    QButtonGroup* roleGroup_;

    // 私有方法
    void setupUI();
    void setupConnections();
    void centerWindow();
    void applyStyles();
    void showError(const QString& message);
    void showSuccess(const QString& message);
    bool validateInput();
    void saveLoginSettings();
    void loadLoginSettings();
    void animateLogin();
    
    // 样式相关
    void createGradientBackground();
    void setupAnimations();
};

#endif // LOGINWINDOW_H
