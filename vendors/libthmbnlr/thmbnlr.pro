#-------------------------------------------------
#
# Project created by QtCreator 2018-09-26T01:50:06
#
#-------------------------------------------------

TARGET = thmbnlr
TEMPLATE = lib
CONFIG += staticlib
CONFIG += c++11

CONFIG(debug, debug|release)  {
    message("Building debug")
    DESTDIR = ../../libs/debug
} else {
    message("Building release")
    DESTDIR = ../../libs/release
}

HEADERS += \
    libthmbnlr_global.h \
    thumbnailcreator.h \
    videofilemetadata.h

SOURCES += \
    thumbnailcreator_stub.cpp
