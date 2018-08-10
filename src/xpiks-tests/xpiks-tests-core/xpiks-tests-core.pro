#-------------------------------------------------
#
# Project created by QtCreator 2015-02-12T22:20:15
#
#-------------------------------------------------

QMAKE_MAC_SDK = macosx10.11

QT       += core testlib qml quick concurrent

QT       -= gui

TARGET = xpiks-tests-core
CONFIG   += console
CONFIG   += testcase
CONFIG   -= app_bundle
CONFIG   += c++14

win32:QT += winextras

INCLUDEPATH += ../../../vendors/tiny-aes
win32:INCLUDEPATH += ../../xpiks-qt/Encryption
INCLUDEPATH += ../../../vendors/hunspell-repo/src
INCLUDEPATH += ../../../vendors/ssdll/src/ssdll
INCLUDEPATH += ../../../vendors/libthmbnlr
INCLUDEPATH += ../../../vendors/libxpks
INCLUDEPATH += ../../xpiks-qt

DEFINES += HUNSPELL_STATIC
DEFINES += CORE_TESTS

DEFINES += QT_MESSAGELOGCONTEXT

DEFINES += QT_NO_CAST_TO_ASCII \
           QT_RESTRICTED_CAST_FROM_ASCII \
           QT_NO_CAST_FROM_BYTEARRAY

CONFIG(debug, debug|release)  {
    LIBS += -L"$$PWD/../../../libs/debug"
} else {
    LIBS += -L"$$PWD/../../../libs/release"
}

LIBS += -lhunspell
LIBS += -lssdll

macx {
    #INCLUDEPATH += "../quazip"
    #INCLUDEPATH += "../../libcurl/include"
}

win32 {
    INCLUDEPATH += "../../../vendors/zlib-1.2.8"
    #INCLUDEPATH += "../quazip"
    #INCLUDEPATH += "../libcurl/include"
    #LIBS -= -lcurl
    #LIBS += -llibcurl_debug
}

travis-ci {
    message("for Travis CI")
    DEFINES += TRAVIS_CI

    # sqlite
    LIBS += -ldl

    # gcov
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    LIBS += -lgcov
}

appveyor {
    DEFINES += APPVEYOR
}

linux-g++-64 {
    target.path=/usr/bin/
    QML_IMPORT_PATH += /usr/lib/x86_64-linux-gnu/qt5/imports/
    UNAME = $$system(cat /proc/version)

    #contains(UNAME, Debian): {
    #    message("on Debian Linux")
    #    LIBS += -L/lib/x86_64-linux-gnu/
    #    LIBS -= -lquazip # temporary static link
    #    LIBS += /usr/lib/x86_64-linux-gnu/libquazip-qt5.a
    #}
    #contains(UNAME, SUSE): {
    #    message("on SUSE Linux")
    #    LIBS += -L/usr/lib64/
    #    LIBS += /usr/lib64/libcurl.so.4
    #}
}

TEMPLATE = app

SOURCES += main.cpp \
    encryption_tests.cpp \
    indicestoranges_tests.cpp \
    addcommand_tests.cpp \
    removecommand_tests.cpp \
    vectorfilenames_tests.cpp \
    artworkmetadata_tests.cpp \
    combinedmodel_tests.cpp \
    stringhelpers_tests.cpp \
    keywordvalidation_tests.cpp \
    artworkrepository_tests.cpp \
    filteredmodel_tests.cpp \
    undoredo_tests.cpp \
    artworkfilter_tests.cpp \
    removefilesfs_tests.cpp \
    recentitems_tests.cpp \
    fixspelling_tests.cpp \
    replacepreview_tests.cpp \
    replace_tests.cpp \
    stringhelpersfortests.cpp \
    deletekeywords_tests.cpp \
    basicmetadatamodel_tests.cpp \
    preset_tests.cpp \
    quickbuffer_tests.cpp \
    deleteoldlogs_tests.cpp \
    jsonmerge_tests.cpp \
    warningscheck_tests.cpp \
    dbimagecache_tests.cpp \
    ../../../vendors/tiny-aes/aes.cpp \
    ../../../vendors/sqlite/sqlite3.c \
    ../../xpiks-qt/Common/logging.cpp \
    ../../xpiks-qt/Common/statefulentity.cpp \
    ../../xpiks-qt/Common/systemenvironment.cpp \
    ../../xpiks-qt/Artworks/artworkmetadata.cpp \
    ../../xpiks-qt/Artworks/artworkssnapshot.cpp \
    ../../xpiks-qt/Artworks/basickeywordsmodel.cpp \
    ../../xpiks-qt/Artworks/basickeywordsmodelimpl.cpp \
    ../../xpiks-qt/Artworks/basicmetadatamodel.cpp \
    ../../xpiks-qt/Artworks/imageartwork.cpp \
    ../../xpiks-qt/Artworks/videoartwork.cpp \
    ../../xpiks-qt/Filesystem/directoriescollection.cpp \
    ../../xpiks-qt/Filesystem/filescollection.cpp \
    ../../xpiks-qt/Filesystem/filesdirectoriescollection.cpp \
    ../../xpiks-qt/Helpers/artworkshelpers.cpp \
    ../../xpiks-qt/Helpers/asynccoordinator.cpp \
    ../../xpiks-qt/Helpers/filehelpers.cpp \
    ../../xpiks-qt/Helpers/filterhelpers.cpp \
    ../../xpiks-qt/Helpers/indiceshelper.cpp \
    ../../xpiks-qt/Helpers/indicesranges.cpp \
    ../../xpiks-qt/Helpers/jsonhelper.cpp \
    ../../xpiks-qt/Helpers/keywordshelpers.cpp \
    ../../xpiks-qt/Helpers/localconfig.cpp \
    ../../xpiks-qt/Helpers/stringhelper.cpp \
    ../../xpiks-qt/Models/settingsmodel.cpp \
    ../../xpiks-qt/Models/Artworks/artworkslistmodel.cpp \
    ../../xpiks-qt/Models/Artworks/artworksrepository.cpp \
    ../../xpiks-qt/Models/Artworks/artworksviewmodel.cpp \
    ../../xpiks-qt/Models/Artworks/filteredartworkslistmodel.cpp \
    ../../xpiks-qt/Models/Editing/artworkproxybase.cpp \
    ../../xpiks-qt/Models/Editing/artworkproxymodel.cpp \
    ../../xpiks-qt/Models/Editing/combinedartworksmodel.cpp \
    ../../xpiks-qt/Models/Editing/currenteditableartwork.cpp \
    ../../xpiks-qt/Models/Editing/currenteditablemodel.cpp \
    ../../xpiks-qt/Models/Editing/currenteditableproxyartwork.cpp \
    ../../xpiks-qt/Models/Editing/quickbuffer.cpp \
    ../../xpiks-qt/Commands/Files/addfilescommand.cpp \
    Mocks/filescollectionmock.cpp \
    ../../xpiks-qt/UndoRedo/undoredomanager.cpp \
    ../../xpiks-qt/Models/Session/recentdirectoriesmodel.cpp \
    ../../xpiks-qt/Models/Session/recentitemsmodel.cpp \
    ../../xpiks-qt/Models/Connectivity/ziparchiver.cpp \
    artworkslistmodel_tests.cpp \
    ../../xpiks-qt/Services/artworksupdatehub.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckservice.cpp \
    ../../xpiks-qt/Encryption/aes-qt.cpp \
    ../../xpiks-qt/Commands/Files/removefilescommandbase.cpp \
    ../../xpiks-qt/Commands/Files/removeselectedfilescommand.cpp \
    ../../xpiks-qt/MetadataIO/cachedartwork.cpp \
    ../../xpiks-qt/Helpers/threadhelpers.cpp \
    ../../xpiks-qt/Commands/commandmanager.cpp \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodel.cpp \
    ../../xpiks-qt/Commands/Editing/editartworkstemplate.cpp \
    ../../xpiks-qt/Commands/Editing/findandreplacetemplate.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellsuggestionsitem.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckiteminfo.cpp \
    ../../xpiks-qt/Models/Editing/deletekeywordsviewmodel.cpp \
    ../../xpiks-qt/Services/Maintenance/logscleanupjobitem.cpp \
    ../../xpiks-qt/Services/Warnings/warningsitem.cpp \
    ../../xpiks-qt/Services/Warnings/warningssettingsmodel.cpp \
    ../../xpiks-qt/QMLExtensions/cachedimage.cpp \
    ../../xpiks-qt/QMLExtensions/dbimagecacheindex.cpp \
    ../../xpiks-qt/Commands/Editing/expandpresettemplate.cpp \
    ../../xpiks-qt/Commands/Editing/keywordedittemplate.cpp \
    ../../xpiks-qt/Models/keyvaluelist.cpp \
    ../../xpiks-qt/Services/SpellCheck/userdictionary.cpp \
    ../../xpiks-qt/Models/Session/recentfilesmodel.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellchecksuggestionmodel.cpp \
    ../../xpiks-qt/Storage/databasemanager.cpp \
    ../../xpiks-qt/Commands/Editing/deletekeywordstemplate.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckitem.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckworker.cpp \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodelconfig.cpp \
    ../../xpiks-qt/UndoRedo/artworkmetadatabackup.cpp \
    ../../xpiks-qt/Models/Connectivity/abstractconfigupdatermodel.cpp \
    ../../xpiks-qt/Storage/memorytable.cpp \
    ../../xpiks-qt/Commands/artworksupdatetemplate.cpp \
    ../../xpiks-qt/KeywordsPresets/presetgroupsmodel.cpp \
    ../../xpiks-qt/Helpers/remoteconfig.cpp \
    ../../xpiks-qt/Storage/database.cpp \
    ../../xpiks-qt/Connectivity/configrequest.cpp \
    ../../xpiks-qt/Encryption/obfuscation.cpp \
    ../../xpiks-qt/Models/Connectivity/proxysettings.cpp \
    ../../xpiks-qt/Commands/Files/removedirectorycommand.cpp \
    ../../xpiks-qt/Suggestion/keywordssuggestor.cpp \
    ../../xpiks-qt/Models/switchermodel.cpp \
    ../../xpiks-qt/Connectivity/switcherconfig.cpp

HEADERS += \
    encryption_tests.h \
    indicestoranges_tests.h \
    Mocks/commandmanagermock.h \
    addcommand_tests.h \
    Mocks/artworkmetadatamock.h \
    removecommand_tests.h \
    vectorfilenames_tests.h \
    artworkmetadata_tests.h \
    combinedmodel_tests.h \
    stringhelpers_tests.h \
    keywordvalidation_tests.h \
    artworkrepository_tests.h \
    filteredmodel_tests.h \
    undoredo_tests.h \
    artworkfilter_tests.h \
    removefilesfs_tests.h \
    Mocks/artworksrepositorymock.h \
    recentitems_tests.h \
    removefilesfs_tests.h \
    fixspelling_tests.h \
    Mocks/spellcheckservicemock.h \
    replacepreview_tests.h \
    replace_tests.h \
    stringhelpersfortests.h \
    deletekeywords_tests.h \
    basicmetadatamodel_tests.h \
    preset_tests.h \
    quickbuffer_tests.h \
    deleteoldlogs_tests.h \
    jsonmerge_tests.h \
    warningscheck_tests.h \
    Mocks/warningssettingsmock.h \
    Mocks/coretestsenvironment.h \
    dbimagecache_tests.h \
    ../../../vendors/tiny-aes/aes.h \
    ../../../vendors/sqlite/sqlite3.h \
    ../../xpiks-qt/Common/abstractlistmodel.h \
    ../../xpiks-qt/Common/defines.h \
    ../../xpiks-qt/Common/delayedactionentity.h \
    ../../xpiks-qt/Common/flags.h \
    ../../xpiks-qt/Common/hold.h \
    ../../xpiks-qt/Common/iflagsprovider.h \
    ../../xpiks-qt/Common/irefcountedobject.h \
    ../../xpiks-qt/Common/isystemenvironment.h \
    ../../xpiks-qt/Common/logging.h \
    ../../xpiks-qt/Common/messages.h \
    ../../xpiks-qt/Common/statefulentity.h \
    ../../xpiks-qt/Common/systemenvironment.h \
    ../../xpiks-qt/Common/types.h \
    ../../xpiks-qt/Common/version.h \
    ../../xpiks-qt/Common/wordanalysisresult.h \
    ../../xpiks-qt/Artworks/artworkmetadata.h \
    ../../xpiks-qt/Artworks/artworkssnapshot.h \
    ../../xpiks-qt/Artworks/basickeywordsmodel.h \
    ../../xpiks-qt/Artworks/basickeywordsmodelimpl.h \
    ../../xpiks-qt/Artworks/basicmetadatamodel.h \
    ../../xpiks-qt/Artworks/iartworkmetadata.h \
    ../../xpiks-qt/Artworks/imageartwork.h \
    ../../xpiks-qt/Artworks/iselectedartworkssource.h \
    ../../xpiks-qt/Artworks/iselectedindicessource.h \
    ../../xpiks-qt/Artworks/keyword.h \
    ../../xpiks-qt/Artworks/videoartwork.h \
    ../../xpiks-qt/Filesystem/directoriescollection.h \
    ../../xpiks-qt/Filesystem/filescollection.h \
    ../../xpiks-qt/Filesystem/filesdirectoriescollection.h \
    ../../xpiks-qt/Filesystem/ifilescollection.h \
    ../../xpiks-qt/Helpers/artworkshelpers.h \
    ../../xpiks-qt/Helpers/asynccoordinator.h \
    ../../xpiks-qt/Helpers/comparevaluesjson.h \
    ../../xpiks-qt/Helpers/cpphelpers.h \
    ../../xpiks-qt/Helpers/filehelpers.h \
    ../../xpiks-qt/Helpers/filterhelpers.h \
    ../../xpiks-qt/Helpers/indiceshelper.h \
    ../../xpiks-qt/Helpers/indicesranges.h \
    ../../xpiks-qt/Helpers/jsonhelper.h \
    ../../xpiks-qt/Helpers/jsonobjectmap.h \
    ../../xpiks-qt/Helpers/keywordshelpers.h \
    ../../xpiks-qt/Helpers/localconfig.h \
    ../../xpiks-qt/Helpers/stringhelper.h \
    ../../xpiks-qt/Models/settingsmodel.h \
    ../../xpiks-qt/Models/Artworks/artworkslistmodel.h \
    ../../xpiks-qt/Models/Artworks/artworksrepository.h \
    ../../xpiks-qt/Models/Artworks/artworksviewmodel.h \
    ../../xpiks-qt/Models/Artworks/filteredartworkslistmodel.h \
    ../../xpiks-qt/Models/Editing/artworkproxybase.h \
    ../../xpiks-qt/Models/Editing/artworkproxymodel.h \
    ../../xpiks-qt/Models/Editing/combinedartworksmodel.h \
    ../../xpiks-qt/Models/Editing/currenteditableartwork.h \
    ../../xpiks-qt/Models/Editing/currenteditablemodel.h \
    ../../xpiks-qt/Models/Editing/currenteditableproxyartwork.h \
    ../../xpiks-qt/Models/Editing/icurrenteditable.h \
    ../../xpiks-qt/Models/Editing/quickbuffer.h \
    ../../xpiks-qt/Models/Editing/quickbuffermessage.h \
    Mocks/artworkslistmodelmock.h \
    ../../xpiks-qt/Commands/Base/icommand.h \
    ../../xpiks-qt/Commands/Base/icommandmanager.h \
    ../../xpiks-qt/Commands/Base/icommandtemplate.h \
    ../../xpiks-qt/Commands/Base/templatedcommand.h \
    ../../xpiks-qt/Commands/Files/addfilescommand.h \
    Mocks/filescollectionmock.h \
    ../../xpiks-qt/UndoRedo/iundoredomanager.h \
    ../../xpiks-qt/UndoRedo/undoredomanager.h \
    ../../xpiks-qt/Models/Session/recentdirectoriesmodel.h \
    ../../xpiks-qt/Models/Session/recentitemsmodel.h \
    Mocks/selectedindicessourcemock.h \
    ../../xpiks-qt/Artworks/artworkelement.h \
    ../../xpiks-qt/Services/AutoComplete/icompletionsource.h \
    ../../xpiks-qt/Models/Connectivity/ziparchiver.h \
    artworkslistmodel_tests.h \
    ../../xpiks-qt/Services/artworksupdatehub.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckservice.h \
    Mocks/flagsprovidermock.h \
    Mocks/artworksupdatermock.h \
    ../../xpiks-qt/Encryption/aes-qt.h \
    ../../xpiks-qt/Commands/Files/removefilescommandbase.h \
    ../../xpiks-qt/Commands/Files/removeselectedfilescommand.h \
    ../../xpiks-qt/MetadataIO/cachedartwork.h \
    ../../xpiks-qt/Helpers/threadhelpers.h \
    ../../xpiks-qt/Commands/commandmanager.h \
    ../../xpiks-qt/KeywordsPresets/ipresetsmanager.h \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodel.h \
    ../../xpiks-qt/Commands/Editing/editartworkstemplate.h \
    ../../xpiks-qt/Commands/Editing/findandreplacetemplate.h \
    ../../xpiks-qt/Services/SpellCheck/spellsuggestionsitem.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckiteminfo.h \
    ../../xpiks-qt/Models/Editing/deletekeywordsviewmodel.h \
    ../../xpiks-qt/Services/Maintenance/imaintenanceitem.h \
    ../../xpiks-qt/Services/Maintenance/logscleanupjobitem.h \
    ../../xpiks-qt/Services/Warnings/iwarningsitem.h \
    ../../xpiks-qt/Services/Warnings/warningsitem.h \
    ../../xpiks-qt/Services/Warnings/warningssettingsmodel.h \
    ../../xpiks-qt/QMLExtensions/cachedimage.h \
    ../../xpiks-qt/Storage/idatabase.h \
    ../../xpiks-qt/Storage/idatabasemanager.h \
    ../../xpiks-qt/QMLExtensions/dbimagecacheindex.h \
    ../../xpiks-qt/Commands/Editing/expandpresettemplate.h \
    ../../xpiks-qt/Commands/Editing/keywordedittemplate.h \
    ../../xpiks-qt/Models/keyvaluelist.h \
    ../../xpiks-qt/Services/SpellCheck/userdictionary.h \
    ../../xpiks-qt/Models/Session/recentfilesmodel.h \
    ../../xpiks-qt/Services/SpellCheck/spellchecksuggestionmodel.h \
    ../../xpiks-qt/Storage/databasemanager.h \
    ../../xpiks-qt/Commands/Editing/deletekeywordstemplate.h \
    ../../xpiks-qt/Services/SpellCheck/ispellcheckable.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckitem.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckworker.h \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodelconfig.h \
    ../../xpiks-qt/UndoRedo/artworkmetadatabackup.h \
    ../../xpiks-qt/Models/Connectivity/abstractconfigupdatermodel.h \
    ../../xpiks-qt/Storage/memorytable.h \
    ../../xpiks-qt/Commands/artworksupdatetemplate.h \
    ../../xpiks-qt/KeywordsPresets/presetgroupsmodel.h \
    ../../xpiks-qt/Helpers/remoteconfig.h \
    ../../xpiks-qt/Storage/database.h \
    ../../xpiks-qt/Connectivity/configrequest.h \
    ../../xpiks-qt/Encryption/obfuscation.h \
    ../../xpiks-qt/Models/Connectivity/proxysettings.h \
    ../../xpiks-qt/Commands/Files/removedirectorycommand.h \
    ../../xpiks-qt/Models/Artworks/artworkslistoperations.h \
    Mocks/microstockclientsmock.h \
    ../../xpiks-qt/Suggestion/keywordssuggestor.h \
    ../../xpiks-qt/Models/switchermodel.h \
    ../../xpiks-qt/Connectivity/switcherconfig.h

