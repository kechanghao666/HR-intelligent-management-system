#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QPushButton>
#include <QColorDialog>
#include <QFontComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QSettings>

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

private slots:
    void onApplyClicked();
    void onResetClicked();
    void onOkClicked();
    void onCancelClicked();
    void onThemeChanged();
    void onFontSizeChanged();
    void onColorSchemeChanged();

private:
    void setupUI();
    void setupAppearanceTab();
    void setupBehaviorTab();
    void setupAdvancedTab();
    void loadSettings();
    void saveSettings();
    void applySettings();
    void resetToDefaults();
    void applyTheme(int themeIndex);
    
    // UI组件
    QTabWidget* tabWidget_;
    
    // 外观设置
    QWidget* appearanceTab_;
    QComboBox* themeCombo_;
    QFontComboBox* fontCombo_;
    QSpinBox* fontSizeSpin_;
    QPushButton* primaryColorBtn_;
    QPushButton* secondaryColorBtn_;
    QSlider* opacitySlider_;
    QCheckBox* animationCheckBox_;
    
    // 行为设置
    QWidget* behaviorTab_;
    QCheckBox* autoSaveCheckBox_;
    QSpinBox* autoSaveIntervalSpin_;
    QCheckBox* soundCheckBox_;
    QCheckBox* notificationCheckBox_;
    QComboBox* languageCombo_;
    QCheckBox* startMinimizedCheckBox_;
    
    // 高级设置
    QWidget* advancedTab_;
    QLineEdit* maxHistoryEdit_;
    QCheckBox* debugModeCheckBox_;
    QLineEdit* backupPathEdit_;
    QPushButton* browseBackupBtn_;
    QCheckBox* autoBackupCheckBox_;
    QPushButton* exportSettingsBtn_;
    QPushButton* importSettingsBtn_;
    
    // 按钮
    QPushButton* okBtn_;
    QPushButton* cancelBtn_;
    QPushButton* applyBtn_;
    QPushButton* resetBtn_;
    
    // 设置存储
    QSettings* settings_;
    
    // 颜色值
    QColor primaryColor_;
    QColor secondaryColor_;
};

#endif // SETTINGSDIALOG_H
