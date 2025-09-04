#ifndef DOCUMENTDIALOG_H
#define DOCUMENTDIALOG_H

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

class DocumentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DocumentDialog(Database* database, QWidget* parent = nullptr);
    explicit DocumentDialog(Database* database, const Document& document, QWidget* parent = nullptr);
    
    Document getDocument() const;
    bool isEditMode() const { return editMode_; }

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void validateInput();

private:
    void setupUI();
    void loadDocument(const Document& document);
    bool validateForm();

    Database* database_;
    bool editMode_;
    
    // 数据
    Document document_;
    
    // UI组件
    QLineEdit* titleEdit_;
    QComboBox* categoryCombo_;
    QTextEdit* contentEdit_;
    QDialogButtonBox* buttonBox_;
    QPushButton* saveButton_;
    QPushButton* cancelButton_;
};

#endif // DOCUMENTDIALOG_H
