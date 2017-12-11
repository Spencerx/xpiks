QT += qml

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES += DEBUG_UTILITY

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += ../../vendors/libthmbnlr

SOURCES += main.cpp \
    ../xpiks-qt/Helpers/database.cpp \
    ../xpiks-qt/MetadataIO/cachedartwork.cpp \
    ../xpiks-qt/MetadataIO/metadatacache.cpp \
    ../xpiks-qt/Common/baseentity.cpp \
    ../xpiks-qt/Common/basickeywordsmodel.cpp \
    ../xpiks-qt/Common/basickeywordsmodelimpl.cpp \
    ../xpiks-qt/Common/basicmetadatamodel.cpp \
    ../xpiks-qt/Common/flags.cpp \
    ../../vendors/sqlite/sqlite3.c \
    ../xpiks-qt/SpellCheck/spellcheckitem.cpp \
    ../xpiks-qt/SpellCheck/spellcheckiteminfo.cpp \
    ../xpiks-qt/Models/artworkmetadata.cpp \
    ../xpiks-qt/Models/imageartwork.cpp \
    ../xpiks-qt/Models/videoartwork.cpp \
    ../xpiks-qt/Helpers/asynccoordinator.cpp \
    ../xpiks-qt/Helpers/stringhelper.cpp \
    ../xpiks-qt/SpellCheck/spellcheckerrorshighlighter.cpp \
    ../xpiks-qt/Helpers/indiceshelper.cpp \
    ../xpiks-qt/Helpers/keywordshelpers.cpp \
    ../xpiks-qt/QMLExtensions/colorsmodel.cpp

HEADERS += \
    ../xpiks-qt/Helpers/database.h \
    ../xpiks-qt/MetadataIO/cachedartwork.h \
    ../xpiks-qt/MetadataIO/metadatacache.h \
    ../xpiks-qt/MetadataIO/originalmetadata.h \
    ../xpiks-qt/Common/abstractlistmodel.h \
    ../xpiks-qt/Common/baseentity.h \
    ../xpiks-qt/Common/basickeywordsmodel.h \
    ../xpiks-qt/Common/basickeywordsmodelimpl.h \
    ../xpiks-qt/Common/basicmetadatamodel.h \
    ../xpiks-qt/Common/defines.h \
    ../xpiks-qt/Common/flags.h \
    ../xpiks-qt/Common/hold.h \
    ../xpiks-qt/Common/ibasicartwork.h \
    ../xpiks-qt/Common/iflagsprovider.h \
    ../xpiks-qt/Common/imetadataoperator.h \
    ../xpiks-qt/Common/keyword.h \
    ../../vendors/sqlite/sqlite3.h \
    ../xpiks-qt/SpellCheck/spellcheckitem.h \
    ../xpiks-qt/SpellCheck/spellcheckiteminfo.h \
    ../xpiks-qt/Models/artworkmetadata.h \
    ../xpiks-qt/Models/imageartwork.h \
    ../xpiks-qt/Models/videoartwork.h \
    ../xpiks-qt/Helpers/asynccoordinator.h \
    ../xpiks-qt/Helpers/stringhelper.h \
    ../xpiks-qt/SpellCheck/spellcheckerrorshighlighter.h \
    ../xpiks-qt/Helpers/indiceshelper.h \
    ../xpiks-qt/Helpers/keywordshelpers.h \
    ../xpiks-qt/QMLExtensions/colorsmodel.h \
    ../xpiks-qt/Helpers/constants.h
