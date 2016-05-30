#-------------------------------------------------
#
# Project created by QtCreator 2014-11-24T01:51:23
#
#-------------------------------------------------

QT += core gui widgets network positioning

TARGET = HornAppDesktop
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp \
    feedwidget.cpp \
    requestmanager.cpp \
    feedlistmodel.cpp \
    feeditemdelegate.cpp \
    commentswidget.cpp \
    feeditemwidget.cpp \
    feedimagecache.cpp \
    mainwindow.cpp \
    notificationsdialog.cpp

HEADERS += feedwidget.h \
    requestmanager.h \
    feeditem.h \
    feedlistmodel.h \
    feeditemdelegate.h \
    commentswidget.h \
    feeditemwidget.h \
    feedlistview.hpp \
    feedimagecache.h \
    mainwindow.h \
    notificationsdialog.h

FORMS += feedwidget.ui \
    commentswidget.ui \
    feeditemwidget.ui \
    mainwindow.ui \
    notificationsdialog.ui

macx {
    QT += macextras
    OBJECTIVE_SOURCES += notificationsdialog_mac.mm \
        feedwidget_mac.mm
    QMAKE_CXXFLAGS += -fobjc-arc
    LIBS += -framework Foundation \
        -framework AppKit
}
