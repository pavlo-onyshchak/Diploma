#-------------------------------------------------
#
# Project created by QtCreator 2018-01-27T22:46:54
#
#-------------------------------------------------

QT       += core gui
QT       += core
QT       += networkauth
QT       += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = InstagramBACKUP
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -L/usr/lib/x86_64-linux-gnu -lboost_system
LIBS += -L/usr/lib/x86_64-linux-gnu -lcpprest
LIBS += -L/usr/lib/x86_64-linux-gnu -lssl
LIBS += -L/usr/lib/x86_64-linux-gnu -lcrypto

QMAKE_CXXFLAGS += -std=c++17

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    downloadmanager.cpp \
    databaseconnection.cpp \
    requestexecutorandtokenhandler.cpp \
    logindialog.cpp

HEADERS += \
        mainwindow.h \
    json.hpp \
    downloadmanager.h \
    oauthcodelistener.h \
    oauthgettingcodesession.h \
    authorizationinfo.h \
    databaseconnection.h \
    databasedata.h \
    requestexecutorandtokenhandler.h \
    logindialog.h

FORMS += \
        mainwindow.ui \
    logindialog.ui

RESOURCES += \
    icons.qrc
