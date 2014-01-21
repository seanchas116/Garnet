QT       += testlib qml
QT       -= gui

TARGET = garnet_test
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    test.cpp \
    testgarnetvalue.cpp \
    testgarnetbridgeclass.cpp \
    testgarnetengine.cpp

HEADERS += \
    test.h \
    testgarnetvalue.h \
    testgarnetbridgeclass.h \
    testgarnetengine.h

RESOURCES += \
    test.qrc

OTHER_FILES += \
    test.qml

INCLUDEPATH += $${PWD}/../garnet/include
LIBS += $${OUT_PWD}/../garnet/libgarnet.a

include(../common.pri)
