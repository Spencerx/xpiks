TEMPLATE = app
TARGET = xpiks-tests-integration
DEFINES += APPNAME=xpiks-tests-integration

QMAKE_MAC_SDK = macosx10.11

QT += qml quick widgets concurrent svg testlib
QT -= gui

CONFIG   += console
CONFIG   -= app_bundle

CONFIG += c++11

BUILDNO = $$system(git log -n 1 --pretty=format:"%H")

DEFINES += QT_NO_CAST_TO_ASCII \
           QT_RESTRICTED_CAST_FROM_ASCII \
           QT_NO_CAST_FROM_BYTEARRAY

DEFINES += HUNSPELL_STATIC
DEFINES += TELEMETRY_ENABLED
DEFINES += WITH_STDOUT_LOGS
DEFINES += WITH_LOGS
DEFINES += INTEGRATION_TESTS
DEFINES += WITH_PLUGINS
DEFINES += WITH_UPDATES

SOURCES += main.cpp \
    ../../../vendors/tiny-aes/aes.cpp \
    ../../../vendors/sqlite/sqlite3.c \
    addfilesbasictest.cpp \
    autoattachvectorstest.cpp \
    savefilebasictest.cpp \
    spellcheckmultireplacetest.cpp \
    spellcheckcombinedmodeltest.cpp \
    zipartworkstest.cpp \
    spellcheckundotest.cpp \
    autocompletebasictest.cpp \
    spellingproduceswarningstest.cpp \
    undoaddwithvectorstest.cpp \
    readlegacysavedtest.cpp \
    clearmetadatatest.cpp \
    savewithemptytitletest.cpp \
    combinededitfixspellingtest.cpp \
    findandreplacemodeltest.cpp \
    addtouserdictionarytest.cpp \
    autodetachvectortest.cpp \
    removefromuserdictionarytest.cpp \
    testshelpers.cpp \
    artworkuploaderbasictest.cpp \
    plaintextedittest.cpp \
    fixspellingmarksmodifiedtest.cpp \
    presetstest.cpp \
    translatorbasictest.cpp \
    userdictedittest.cpp \
    weirdnamesreadtest.cpp \
    restoresessiontest.cpp \
    savefilelegacytest.cpp \
    locallibrarysearchtest.cpp \
    metadatacachesavetest.cpp \
    savevideobasictest.cpp \
    autocompletepresetstest.cpp \
    duplicatesearchtest.cpp \
    csvexporttest.cpp \
    exiv2iohelpers.cpp \
    unicodeiotest.cpp \
    faileduploadstest.cpp \
    undoadddirectorytest.cpp \
    undorestoresessiontest.cpp \
    masterpasswordtest.cpp \
    reimporttest.cpp \
    autoimporttest.cpp \
    importlostmetadatatest.cpp \
    warningscombinedtest.cpp \
    csvdefaultexporttest.cpp \
    loadpluginbasictest.cpp \
    integrationtestsenvironment.cpp \
    integrationtestbase.cpp \
    stockftpautocompletetest.cpp \
    xpikstestsapp.cpp

RESOURCES +=

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    ../../../vendors/tiny-aes/aes.h \
    ../../../vendors/sqlite/sqlite3.h \
    integrationtestbase.h \
    addfilesbasictest.h \
    signalwaiter.h \
    autoattachvectorstest.h \
    savefilebasictest.h \
    spellcheckmultireplacetest.h \
    spellcheckcombinedmodeltest.h \
    zipartworkstest.h \
    spellcheckundotest.h \
    autocompletebasictest.h \
    undoaddwithvectorstest.h \
    readlegacysavedtest.h \
    clearmetadatatest.h \
    savewithemptytitletest.h \
    spellingproduceswarningstest.h \
    combinededitfixspellingtest.h \
    findandreplacemodeltest.h \
    addtouserdictionarytest.h \
    autodetachvectortest.h \
    removefromuserdictionarytest.h \
    testshelpers.h \
    artworkuploaderbasictest.h \
    plaintextedittest.h \
    fixspellingmarksmodifiedtest.h \
    presetstest.h \
    translatorbasictest.h \
    userdictedittest.h \
    weirdnamesreadtest.h \
    restoresessiontest.h \
    savefilelegacytest.h \
    locallibrarysearchtest.h \
    metadatacachesavetest.h \
    savevideobasictest.h \
    autocompletepresetstest.h \
    duplicatesearchtest.h \
    csvexporttest.h \
    ../../../vendors/csv/csv.h \
    exiv2iohelpers.h \
    unicodeiotest.h \
    faileduploadstest.h \
    undoadddirectorytest.h \
    undorestoresessiontest.h \
    masterpasswordtest.h \
    reimporttest.h \
    autoimporttest.h \
    importlostmetadatatest.h \
    warningscombinedtest.h \
    csvdefaultexporttest.h \
    loadpluginbasictest.h \
    integrationtestsenvironment.h \
    stockftpautocompletetest.h \
    xpikstestsapp.h

INCLUDEPATH += ../../../vendors/tiny-aes
INCLUDEPATH += ../../../vendors/cpp-libface
INCLUDEPATH += ../../../vendors/ssdll/src/ssdll
INCLUDEPATH += ../../../vendors/hunspell-repo/src
INCLUDEPATH += ../../../vendors/libthmbnlr
INCLUDEPATH += ../../../vendors/libxpks
INCLUDEPATH += ../../../vendors/chillout/src/chillout
INCLUDEPATH += ../../xpiks-qt

CONFIG(debug, debug|release)  {
    LIBS += -L"$$PWD/../../../libs/debug"
} else {
    LIBS += -L"$$PWD/../../../libs/release"
}
LIBS += -lhunspell
LIBS += -lz
LIBS += -lcurl
LIBS += -lquazip
LIBS += -lface
LIBS += -lssdll
LIBS += -lthmbnlr
LIBS += -lxpks
LIBS += -lchillout

include(../../xpiks-common/xpiks-common.pri)

BUILDNO=$$system(git log -n 1 --pretty=format:"%h")
BRANCH_NAME=$$system(git rev-parse --abbrev-ref HEAD)

macx {
    INCLUDEPATH += "../../../vendors/quazip"
    INCLUDEPATH += "../../../vendors/libcurl/include"
    INCLUDEPATH += "../../../vendors/exiv2-0.25/include"

    LIBS += -liconv
    LIBS += -lexpat

    LIBS += -lxmpsdk
    LIBS += -lexiv2

    LIBS += -lavcodec.57
    LIBS += -lavfilter.6
    LIBS += -lavformat.57
    LIBS += -lavutil.55
    LIBS += -lswscale.4
}

win32 {
    DEFINES += QT_NO_PROCESS_COMBINED_ARGUMENT_START
    QT += winextras
    INCLUDEPATH += "../../../vendors/zlib-1.2.11"
    INCLUDEPATH += "../../../vendors/quazip"
    INCLUDEPATH += "../../../vendors/libcurl/include"
    INCLUDEPATH += "../../../vendors/exiv2-0.25/include"

    LIBS -= -lcurl
    LIBS += -lmman

    LIBS += -llibexpat
    LIBS += -llibexiv2

    LIBS += -lavcodec
    LIBS += -lavfilter
    LIBS += -lavformat
    LIBS += -lavutil
    LIBS += -lswscale

    CONFIG(debug, debug|release) {
        EXE_DIR = debug
        LIBS += -llibcurl_debug
        LIBS -= -lquazip
        LIBS += -lquazipd
    }

    CONFIG(release, debug|release) {
        EXE_DIR = release
        LIBS += -llibcurl
    }

    # chillout dependencies
    LIBS += -lAdvapi32 -lDbgHelp
}

linux {
    message("for Linux")
    INCLUDEPATH += "../../../vendors/quazip"
    BUILDNO = $$system($$PWD/buildno.sh)

    LIBS += -lexiv2

    LIBS += -ldl

    LIBS += -lavcodec
    LIBS += -lavfilter
    LIBS += -lavformat
    LIBS += -lavutil
    LIBS += -lswscale
}

travis-ci {
    message("for Travis CI")
    INCLUDEPATH += "../../../vendors/quazip"

    LIBS -= -lz
    LIBS += /usr/lib/x86_64-linux-gnu/libz.so
    LIBS += -ldl
    LIBS += -lexiv2

    DEFINES += TRAVIS_CI

    # gcov
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -lgcov
}

appveyor {
    message("for Appveyor")
    DEFINES += APPVEYOR
}

without-video {
    message("Without video")

    LIBS -= -lavcodec
    LIBS -= -lavfilter
    LIBS -= -lavformat
    LIBS -= -lavutil
    LIBS -= -lswscale

    LIBS -= -lavcodec.57
    LIBS -= -lavfilter.6
    LIBS -= -lavformat.57
    LIBS -= -lavutil.55
    LIBS -= -lswscale.4

    LIBS -= -lthmbnlr
    SOURCES += ../../../vendors/libthmbnlr/thumbnailcreator_stub.cpp
}

DEFINES += BUILDNUMBER=$${BUILDNO}
DEFINES += BRANCHNAME=$${BRANCH_NAME}

DISTFILES += \
    ../../xpiks-common/xpiks-common.pri
