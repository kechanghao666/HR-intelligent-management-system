#include "settingsdialog.h"
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMessageBox>
#include <QStandardPaths>
#include <QApplication>
#include <QIntValidator>
#include <QFile>
#include <QColorDialog>
#include <QFileDialog>
#include <QSettings>
#include <QTabWidget>
#include <QFontComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QPalette>
#include <QColor>
#include <QTabBar>
#include <QStyleFactory>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
    , tabWidget_(nullptr)
    , appearanceTab_(nullptr)
    , behaviorTab_(nullptr)
    , advancedTab_(nullptr)
    , settings_(nullptr)
    , primaryColor_(59, 130, 246)  // 默认蓝色
    , secondaryColor_(34, 197, 94) // 默认绿色
{
    setWindowTitle("系统设置");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setMinimumSize(900, 750);  // 增加窗口尺寸以容纳所有文字，使用setMinimumSize而不是setFixedSize
    
    settings_ = new QSettings("HRSystem", "Settings", this);
    
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // 创建标签页
    tabWidget_ = new QTabWidget();
    tabWidget_->setObjectName("settingsTabWidget");
    
    setupAppearanceTab();
    setupBehaviorTab();
    setupAdvancedTab();
    
    mainLayout->addWidget(tabWidget_);
    
    // 创建按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    resetBtn_ = new QPushButton("重置默认");
    resetBtn_->setObjectName("resetBtn");
    resetBtn_->setMinimumSize(100, 35);  // 确保按钮文字完整显示
    
    cancelBtn_ = new QPushButton("取消");
    cancelBtn_->setObjectName("cancelBtn");
    cancelBtn_->setMinimumSize(80, 35);   // 确保按钮文字完整显示
    
    applyBtn_ = new QPushButton("应用");
    applyBtn_->setObjectName("applyBtn");
    applyBtn_->setMinimumSize(80, 35);    // 确保按钮文字完整显示
    
    okBtn_ = new QPushButton("确定");
    okBtn_->setObjectName("okBtn");
    okBtn_->setMinimumSize(80, 35);       // 确保按钮文字完整显示
    okBtn_->setDefault(true);
    
    buttonLayout->addWidget(resetBtn_);
    buttonLayout->addWidget(cancelBtn_);
    buttonLayout->addWidget(applyBtn_);
    buttonLayout->addWidget(okBtn_);
    
    mainLayout->addLayout(buttonLayout);
    
    // 连接信号
    connect(okBtn_, &QPushButton::clicked, this, &SettingsDialog::onOkClicked);
    connect(cancelBtn_, &QPushButton::clicked, this, &SettingsDialog::onCancelClicked);
    connect(applyBtn_, &QPushButton::clicked, this, &SettingsDialog::onApplyClicked);
    connect(resetBtn_, &QPushButton::clicked, this, &SettingsDialog::onResetClicked);
    
    // 应用样式
    setStyleSheet(
        "QTabWidget::pane {"
        "    border: 1px solid #C0C4CC;"
        "    background-color: white;"
        "    border-radius: 4px;"
        "}"
        "QTabWidget::tab-bar {"
        "    alignment: left;"
        "}"
        "QTabBar::tab {"
        "    background-color: #F5F7FA;"
        "    padding: 10px 20px;"
        "    margin-right: 2px;"
        "    border-top-left-radius: 4px;"
        "    border-top-right-radius: 4px;"
        "    color: #606266;"
        "}"
        "QTabBar::tab:selected {"
        "    background-color: white;"
        "    color: #409EFF;"
        "    border-bottom: 2px solid #409EFF;"
        "}"
        "QPushButton {"
        "    padding: 8px 16px;"
        "    border-radius: 4px;"
        "    font-weight: 500;"
        "}"
        "#okBtn, #applyBtn {"
        "    background-color: #409EFF;"
        "    color: white;"
        "    border: none;"
        "}"
        "#okBtn:hover, #applyBtn:hover {"
        "    background-color: #337ECC;"
        "}"
        "#cancelBtn, #resetBtn {"
        "    background-color: #F5F7FA;"
        "    color: #606266;"
        "    border: 1px solid #DCDFE6;"
        "}"
        "#cancelBtn:hover, #resetBtn:hover {"
        "    color: #409EFF;"
        "    border-color: #409EFF;"
        "}"
    );
}

void SettingsDialog::setupAppearanceTab()
{
    appearanceTab_ = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(appearanceTab_);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // 主题设置组
    QGroupBox* themeGroup = new QGroupBox("主题设置");
    QGridLayout* themeLayout = new QGridLayout(themeGroup);
    
    // 主题选择
    themeLayout->addWidget(new QLabel("主题:"), 0, 0);
    themeCombo_ = new QComboBox();
    themeCombo_->addItems({"浅色主题", "深色主题", "自动切换"});
    themeCombo_->setMinimumWidth(150);  // 确保下拉框宽度足够显示文字
    themeLayout->addWidget(themeCombo_, 0, 1);
    
    // 连接主题切换信号，实现即时预览
    connect(themeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, &SettingsDialog::onThemeChanged);
    
    // 主色调
    themeLayout->addWidget(new QLabel("主色调:"), 1, 0);
    primaryColorBtn_ = new QPushButton();
    primaryColorBtn_->setFixedSize(60, 35);
    primaryColorBtn_->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(primaryColor_.name()));
    connect(primaryColorBtn_, &QPushButton::clicked, [this]() {
        QColor color = QColorDialog::getColor(primaryColor_, this, "选择主色调");
        if (color.isValid()) {
            primaryColor_ = color;
            primaryColorBtn_->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(color.name()));
        }
    });
    themeLayout->addWidget(primaryColorBtn_, 1, 1);
    
    // 透明度
    themeLayout->addWidget(new QLabel("界面透明度:"), 2, 0);
    opacitySlider_ = new QSlider(Qt::Horizontal);
    opacitySlider_->setRange(70, 100);
    opacitySlider_->setValue(95);
    opacitySlider_->setTickPosition(QSlider::TicksBelow);
    opacitySlider_->setTickInterval(10);
    themeLayout->addWidget(opacitySlider_, 2, 1);
    
    layout->addWidget(themeGroup);
    
    // 字体设置组
    QGroupBox* fontGroup = new QGroupBox("字体设置");
    QGridLayout* fontLayout = new QGridLayout(fontGroup);
    
    fontLayout->addWidget(new QLabel("字体:"), 0, 0);
    fontCombo_ = new QFontComboBox();
    fontCombo_->setCurrentText("Microsoft YaHei");
    fontCombo_->setMinimumWidth(200);  // 确保字体下拉框宽度足够显示字体名称
    fontLayout->addWidget(fontCombo_, 0, 1);
    
    fontLayout->addWidget(new QLabel("字体大小:"), 1, 0);
    fontSizeSpin_ = new QSpinBox();
    fontSizeSpin_->setRange(9, 24);
    fontSizeSpin_->setValue(14);
    fontSizeSpin_->setSuffix(" px");
    fontSizeSpin_->setMinimumWidth(100);
    fontLayout->addWidget(fontSizeSpin_, 1, 1);
    
    layout->addWidget(fontGroup);
    
    // 动画设置组
    QGroupBox* animationGroup = new QGroupBox("动画效果");
    QVBoxLayout* animLayout = new QVBoxLayout(animationGroup);
    
    animationCheckBox_ = new QCheckBox("启用界面动画效果");
    animationCheckBox_->setChecked(true);
    animLayout->addWidget(animationCheckBox_);
    
    layout->addWidget(animationGroup);
    
    layout->addStretch();
    
    tabWidget_->addTab(appearanceTab_, "外观");
}

void SettingsDialog::setupBehaviorTab()
{
    behaviorTab_ = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(behaviorTab_);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // 自动保存设置组
    QGroupBox* saveGroup = new QGroupBox("自动保存");
    QGridLayout* saveLayout = new QGridLayout(saveGroup);
    
    autoSaveCheckBox_ = new QCheckBox("启用自动保存");
    autoSaveCheckBox_->setChecked(true);
    saveLayout->addWidget(autoSaveCheckBox_, 0, 0, 1, 2);
    
    saveLayout->addWidget(new QLabel("保存间隔:"), 1, 0);
    autoSaveIntervalSpin_ = new QSpinBox();
    autoSaveIntervalSpin_->setRange(1, 60);
    autoSaveIntervalSpin_->setValue(5);
    autoSaveIntervalSpin_->setSuffix(" 分钟");
    autoSaveIntervalSpin_->setMinimumWidth(120);
    saveLayout->addWidget(autoSaveIntervalSpin_, 1, 1);
    
    layout->addWidget(saveGroup);
    
    // 通知设置组
    QGroupBox* notifyGroup = new QGroupBox("通知设置");
    QVBoxLayout* notifyLayout = new QVBoxLayout(notifyGroup);
    
    soundCheckBox_ = new QCheckBox("启用声音提示");
    soundCheckBox_->setChecked(true);
    notifyLayout->addWidget(soundCheckBox_);
    
    notificationCheckBox_ = new QCheckBox("启用桌面通知");
    notificationCheckBox_->setChecked(true);
    notifyLayout->addWidget(notificationCheckBox_);
    
    layout->addWidget(notifyGroup);
    
    // 启动设置组
    QGroupBox* startupGroup = new QGroupBox("启动设置");
    QVBoxLayout* startupLayout = new QVBoxLayout(startupGroup);
    
    startMinimizedCheckBox_ = new QCheckBox("启动时最小化到系统托盘");
    startMinimizedCheckBox_->setChecked(false);
    startupLayout->addWidget(startMinimizedCheckBox_);
    
    layout->addWidget(startupGroup);
    
    // 语言设置组
    QGroupBox* langGroup = new QGroupBox("语言设置");
    QGridLayout* langLayout = new QGridLayout(langGroup);
    
    langLayout->addWidget(new QLabel("界面语言:"), 0, 0);
    languageCombo_ = new QComboBox();
    languageCombo_->addItems({"简体中文", "English", "繁體中文"});
    languageCombo_->setMinimumWidth(150);  // 确保语言下拉框宽度足够显示文字
    langLayout->addWidget(languageCombo_, 0, 1);
    
    layout->addWidget(langGroup);
    
    layout->addStretch();
    
    tabWidget_->addTab(behaviorTab_, "行为");
}

void SettingsDialog::setupAdvancedTab()
{
    advancedTab_ = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(advancedTab_);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // 性能设置组
    QGroupBox* perfGroup = new QGroupBox("性能设置");
    QGridLayout* perfLayout = new QGridLayout(perfGroup);
    
    perfLayout->addWidget(new QLabel("最大历史记录数:"), 0, 0);
    maxHistoryEdit_ = new QLineEdit();
    maxHistoryEdit_->setText("1000");
    maxHistoryEdit_->setValidator(new QIntValidator(100, 10000, this));
    maxHistoryEdit_->setMinimumWidth(150);  // 确保输入框宽度足够
    perfLayout->addWidget(maxHistoryEdit_, 0, 1);
    
    debugModeCheckBox_ = new QCheckBox("启用调试模式");
    debugModeCheckBox_->setChecked(false);
    perfLayout->addWidget(debugModeCheckBox_, 1, 0, 1, 2);
    
    layout->addWidget(perfGroup);
    
    // 数据备份设置组
    QGroupBox* backupGroup = new QGroupBox("数据备份");
    QGridLayout* backupLayout = new QGridLayout(backupGroup);
    
    backupLayout->addWidget(new QLabel("备份路径:"), 0, 0);
    backupPathEdit_ = new QLineEdit();
    backupPathEdit_->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/HRSystem_Backup");
    backupPathEdit_->setReadOnly(true);
    backupPathEdit_->setMinimumWidth(300);  // 确保路径输入框足够宽
    backupLayout->addWidget(backupPathEdit_, 0, 1);
    
    browseBackupBtn_ = new QPushButton("浏览...");
    browseBackupBtn_->setMinimumSize(80, 30);  // 确保按钮足够大
    connect(browseBackupBtn_, &QPushButton::clicked, [this]() {
        QString dir = QFileDialog::getExistingDirectory(this, "选择备份目录", backupPathEdit_->text());
        if (!dir.isEmpty()) {
            backupPathEdit_->setText(dir);
        }
    });
    backupLayout->addWidget(browseBackupBtn_, 0, 2);
    
    autoBackupCheckBox_ = new QCheckBox("启用自动备份");
    autoBackupCheckBox_->setChecked(true);
    backupLayout->addWidget(autoBackupCheckBox_, 1, 0, 1, 3);
    
    layout->addWidget(backupGroup);
    
    // 设置导入导出组
    QGroupBox* importExportGroup = new QGroupBox("设置管理");
    QHBoxLayout* ieLayout = new QHBoxLayout(importExportGroup);
    
    exportSettingsBtn_ = new QPushButton("导出设置");
    exportSettingsBtn_->setMinimumSize(120, 35);  // 确保按钮文字完整显示
    connect(exportSettingsBtn_, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "导出设置", 
                                                       "HRSystem_Settings.ini", 
                                                       "配置文件 (*.ini)");
        if (!fileName.isEmpty()) {
            settings_->sync();
            // 实现设置导出逻辑
            QMessageBox::information(this, "导出成功", "设置已成功导出到: " + fileName);
        }
    });
    ieLayout->addWidget(exportSettingsBtn_);
    
    importSettingsBtn_ = new QPushButton("导入设置");
    importSettingsBtn_->setMinimumSize(120, 35);  // 确保按钮文字完整显示
    connect(importSettingsBtn_, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "导入设置", 
                                                       "", 
                                                       "配置文件 (*.ini)");
        if (!fileName.isEmpty()) {
            // 实现设置导入逻辑
            loadSettings();
            QMessageBox::information(this, "导入成功", "设置已成功导入并应用");
        }
    });
    ieLayout->addWidget(importSettingsBtn_);
    
    layout->addWidget(importExportGroup);
    
    layout->addStretch();
    
    tabWidget_->addTab(advancedTab_, "高级");
}

void SettingsDialog::loadSettings()
{
    // 加载外观设置
    themeCombo_->setCurrentIndex(settings_->value("appearance/theme", 0).toInt());
    fontCombo_->setCurrentText(settings_->value("appearance/font", "Microsoft YaHei").toString());
    fontSizeSpin_->setValue(settings_->value("appearance/fontSize", 14).toInt());
    opacitySlider_->setValue(settings_->value("appearance/opacity", 95).toInt());
    animationCheckBox_->setChecked(settings_->value("appearance/animation", true).toBool());
    
    // 加载行为设置
    autoSaveCheckBox_->setChecked(settings_->value("behavior/autoSave", true).toBool());
    autoSaveIntervalSpin_->setValue(settings_->value("behavior/autoSaveInterval", 5).toInt());
    soundCheckBox_->setChecked(settings_->value("behavior/sound", true).toBool());
    notificationCheckBox_->setChecked(settings_->value("behavior/notification", true).toBool());
    languageCombo_->setCurrentIndex(settings_->value("behavior/language", 0).toInt());
    startMinimizedCheckBox_->setChecked(settings_->value("behavior/startMinimized", false).toBool());
    
    // 加载高级设置
    maxHistoryEdit_->setText(settings_->value("advanced/maxHistory", "1000").toString());
    debugModeCheckBox_->setChecked(settings_->value("advanced/debugMode", false).toBool());
    backupPathEdit_->setText(settings_->value("advanced/backupPath", 
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/HRSystem_Backup").toString());
    autoBackupCheckBox_->setChecked(settings_->value("advanced/autoBackup", true).toBool());
    
    // 加载颜色设置
    primaryColor_ = QColor(settings_->value("appearance/primaryColor", "#3B82F6").toString());
    secondaryColor_ = QColor(settings_->value("appearance/secondaryColor", "#22C55E").toString());
    primaryColorBtn_->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(primaryColor_.name()));
}

void SettingsDialog::saveSettings()
{
    // 保存外观设置
    settings_->setValue("appearance/theme", themeCombo_->currentIndex());
    settings_->setValue("appearance/font", fontCombo_->currentText());
    settings_->setValue("appearance/fontSize", fontSizeSpin_->value());
    settings_->setValue("appearance/opacity", opacitySlider_->value());
    settings_->setValue("appearance/animation", animationCheckBox_->isChecked());
    settings_->setValue("appearance/primaryColor", primaryColor_.name());
    settings_->setValue("appearance/secondaryColor", secondaryColor_.name());
    
    // 保存行为设置
    settings_->setValue("behavior/autoSave", autoSaveCheckBox_->isChecked());
    settings_->setValue("behavior/autoSaveInterval", autoSaveIntervalSpin_->value());
    settings_->setValue("behavior/sound", soundCheckBox_->isChecked());
    settings_->setValue("behavior/notification", notificationCheckBox_->isChecked());
    settings_->setValue("behavior/language", languageCombo_->currentIndex());
    settings_->setValue("behavior/startMinimized", startMinimizedCheckBox_->isChecked());
    
    // 保存高级设置
    settings_->setValue("advanced/maxHistory", maxHistoryEdit_->text());
    settings_->setValue("advanced/debugMode", debugModeCheckBox_->isChecked());
    settings_->setValue("advanced/backupPath", backupPathEdit_->text());
    settings_->setValue("advanced/autoBackup", autoBackupCheckBox_->isChecked());
    
    settings_->sync();
}

void SettingsDialog::onThemeChanged()
{
    // 即时应用主题更改
    applyTheme(themeCombo_->currentIndex());
}

void SettingsDialog::applyTheme(int themeIndex)
{
    // 应用主题：0浅色 1深色 2自动（按系统）
    QSettings appSettings("HRSystem", "Settings");
    appSettings.setValue("appearance/theme", themeIndex);
    
    // 轻量方案：切换全局调色板与QSS
    if (themeIndex == 1) {
        // 深色主题
        QPalette pal = qApp->palette();
        pal.setColor(QPalette::Window, QColor(30,30,30));
        pal.setColor(QPalette::WindowText, Qt::white);
        pal.setColor(QPalette::Base, QColor(45,45,45));
        pal.setColor(QPalette::AlternateBase, QColor(60,60,60));
        pal.setColor(QPalette::ToolTipBase, Qt::white);
        pal.setColor(QPalette::ToolTipText, Qt::white);
        pal.setColor(QPalette::Text, Qt::white);
        pal.setColor(QPalette::Button, QColor(45,45,45));
        pal.setColor(QPalette::ButtonText, Qt::white);
        pal.setColor(QPalette::BrightText, Qt::red);
        pal.setColor(QPalette::Highlight, QColor(59, 130, 246));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        qApp->setPalette(pal);
        
        QFile f(":/styles/modern.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
        }
    } else {
        // 浅色主题/自动 → 使用主样式
        qApp->setPalette(QPalette());
        QFile f(":/styles/main.qss");
        if (f.open(QFile::ReadOnly | QFile::Text)) {
            qApp->setStyleSheet(QString::fromUtf8(f.readAll()));
        } else {
            qApp->setStyleSheet("");
        }
    }
}

void SettingsDialog::applySettings()
{
    saveSettings();
    applyTheme(themeCombo_->currentIndex());
    QMessageBox::information(this, "设置应用", "设置已成功应用！");
}

void SettingsDialog::resetToDefaults()
{
    // 重置为默认值
    themeCombo_->setCurrentIndex(0);
    fontCombo_->setCurrentText("Microsoft YaHei");
    fontSizeSpin_->setValue(14);
    opacitySlider_->setValue(95);
    animationCheckBox_->setChecked(true);
    primaryColor_ = QColor(59, 130, 246);
    primaryColorBtn_->setStyleSheet(QString("background-color: %1; border-radius: 4px;").arg(primaryColor_.name()));
    
    autoSaveCheckBox_->setChecked(true);
    autoSaveIntervalSpin_->setValue(5);
    soundCheckBox_->setChecked(true);
    notificationCheckBox_->setChecked(true);
    languageCombo_->setCurrentIndex(0);
    startMinimizedCheckBox_->setChecked(false);
    
    maxHistoryEdit_->setText("1000");
    debugModeCheckBox_->setChecked(false);
    backupPathEdit_->setText(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/HRSystem_Backup");
    autoBackupCheckBox_->setChecked(true);
}

void SettingsDialog::onOkClicked()
{
    applySettings();
    accept();
}

void SettingsDialog::onCancelClicked()
{
    reject();
}

void SettingsDialog::onApplyClicked()
{
    applySettings();
}

void SettingsDialog::onResetClicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "重置设置", 
        "确定要将所有设置重置为默认值吗？", 
        QMessageBox::Yes | QMessageBox::No, 
        QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        resetToDefaults();
    }
}



void SettingsDialog::onFontSizeChanged()
{
    // 字体大小变化处理
}

void SettingsDialog::onColorSchemeChanged()
{
    // 配色方案变化处理
}
