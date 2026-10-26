QT += core gui widgets network sql printsupport xml

CONFIG += c++17

TARGET = HRSystem
TEMPLATE = app

# 项目版本信息
VERSION = 1.0.0
QMAKE_TARGET_COMPANY = "Company HR System"
QMAKE_TARGET_PRODUCT = "HR智能问答系统"
QMAKE_TARGET_DESCRIPTION = "公司HR制度智能问答系统"
QMAKE_TARGET_COPYRIGHT = "Copyright 2024"

# 源文件
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    loginwindow.cpp \
    registerwindow.cpp \
    qawidget.cpp \
    modernqawidget.cpp \
    aiassistantwidget.cpp \
    chatbubble.cpp \
    avatarwidget.cpp \
    adminpanel.cpp \
    database.cpp \
    user.cpp \
    qaengine.cpp \
    chartwidget.cpp \
    documentviewer.cpp \
    documentdialog.cpp \
    userdialog.cpp \
    qadialog.cpp \
    reportgenerator.cpp \
    batchuploaddialog.cpp \
    settingsdialog.cpp \
    attachmentdialog.cpp \
    documentprocessor.cpp \
    relatedclausesdialog.cpp \
    xfyun_api.cpp \
    formtemplate.cpp \
    nlpprocessor.cpp \
    formexporter.cpp \
    weatherwidget.cpp

# 头文件
HEADERS += \
    mainwindow.h \
    loginwindow.h \
    registerwindow.h \
    qawidget.h \
    modernqawidget.h \
    aiassistantwidget.h \
    chatbubble.h \
    avatarwidget.h \
    adminpanel.h \
    database.h \
    user.h \
    qaengine.h \
    chartwidget.h \
    documentviewer.h \
    documentdialog.h \
    userdialog.h \
    qadialog.h \
    reportgenerator.h \
    batchuploaddialog.h \
    settingsdialog.h \
    attachmentdialog.h \
    documentprocessor.h \
    relatedclausesdialog.h \
    xfyun_api.h \
    formtemplate.h \
    nlpprocessor.h \
    formexporter.h \
    weatherwidget.h

# 资源文件
RESOURCES += \
    resources.qrc

# 默认规则使 moc 处理所有头文件
CONFIG += qt

# 编译选项
DEFINES += QT_DEPRECATED_WARNINGS

# 链接器选项 - 使用Qt自动链接
win32 {
    # 让Qt自动处理库链接
}

# 输出目录
DESTDIR = $$PWD/bin

# 包含路径
INCLUDEPATH += $$PWD/include

# 库路径
LIBS += -L$$PWD/lib

# 调试信息（与Shadow Build兼容）
CONFIG(debug, debug|release) {
    DESTDIR = $$OUT_PWD/bin/debug
    OBJECTS_DIR = $$OUT_PWD/build/debug
    MOC_DIR = $$OUT_PWD/build/debug
    RCC_DIR = $$OUT_PWD/build/debug
    UI_DIR = $$OUT_PWD/build/debug
}

# 发布版本（与Shadow Build兼容）
CONFIG(release, debug|release) {
    DESTDIR = $$OUT_PWD/bin/release
    OBJECTS_DIR = $$OUT_PWD/build/release
    MOC_DIR = $$OUT_PWD/build/release
    RCC_DIR = $$OUT_PWD/build/release
    UI_DIR = $$OUT_PWD/build/release
}
