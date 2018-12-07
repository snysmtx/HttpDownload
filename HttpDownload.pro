QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = HttpDownload
TEMPLATE = app


SOURCES += main.cpp\
        httpdownload.cpp

HEADERS  += httpdownload.h

FORMS    += httpdownload.ui
