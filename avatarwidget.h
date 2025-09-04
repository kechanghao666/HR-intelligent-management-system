#ifndef AVATARWIDGET_H
#define AVATARWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QPropertyAnimation>
#include <QGraphicsDropShadowEffect>
#include "user.h"

class AvatarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AvatarWidget(const User& user, QWidget *parent = nullptr);
    ~AvatarWidget();

    void setAvatarPixmap(const QPixmap& pixmap);
    QPixmap getAvatarPixmap() const;
    void setSize(int size);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

signals:
    void avatarClicked();
    void avatarChanged(const QPixmap& newAvatar);

private slots:
    void onHoverAnimation();

private:
    User user_;
    QPixmap currentAvatar_;
    int avatarSize_;
    bool isHovered_;
    
    QPropertyAnimation* hoverAnimation_;
    QGraphicsDropShadowEffect* shadowEffect_;
    
    // 私有方法
    void setupAnimations();
    void loadUserAvatar();
    QPixmap createDefaultAvatar();
    QPixmap makeCircular(const QPixmap& pixmap);
};

// 头像选择对话框
class AvatarSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AvatarSelectionDialog(const User& user, QWidget *parent = nullptr);
    ~AvatarSelectionDialog();

    QPixmap getSelectedAvatar() const;

signals:
    void avatarSelected(const QPixmap& avatar);

private slots:
    void onDefaultAvatarClicked();
    void onUploadAvatarClicked();
    void onAvatarButtonClicked();

private:
    User user_;
    QPixmap selectedAvatar_;
    
    // UI组件
    QVBoxLayout* mainLayout_;
    QLabel* titleLabel_;
    QLabel* currentAvatarLabel_;
    
    // 默认头像区域
    QGroupBox* defaultAvatarsGroup_;
    QGridLayout* defaultAvatarsLayout_;
    QList<QPushButton*> defaultAvatarButtons_;
    
    // 操作按钮
    QHBoxLayout* buttonLayout_;
    QPushButton* uploadButton_;
    QPushButton* confirmButton_;
    QPushButton* cancelButton_;
    
    // 私有方法
    void setupUI();
    void setupConnections();
    void loadDefaultAvatars();
    void applyStyles();
    QPushButton* createAvatarButton(const QPixmap& pixmap, int index);
    QPixmap loadDefaultAvatar(int index);
    void selectAvatar(const QPixmap& avatar);
};

// 头像管理器
class AvatarManager
{
public:
    static AvatarManager& instance();
    
    // 头像操作
    void saveUserAvatar(const QString& employeeId, const QPixmap& avatar);
    QPixmap loadUserAvatar(const QString& employeeId);
    bool hasUserAvatar(const QString& employeeId) const;
    void removeUserAvatar(const QString& employeeId);
    
    // 默认头像
    QList<QPixmap> getDefaultAvatars();
    QPixmap getDefaultAvatar(int index);
    QPixmap createPersonalizedAvatar(const QString& name);
    QPixmap createAssistantAvatar();

private:
    AvatarManager();
    ~AvatarManager();
    
    QString getAvatarPath(const QString& employeeId) const;
    QString getDefaultAvatarPath(int index) const;
    void ensureAvatarDirectory();
    void createDefaultAvatars();
    QPixmap generateGradientAvatar(const QString& text, const QColor& color1, const QColor& color2);
    
    QList<QPixmap> defaultAvatars_;
    QString avatarDirectory_;
};

#endif // AVATARWIDGET_H
