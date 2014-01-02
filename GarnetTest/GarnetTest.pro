QT       += testlib

QT       -= gui

TARGET = tst_garnet
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_garnet.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

CONFIG += c++11

INCLUDEPATH += $${PWD}/../GarnetLib/include /usr/local/include
LIBS += -L$${PWD}/../GarnetLib/bin -lgarnet -L/usr/local/lib -lmruby
