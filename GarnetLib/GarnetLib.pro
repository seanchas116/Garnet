QT       -= gui

TARGET = garnet
TEMPLATE = lib

CONFIG += staticlib

DEFINES += GARNETLIB_LIBRARY

SOURCES += src/object.cpp src/variant.cpp src/utils.cpp

HEADERS += include/garnet.h include/garnetlib_global.h include/garnet/object.h include/garnet/utils.h include/garnet/variant.h

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

CONFIG += c++11

DESTDIR = $$PWD/bin
INCLUDEPATH += include /usr/local/include
