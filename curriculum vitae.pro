QT += core gui widgets

CONFIG += c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = curriculum_vitae

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/datastore.cpp \
    src/resumeexporter.cpp \
    src/models.cpp \
    src/radarchartwidget.cpp

HEADERS += \
    include/mainwindow.h \
    include/datastore.h \
    include/resumeexporter.h \
    include/models.h \
    include/radarchartwidget.h

FORMS += forms/mainwindow.ui

INCLUDEPATH += include

DISTFILES += README.md \
    docs/分工建议.md \
    docs/演示说明.md
