QT       += testlib qml

QT       -= gui

TARGET = tst_garnet
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += \
    test.cpp \
    testgarnetvalue.cpp \
    testgarnetbridgeclass.cpp \
    testgarnetengine.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

CONFIG += c++11

INCLUDEPATH += $${PWD}/../GarnetLib/include /usr/local/include
LIBS += -L$${PWD}/../GarnetLib/bin -lgarnet -L/usr/local/lib -lmruby

HEADERS += \
    test.h \
    testgarnetvalue.h \
    testgarnetbridgeclass.h \
    testgarnetengine.h

RESOURCES += \
    test.qrc

OTHER_FILES += \
    test.qml
