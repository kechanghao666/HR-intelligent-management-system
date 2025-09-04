#ifndef QADIALOG_H
#define QADIALOG_H

#include <QObject>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "database.h"

class QADialog : public QDialog
{
    Q_OBJECT

public:
    explicit QADialog(Database* database, QWidget* parent = nullptr);
    explicit QADialog(Database* database, const QuestionRecord& qa, QWidget* parent = nullptr);
    
    QuestionRecord getQA() const;
    bool isEditMode() const { return editMode_; }

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void validateInput();

private:
    void setupUI();
    void loadQA(const QuestionRecord& qa);
    bool validateForm();

    Database* database_;
    bool editMode_;
    
    // UI组件
    QLineEdit* employeeIdEdit_;
    QTextEdit* questionEdit_;
    QTextEdit* answerEdit_;
    QComboBox* categoryCombo_;
    QDialogButtonBox* buttonBox_;
    QPushButton* saveButton_;
    QPushButton* cancelButton_;
    
    // 数据
    QuestionRecord qa_;
};

#endif // QADIALOG_H
