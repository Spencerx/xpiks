#-------------------------------------------------
#
# Project created by QtCreator 2015-11-15T16:40:11
#
#-------------------------------------------------

QT       -= core gui

TARGET = hunspell
TEMPLATE = lib
CONFIG += staticlib
DEFINES += HUNSPELL_STATIC

CONFIG += c++11

INCLUDEPATH += "../hunspell-repo/src/hunspell/"
INCLUDEPATH += "../hunspell-repo/msvc/"

SOURCES += \
    ../hunspell-repo/src/hunspell/affentry.cxx \
    ../hunspell-repo/src/hunspell/affixmgr.cxx \
    ../hunspell-repo/src/hunspell/csutil.cxx \
    ../hunspell-repo/src/hunspell/filemgr.cxx \
    ../hunspell-repo/src/hunspell/hashmgr.cxx \
    ../hunspell-repo/src/hunspell/hunspell.cxx \
    ../hunspell-repo/src/hunspell/hunzip.cxx \
    ../hunspell-repo/src/hunspell/phonet.cxx \
    ../hunspell-repo/src/hunspell/replist.cxx \
    ../hunspell-repo/src/hunspell/suggestmgr.cxx \
    ../hunspell-repo/src/hunspell/utf_info.cxx

HEADERS += \
    ../hunspell-repo/src/hunspell/affentry.hxx \
    ../hunspell-repo/src/hunspell/affixmgr.hxx \
    ../hunspell-repo/src/hunspell/atypes.hxx \
    ../hunspell-repo/src/hunspell/baseaffix.hxx \
    ../hunspell-repo/src/hunspell/csutil.hxx \
    ../hunspell-repo/src/hunspell/dictmgr.hxx \
    ../hunspell-repo/src/hunspell/filemgr.hxx \
    ../hunspell-repo/src/hunspell/hashmgr.hxx \
    ../hunspell-repo/src/hunspell/htypes.hxx \
    ../hunspell-repo/src/hunspell/hunspell.h \
    ../hunspell-repo/src/hunspell/hunspell.hxx \
    ../hunspell-repo/src/hunspell/hunvisapi.h \
    ../hunspell-repo/src/hunspell/hunzip.hxx \
    ../hunspell-repo/src/hunspell/langnum.hxx \
    ../hunspell-repo/src/hunspell/phonet.hxx \
    ../hunspell-repo/src/hunspell/replist.hxx \
    ../hunspell-repo/src/hunspell/suggestmgr.hxx \
    ../hunspell-repo/src/hunspell/w_char.hxx \
    ../hunspell-repo/msvc/config.h \
    ../hunspell-repo/src/hunspell/hunvisapi.h.in

unix {
    target.path = /usr/lib
    INSTALLS += target
}

DISTFILES += \
    ../hunspell-repo/src/hunspell/Makefile.am
