QT       -= gui

TARGET = garnet
TEMPLATE = lib
CONFIG += static

DEFINES += GARNETLIB_LIBRARY

SOURCES += \
    src/engine.cpp \
    src/bridgeclass.cpp \
    src/bridgecall.cpp \
    src/conversion.cpp \
    src/utils.cpp

HEADERS += src/garnetlib_global.h \
	src/engine.h \
    src/bridgeclass.h \
    src/bridgecall.h \
	src/variadicargument.h \
	src/conversion.h \
    src/utils.h

include(../common.pri)
