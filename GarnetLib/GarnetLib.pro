QT       -= gui

TARGET = garnet
TEMPLATE = lib

CONFIG += staticlib

DEFINES += GARNETLIB_LIBRARY

SOURCES += \
    src/engine.cpp \
    src/bridgeclass.cpp \
    src/bridgecall.cpp \
    src/conversion.cpp

HEADERS += src/garnetlib_global.h \
	src/engine.h \
    src/bridgeclass.h \
    src/bridgecall.h \
	src/variadicargument.h \
	src/conversion.h

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
