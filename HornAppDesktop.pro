#-------------------------------------------------
#
# Project created by QtCreator 2014-11-24T01:51:23
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HornAppDesktop
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    requestmanager.cpp \
    feedlistmodel.cpp \
    feeditemdelegate.cpp \
    commentswidget.cpp \
    feeditemwidget.cpp

HEADERS  += widget.h \
    requestmanager.h \
    feeditem.h \
    feedlistmodel.h \
    feeditemdelegate.h \
    commentswidget.h \
    feeditemwidget.h

FORMS    += widget.ui \
    commentswidget.ui \
    feeditemwidget.ui

macx {
    *-clang*: cache()
    QMAKE_MAC_SDK = macosx10.10
    CONFIG += c++11
}
