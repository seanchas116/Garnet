QT       += testlib

QT       -= gui

TARGET = tst_garnet
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_garnet.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

QMAKE_CXXFLAGS += -std=c++0x -stdlib=libc++

INCLUDEPATH += $${PWD}/../GarnetLib/include /usr/local/include
QMAKE_LFLAGS_RPATH += $${PWD}/../GarnetLib/bin
LIBS += -L$${PWD}/../GarnetLib/bin -lgarnet -L/usr/local/lib -lmruby
