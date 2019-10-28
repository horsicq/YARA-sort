QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = yara-sort
TEMPLATE = app

SOURCES += \
        guimainwindow.cpp \
        main_gui.cpp \
        dialogscanprogress.cpp \
        scanprogress.cpp

HEADERS += \
        guimainwindow.h \
        ../global.h \
        dialogscanprogress.h \
        scanprogress.h

FORMS += \
        guimainwindow.ui \
        dialogscanprogress.ui

!contains(XCONFIG, qyara) {
        XCONFIG += qyara
        DEFINES += X_YARA_LEGACY_STDIO_DEFS
        include(../QYara/qyara.pri)
}

!contains(XCONFIG, xbinary) {
        XCONFIG += xbinary
        include(../Formats/xbinary.pri)
}

include(../build.pri)

win32 {
    RC_ICONS = ../icons/main.ico
}

macx {
    ICON = ../icons/main.icns
}
