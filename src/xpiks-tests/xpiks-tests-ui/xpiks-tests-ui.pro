TEMPLATE = app

QT += qml quick widgets concurrent svg

CONFIG += qmltestcase plugin c++14
CONFIG -= app_bundle

TARGET = xpiks-tests-ui

DEFINES += QML_IMPORT_TRACE
DEFINES += QT_DEBUG_PLUGINS

INCLUDEPATH += ../../xpiks-qt/
INCLUDEPATH += ../../../

BUILDNO = $$system(git log -n 1 --pretty=format:"%H")

DEFINES += QT_NO_CAST_TO_ASCII \
           QT_RESTRICTED_CAST_FROM_ASCII \
           QT_NO_CAST_FROM_BYTEARRAY

DEFINES += HUNSPELL_STATIC
DEFINES += TELEMETRY_ENABLED
DEFINES += WITH_STDOUT_LOGS
DEFINES += WITH_LOGS
DEFINES += VERBOSE_LOGGING
DEFINES += WITH_FTP_SERVER

DEFINES += UI_TESTS

DEFINES += FAKE_WARNINGS
#DEFINES += QT_FATAL_WARNINGS

STUB_PLUGIN_NAME = UiTestsStubPlugin

DEFINES += XPIKS_QT_DIR="$$PWD/../../xpiks-qt"

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

# Default rules for deployment.
include(deployment.pri)

BUILDNO=$$system(git log -n 1 --pretty=format:"%h")
BRANCH_NAME=$$system(git rev-parse --abbrev-ref HEAD)

macx {
    INCLUDEPATH += "../../../vendors/libcurl/include"
}

win32 {
    DEFINES += QT_NO_PROCESS_COMBINED_ARGUMENT_START
    QT += winextras
    INCLUDEPATH += "../../../vendors/zlib-1.2.11"
    INCLUDEPATH += "../../../vendors/libcurl/include"

    LIBS -= -lcurl
    LIBS += -lmman

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

    LIBS += -ldl
}

travis-ci {
    message("for Travis CI")

    DEFINES += TRAVIS_CI

    linux {
        LIBS -= -lz
        LIBS += /usr/lib/x86_64-linux-gnu/libz.so

        # gcov
        QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
        LIBS += -lgcov
    }

    macx {
        CONFIG += sdk_no_version_check

        QMAKE_CXXFLAGS += --coverage
        QMAKE_LFLAGS += --coverage

        DEFINES -= WITH_FTP_SERVER
    }
}

appveyor {
    message("for Appveyor")
    DEFINES += APPVEYOR
    DEFINES -= WITH_FTP_SERVER
}

# without-video {
{
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

include(../../xpiks-common/xpiks-common.pri)

DEFINES += BUILDNUMBER=$${BUILDNO}
DEFINES += BRANCHNAME=$${BRANCH_NAME}

SOURCES += main.cpp \
    testshost.cpp \
    ../../../vendors/tiny-aes/aes.cpp \
    ../../../vendors/sqlite/sqlite3.c \
    ../../xpiks-qt/Artworks/artworkmetadata.cpp \
    ../../xpiks-qt/Artworks/artworkssnapshot.cpp \
    ../../xpiks-qt/Artworks/basickeywordsmodel.cpp \
    ../../xpiks-qt/Artworks/basickeywordsmodelimpl.cpp \
    ../../xpiks-qt/Artworks/basicmetadatamodel.cpp \
    ../../xpiks-qt/Artworks/imageartwork.cpp \
    ../../xpiks-qt/Artworks/videoartwork.cpp \
    ../../xpiks-qt/Commands/commandmanager.cpp \
    ../../xpiks-qt/Commands/Editing/deletekeywordstemplate.cpp \
    ../../xpiks-qt/Commands/Editing/editartworkstemplate.cpp \
    ../../xpiks-qt/Commands/Editing/expandpresettemplate.cpp \
    ../../xpiks-qt/Commands/Editing/findandreplacetemplate.cpp \
    ../../xpiks-qt/Commands/Editing/keywordedittemplate.cpp \
    ../../xpiks-qt/Commands/Editing/readmetadatatemplate.cpp \
    ../../xpiks-qt/Commands/Files/addfilescommand.cpp \
    ../../xpiks-qt/Commands/Files/addtorecenttemplate.cpp \
    ../../xpiks-qt/Commands/Files/removedirectorycommand.cpp \
    ../../xpiks-qt/Commands/Files/removefilescommandbase.cpp \
    ../../xpiks-qt/Commands/Files/removeselectedfilescommand.cpp \
    ../../xpiks-qt/Commands/Services/autoimportmetadatacommand.cpp \
    ../../xpiks-qt/Commands/Services/cleanuplegacybackupscommand.cpp \
    ../../xpiks-qt/Commands/Services/generatethumbnailstemplate.cpp \
    ../../xpiks-qt/Commands/Services/savebackupstemplate.cpp \
    ../../xpiks-qt/Commands/Services/savesessioncommand.cpp \
    ../../xpiks-qt/Common/logging.cpp \
    ../../xpiks-qt/Common/statefulentity.cpp \
    ../../xpiks-qt/Common/systemenvironment.cpp \
    ../../xpiks-qt/Connectivity/configrequest.cpp \
    ../../xpiks-qt/Connectivity/curlinithelper.cpp \
    ../../xpiks-qt/Connectivity/ftphelpers.cpp \
    ../../xpiks-qt/Connectivity/requestsservice.cpp \
    ../../xpiks-qt/Connectivity/requestsworker.cpp \
    ../../xpiks-qt/Connectivity/simplecurldownloader.cpp \
    ../../xpiks-qt/Connectivity/simplecurlrequest.cpp \
    ../../xpiks-qt/Connectivity/switcherconfig.cpp \
    ../../xpiks-qt/Connectivity/telemetryservice.cpp \
    ../../xpiks-qt/Connectivity/telemetryworker.cpp \
    ../../xpiks-qt/Encryption/aes-qt.cpp \
    ../../xpiks-qt/Encryption/obfuscation.cpp \
    ../../xpiks-qt/Encryption/secretsmanager.cpp \
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
    ../../xpiks-qt/Helpers/logger.cpp \
    ../../xpiks-qt/Helpers/loggingworker.cpp \
    ../../xpiks-qt/Helpers/remoteconfig.cpp \
    ../../xpiks-qt/Helpers/stringhelper.cpp \
    ../../xpiks-qt/Helpers/threadhelpers.cpp \
    ../../xpiks-qt/Helpers/ziphelper.cpp \
    ../../xpiks-qt/KeywordsPresets/presetgroupsmodel.cpp \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodel.cpp \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodelconfig.cpp \
    ../../xpiks-qt/MetadataIO/cachedartwork.cpp \
    ../../xpiks-qt/MetadataIO/csvexportmodel.cpp \
    ../../xpiks-qt/MetadataIO/csvexportplansmodel.cpp \
    ../../xpiks-qt/MetadataIO/csvexportproperties.cpp \
    ../../xpiks-qt/MetadataIO/csvexportworker.cpp \
    ../../xpiks-qt/MetadataIO/metadatacache.cpp \
    ../../xpiks-qt/MetadataIO/metadataiocoordinator.cpp \
    ../../xpiks-qt/MetadataIO/metadataioservice.cpp \
    ../../xpiks-qt/MetadataIO/metadataioworker.cpp \
    ../../xpiks-qt/MetadataIO/metadatareadinghub.cpp \
    ../../xpiks-qt/Microstocks/fotoliaapiclient.cpp \
    ../../xpiks-qt/Microstocks/gettyapiclient.cpp \
    ../../xpiks-qt/Microstocks/microstockservice.cpp \
    ../../xpiks-qt/Microstocks/shutterstockapiclient.cpp \
    ../../xpiks-qt/Microstocks/stocksftplistmodel.cpp \
    ../../xpiks-qt/Models/Artworks/artworkslistmodel.cpp \
    ../../xpiks-qt/Models/Artworks/artworksrepository.cpp \
    ../../xpiks-qt/Models/Artworks/artworksviewmodel.cpp \
    ../../xpiks-qt/Models/Artworks/filteredartworkslistmodel.cpp \
    ../../xpiks-qt/Models/Connectivity/abstractconfigupdatermodel.cpp \
    ../../xpiks-qt/Models/Connectivity/artworksuploader.cpp \
    ../../xpiks-qt/Models/Connectivity/proxysettings.cpp \
    ../../xpiks-qt/Models/Connectivity/uploadinforepository.cpp \
    ../../xpiks-qt/Models/Connectivity/ziparchiver.cpp \
    ../../xpiks-qt/Models/Editing/artworkproxybase.cpp \
    ../../xpiks-qt/Models/Editing/artworkproxymodel.cpp \
    ../../xpiks-qt/Models/Editing/combinedartworksmodel.cpp \
    ../../xpiks-qt/Models/Editing/currenteditableartwork.cpp \
    ../../xpiks-qt/Models/Editing/currenteditablemodel.cpp \
    ../../xpiks-qt/Models/Editing/currenteditableproxyartwork.cpp \
    ../../xpiks-qt/Models/Editing/deletekeywordsviewmodel.cpp \
    ../../xpiks-qt/Models/Editing/findandreplacemodel.cpp \
    ../../xpiks-qt/Models/Editing/quickbuffer.cpp \
    ../../xpiks-qt/Models/Session/recentdirectoriesmodel.cpp \
    ../../xpiks-qt/Models/Session/recentfilesmodel.cpp \
    ../../xpiks-qt/Models/Session/recentitemsmodel.cpp \
    ../../xpiks-qt/Models/Session/sessionmanager.cpp \
    ../../xpiks-qt/Models/keyvaluelist.cpp \
    ../../xpiks-qt/Models/languagesmodel.cpp \
    ../../xpiks-qt/Models/logsmodel.cpp \
    ../../xpiks-qt/Models/settingsmodel.cpp \
    ../../xpiks-qt/Models/switchermodel.cpp \
    ../../xpiks-qt/Plugins/pluginactionsmodel.cpp \
    ../../xpiks-qt/Plugins/plugindatabasemanager.cpp \
    ../../xpiks-qt/Plugins/pluginenvironment.cpp \
    ../../xpiks-qt/Plugins/pluginmanager.cpp \
    ../../xpiks-qt/Plugins/pluginwrapper.cpp \
    ../../xpiks-qt/Plugins/sandboxeddependencies.cpp \
    ../../xpiks-qt/Plugins/uiprovider.cpp \
    ../../xpiks-qt/Services/artworksupdatehub.cpp \
    ../../xpiks-qt/Services/AutoComplete/autocompletemodel.cpp \
    ../../xpiks-qt/Services/AutoComplete/autocompleteservice.cpp \
    ../../xpiks-qt/Services/AutoComplete/autocompleteworker.cpp \
    ../../xpiks-qt/Services/AutoComplete/keywordsautocompletemodel.cpp \
    ../../xpiks-qt/Services/AutoComplete/libfacecompletionengine.cpp \
    ../../xpiks-qt/Services/AutoComplete/presetscompletionengine.cpp \
    ../../xpiks-qt/Services/AutoComplete/stringsautocompletemodel.cpp \
    ../../xpiks-qt/Services/SpellCheck/duplicateshighlighter.cpp \
    ../../xpiks-qt/Services/SpellCheck/duplicatesreviewmodel.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckerrorshighlighter.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckitem.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckinfo.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckservice.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellchecksuggestionmodel.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellcheckworker.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellsuggestionsitem.cpp \
    ../../xpiks-qt/Services/SpellCheck/userdicteditmodel.cpp \
    ../../xpiks-qt/Services/SpellCheck/userdictionary.cpp \
    ../../xpiks-qt/Services/Translation/translationmanager.cpp \
    ../../xpiks-qt/Services/Translation/translationquery.cpp \
    ../../xpiks-qt/Services/Translation/translationservice.cpp \
    ../../xpiks-qt/Services/Translation/translationworker.cpp \
    ../../xpiks-qt/Services/Warnings/warningscheckingworker.cpp \
    ../../xpiks-qt/Services/Warnings/warningsitem.cpp \
    ../../xpiks-qt/Services/Warnings/warningsmodel.cpp \
    ../../xpiks-qt/Services/Warnings/warningsservice.cpp \
    ../../xpiks-qt/Services/Warnings/warningssettingsmodel.cpp \
    ../../xpiks-qt/Storage/database.cpp \
    ../../xpiks-qt/Storage/databasemanager.cpp \
    ../../xpiks-qt/Storage/memorytable.cpp \
    ../../xpiks-qt/Suggestion/fotoliasuggestionengine.cpp \
    ../../xpiks-qt/Suggestion/gettysuggestionengine.cpp \
    ../../xpiks-qt/Suggestion/keywordssuggestor.cpp \
    ../../xpiks-qt/Suggestion/locallibraryqueryengine.cpp \
    ../../xpiks-qt/Suggestion/shutterstocksuggestionengine.cpp \
    ../../xpiks-qt/Suggestion/suggestionresultsresponse.cpp \
    ../../xpiks-qt/UndoRedo/artworkmetadatabackup.cpp \
    ../../xpiks-qt/UndoRedo/undoredomanager.cpp \
    ../../xpiks-qt/xpiksapp.cpp \
    ../../xpiks-qt/QMLExtensions/uicommanddispatcher.cpp \
    ../../xpiks-qt/Commands/UI/generalcommands.cpp \
    ../../xpiks-qt/Commands/UI/selectedartworkscommands.cpp \
    ../../xpiks-qt/Commands/UI/singleeditablecommands.cpp \
    ../../xpiks-qt/Services/Maintenance/initializedictionariesjobitem.cpp \
    ../../xpiks-qt/Services/Maintenance/launchexiftooljobitem.cpp \
    ../../xpiks-qt/Services/Maintenance/logscleanupjobitem.cpp \
    ../../xpiks-qt/Services/Maintenance/maintenanceservice.cpp \
    ../../xpiks-qt/Services/Maintenance/maintenanceworker.cpp \
    ../../xpiks-qt/Services/Maintenance/savesessionjobitem.cpp \
    ../../xpiks-qt/Services/Maintenance/updatebundlecleanupjobitem.cpp \
    ../../xpiks-qt/Services/Maintenance/updatescleanupjobitem.cpp \
    ../../xpiks-qt/Services/Maintenance/xpkscleanupjob.cpp \
    ../../xpiks-qt/Connectivity/updatescheckerworker.cpp \
    ../../xpiks-qt/Connectivity/updateservice.cpp \
    ../../xpiks-qt/Connectivity/uploadwatcher.cpp \
    ../../xpiks-qt/QMLExtensions/colorsmodel.cpp \
    ../../xpiks-qt/QMLExtensions/cachedimage.cpp \
    ../../xpiks-qt/QMLExtensions/cachedvideo.cpp \
    ../../xpiks-qt/QMLExtensions/dbimagecacheindex.cpp \
    ../../xpiks-qt/QMLExtensions/dbvideocacheindex.cpp \
    ../../xpiks-qt/QMLExtensions/imagecachingservice.cpp \
    ../../xpiks-qt/QMLExtensions/imagecachingworker.cpp \
    ../../xpiks-qt/QMLExtensions/videocachingservice.cpp \
    ../../xpiks-qt/QMLExtensions/videocachingworker.cpp \
    ../../xpiks-qt/Commands/Files/removefilescommand.cpp \
    ../../xpiks-qt/Suggestion/suggestedkeywords.cpp \
    ../../xpiks-qt/Models/uimanager.cpp \
    ../../xpiks-qt/Commands/artworksupdatetemplate.cpp \
    ../../xpiks-qt/QMLExtensions/tabsmodel.cpp \
    ../../xpiks-qt/Helpers/updatehelpers.cpp \
    ../../xpiks-qt/Connectivity/testconnection.cpp \
    ../../xpiks-qt/Helpers/loghighlighter.cpp \
    ../../xpiks-qt/Helpers/helpersqmlwrapper.cpp \
    ../../xpiks-qt/Services/SpellCheck/metadataduplicates.cpp \
    ../../xpiks-qt/Services/SpellCheck/spellsuggestionstarget.cpp \
    ../../xpiks-qt/Services/artworkseditinghub.cpp \
    xpiksuitestsapp.cpp \
    ../../xpiks-qt/QMLExtensions/triangleelement.cpp \
    ../xpiks-tests-integration/testshelpers.cpp \
    ../xpiks-tests-core/Mocks/filescollectionmock.cpp \
    fakeinitartworkstemplate.cpp \
    ../../xpiks-qt/Helpers/metadatahighlighter.cpp \
    ../../xpiks-qt/QMLExtensions/folderelement.cpp \
    ../../xpiks-qt/Helpers/uihelpers.cpp \
    ../../xpiks-qt/QMLExtensions/uicommandlistener.cpp \
    ../../xpiks-qt/Commands/Base/actionmodelcommand.cpp \
    ../../xpiks-qt/Commands/Editing/clearactionmodeltemplate.cpp \
    ../../xpiks-qt/Commands/Base/compositecommand.cpp \
    ../../xpiks-qt/Commands/UI/currenteditablecommands.cpp \
    ../../xpiks-qt/QMLExtensions/proxyindexmiddlware.cpp \
    ../../xpiks-qt/Connectivity/curlhelpers.cpp \
    ../xpiks-tests-integration/integrationtestsenvironment.cpp

DISTFILES += \
    ../../xpiks-qt/Components/EditableTags.qml \
    tst_EditableTags.qml \
    ../../xpiks-qt/StackViews/CombinedEditView.qml \
    tst_CombinedEditView.qml \
    tst_ArtworkEditView.qml \
    tst_CsvExportDialog.qml \
    ../../xpiks-qt/Dialogs/CsvExportDialog.qml \
    tst_UploadDialog.qml \
    tst_PresetsEditDialog.qml \
    tst_WhatsNew.qml \
    ../../xpiks-qt/AppHost.qml \
    ../../xpiks-qt/Common.js \
    ../../xpiks-qt/StackViews/ArtworkEditView.qml \
    ../../xpiks-qt/StackViews/DuplicatesReView.qml \
    ../../xpiks-qt/StackViews/MainGrid.qml \
    ../../xpiks-qt/StackViews/WarningsView.qml \
    ../../xpiks-qt/Dialogs/AboutWindow.qml \
    ../../xpiks-qt/Dialogs/AddPresetGroupDialog.qml \
    ../../xpiks-qt/Dialogs/DeleteKeywordsDialog.qml \
    ../../xpiks-qt/Dialogs/DonateDialog.qml \
    ../../xpiks-qt/Dialogs/EditKeywordDialog.qml \
    ../../xpiks-qt/Dialogs/EnterMasterPasswordDialog.qml \
    ../../xpiks-qt/Dialogs/ExportMetadata.qml \
    ../../xpiks-qt/Dialogs/FailedUploadArtworks.qml \
    ../../xpiks-qt/Dialogs/FindAndReplace.qml \
    ../../xpiks-qt/Dialogs/ImportMetadata.qml \
    ../../xpiks-qt/Dialogs/InstallUpdateDialog.qml \
    ../../xpiks-qt/Dialogs/KeywordsSuggestion.qml \
    ../../xpiks-qt/Dialogs/LogsDialog.qml \
    ../../xpiks-qt/Dialogs/MasterPasswordSetupDialog.qml \
    ../../xpiks-qt/Dialogs/PlainTextKeywordsDialog.qml \
    ../../xpiks-qt/Dialogs/PluginsDialog.qml \
    ../../xpiks-qt/Dialogs/PresetsEditDialog.qml \
    ../../xpiks-qt/Dialogs/ProxySetupDialog.qml \
    ../../xpiks-qt/Dialogs/ReplacePreview.qml \
    ../../xpiks-qt/Dialogs/SettingsWindow.qml \
    ../../xpiks-qt/Dialogs/SimplePreview.qml \
    ../../xpiks-qt/Dialogs/SpellCheckSuggestionsDialog.qml \
    ../../xpiks-qt/Dialogs/TermsAndConditionsDialog.qml \
    ../../xpiks-qt/Dialogs/TranslationPreviewDialog.qml \
    ../../xpiks-qt/Dialogs/UpdateWindow.qml \
    ../../xpiks-qt/Dialogs/UploadArtworks.qml \
    ../../xpiks-qt/Dialogs/UserDictEditDialog.qml \
    ../../xpiks-qt/Dialogs/WarningsDialog.qml \
    ../../xpiks-qt/Dialogs/WhatsNewDialog.qml \
    ../../xpiks-qt/Dialogs/WhatsNewMinorDialog.qml \
    ../../xpiks-qt/Dialogs/WipeMetadata.qml \
    ../../xpiks-qt/Dialogs/ZipArtworksDialog.qml \
    ../../xpiks-qt/Components/AddIcon.qml \
    ../../xpiks-qt/Components/BackGlyphButton.qml \
    ../../xpiks-qt/Components/CheckedComponent.qml \
    ../../xpiks-qt/Components/CloseIcon.qml \
    ../../xpiks-qt/Components/ComboBoxPopup.qml \
    ../../xpiks-qt/Components/CompletionBox.qml \
    ../../xpiks-qt/Components/CustomBorder.qml \
    ../../xpiks-qt/Components/CustomScrollbar.qml \
    ../../xpiks-qt/Components/CustomTab.qml \
    ../../xpiks-qt/Components/CustomTooltip.qml \
    ../../xpiks-qt/Components/DonateComponent.qml \
    ../../xpiks-qt/Components/DotsButton.qml \
    ../../xpiks-qt/Components/DraggableKeywordWrapper.qml \
    ../../xpiks-qt/Components/DropdownPopup.qml \
    ../../xpiks-qt/Components/GlyphButton.qml \
    ../../xpiks-qt/Components/KeywordWrapper.qml \
    ../../xpiks-qt/Components/LargeAddIcon.qml \
    ../../xpiks-qt/Components/LayoutButton.qml \
    ../../xpiks-qt/Components/LoaderIcon.qml \
    ../../xpiks-qt/Components/PresentationSlide.qml \
    ../../xpiks-qt/Components/SelectedIcon.qml \
    ../../xpiks-qt/Components/SuggestionWrapper.qml \
    ../../xpiks-qt/Components/ToolButton.qml \
    ../../xpiks-qt/Components/ZoomAmplifier.qml \
    ../../xpiks-qt/Constants/UIConfig.js \
    TestUtils.js \
    tst_MainGrid.qml \
    tst_DeleteKeywordsDialog.qml \
    tst_KeywordsSuggestion.qml \
    ../../xpiks-qt/CollapserTabs/FilesFoldersTab.qml \
    ../../xpiks-qt/CollapserTabs/QuickBufferTab.qml \
    ../../xpiks-qt/CollapserTabs/TranslatorTab.qml \
    ../../xpiks-qt/CollapserTabs/FilesFoldersIcon.qml \
    ../../xpiks-qt/CollapserTabs/QuickBufferIcon.qml \
    ../../xpiks-qt/CollapserTabs/TranslatorIcon.qml \
    tst_QuickBuffer.qml \
    tst_UserDictEditDialog.qml \
    tst_AppHost.qml \
    ../../xpiks-qt/Components/BaseDialog.qml \
    ../../xpiks-qt/Components/StaticDialogBase.qml

HEADERS += \
    testshost.h \
    ../../../vendors/tiny-aes/aes.h \
    ../../../vendors/sqlite/sqlite3.h \
    ../../../vendors/csv/csv.h \
    ../../xpiks-qt/Artworks/artworkelement.h \
    ../../xpiks-qt/Artworks/artworkmetadata.h \
    ../../xpiks-qt/Artworks/artworkssnapshot.h \
    ../../xpiks-qt/Artworks/basickeywordsmodel.h \
    ../../xpiks-qt/Artworks/basickeywordsmodelimpl.h \
    ../../xpiks-qt/Artworks/basicmetadatamodel.h \
    ../../xpiks-qt/Artworks/imageartwork.h \
    ../../xpiks-qt/Artworks/iselectedartworkssource.h \
    ../../xpiks-qt/Artworks/iselectedindicessource.h \
    ../../xpiks-qt/Artworks/keyword.h \
    ../../xpiks-qt/Artworks/videoartwork.h \
    ../../xpiks-qt/Commands/Base/commanduiwrapper.h \
    ../../xpiks-qt/Commands/Base/compositecommandtemplate.h \
    ../../xpiks-qt/Commands/Base/emptycommand.h \
    ../../xpiks-qt/Commands/Base/icommand.h \
    ../../xpiks-qt/Commands/Base/icommandmanager.h \
    ../../xpiks-qt/Commands/Base/icommandtemplate.h \
    ../../xpiks-qt/Commands/Base/iuicommandtemplate.h \
    ../../xpiks-qt/Commands/Base/callbackcommand.h \
    ../../xpiks-qt/Commands/Base/templatedcommand.h \
    ../../xpiks-qt/Commands/Base/templateduicommand.h \
    ../../xpiks-qt/Commands/commandmanager.h \
    ../../xpiks-qt/Commands/Editing/deletekeywordstemplate.h \
    ../../xpiks-qt/Commands/Editing/editartworkstemplate.h \
    ../../xpiks-qt/Commands/Editing/expandpresettemplate.h \
    ../../xpiks-qt/Commands/Editing/findandreplacetemplate.h \
    ../../xpiks-qt/Commands/Editing/keywordedittemplate.h \
    ../../xpiks-qt/Commands/Editing/modifyartworkscommand.h \
    ../../xpiks-qt/Commands/Editing/readmetadatatemplate.h \
    ../../xpiks-qt/Commands/Files/addfilescommand.h \
    ../../xpiks-qt/Commands/Files/addtorecenttemplate.h \
    ../../xpiks-qt/Commands/Files/removedirectorycommand.h \
    ../../xpiks-qt/Commands/Files/removefilescommandbase.h \
    ../../xpiks-qt/Commands/Files/removeselectedfilescommand.h \
    ../../xpiks-qt/Commands/Services/autoimportmetadatacommand.h \
    ../../xpiks-qt/Commands/Services/cleanuplegacybackupscommand.h \
    ../../xpiks-qt/Commands/Services/generatethumbnailstemplate.h \
    ../../xpiks-qt/Commands/Services/savebackupstemplate.h \
    ../../xpiks-qt/Commands/Services/savesessioncommand.h \
    ../../xpiks-qt/Common/abstractlistmodel.h \
    ../../xpiks-qt/Common/defines.h \
    ../../xpiks-qt/Common/delayedactionentity.h \
    ../../xpiks-qt/Common/flags.h \
    ../../xpiks-qt/Common/iflagsprovider.h \
    ../../xpiks-qt/Common/isystemenvironment.h \
    ../../xpiks-qt/Common/itemprocessingworker.h \
    ../../xpiks-qt/Common/logging.h \
    ../../xpiks-qt/Common/messages.h \
    ../../xpiks-qt/Common/readerwriterqueue.h \
    ../../xpiks-qt/Common/statefulentity.h \
    ../../xpiks-qt/Common/systemenvironment.h \
    ../../xpiks-qt/Common/types.h \
    ../../xpiks-qt/Common/version.h \
    ../../xpiks-qt/Common/wordanalysisresult.h \
    ../../xpiks-qt/Connectivity/configrequest.h \
    ../../xpiks-qt/Connectivity/curlinithelper.h \
    ../../xpiks-qt/Connectivity/ftphelpers.h \
    ../../xpiks-qt/Connectivity/iconnectivityrequest.h \
    ../../xpiks-qt/Connectivity/iconnectivityresponse.h \
    ../../xpiks-qt/Connectivity/iftpcoordinator.h \
    ../../xpiks-qt/Connectivity/irequestsservice.h \
    ../../xpiks-qt/Connectivity/requestsservice.h \
    ../../xpiks-qt/Connectivity/requestsworker.h \
    ../../xpiks-qt/Connectivity/simpleapirequest.h \
    ../../xpiks-qt/Connectivity/simplecurldownloader.h \
    ../../xpiks-qt/Connectivity/simplecurlrequest.h \
    ../../xpiks-qt/Connectivity/switcherconfig.h \
    ../../xpiks-qt/Connectivity/telemetryservice.h \
    ../../xpiks-qt/Connectivity/telemetryworker.h \
    ../../xpiks-qt/Encryption/aes-qt.h \
    ../../xpiks-qt/Encryption/isecretsstorage.h \
    ../../xpiks-qt/Encryption/obfuscation.h \
    ../../xpiks-qt/Encryption/secretpair.h \
    ../../xpiks-qt/Encryption/secretsmanager.h \
    ../../xpiks-qt/Filesystem/directoriescollection.h \
    ../../xpiks-qt/Filesystem/filescollection.h \
    ../../xpiks-qt/Filesystem/filesdirectoriescollection.h \
    ../../xpiks-qt/Filesystem/ifilescollection.h \
    ../../xpiks-qt/Helpers/artworkshelpers.h \
    ../../xpiks-qt/Helpers/asynccoordinator.h \
    ../../xpiks-qt/Helpers/comparevaluesjson.h \
    ../../xpiks-qt/Helpers/constants.h \
    ../../xpiks-qt/Helpers/cpphelpers.h \
    ../../xpiks-qt/Helpers/filehelpers.h \
    ../../xpiks-qt/Helpers/filterhelpers.h \
    ../../xpiks-qt/Helpers/indiceshelper.h \
    ../../xpiks-qt/Helpers/indicesranges.h \
    ../../xpiks-qt/Helpers/jsonhelper.h \
    ../../xpiks-qt/Helpers/jsonobjectmap.h \
    ../../xpiks-qt/Helpers/keywordshelpers.h \
    ../../xpiks-qt/Helpers/localconfig.h \
    ../../xpiks-qt/Helpers/logger.h \
    ../../xpiks-qt/Helpers/loggingworker.h \
    ../../xpiks-qt/Helpers/remoteconfig.h \
    ../../xpiks-qt/Helpers/stringhelper.h \
    ../../xpiks-qt/Helpers/threadhelpers.h \
    ../../xpiks-qt/Helpers/ziphelper.h \
    ../../xpiks-qt/KeywordsPresets/groupmodel.h \
    ../../xpiks-qt/KeywordsPresets/ipresetsmanager.h \
    ../../xpiks-qt/KeywordsPresets/presetgroupsmodel.h \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodel.h \
    ../../xpiks-qt/KeywordsPresets/presetkeywordsmodelconfig.h \
    ../../xpiks-qt/KeywordsPresets/presetmodel.h \
    ../../xpiks-qt/MetadataIO/cachedartwork.h \
    ../../xpiks-qt/MetadataIO/csvexportmodel.h \
    ../../xpiks-qt/MetadataIO/csvexportplansmodel.h \
    ../../xpiks-qt/MetadataIO/csvexportproperties.h \
    ../../xpiks-qt/MetadataIO/csvexportworker.h \
    ../../xpiks-qt/MetadataIO/metadatacache.h \
    ../../xpiks-qt/MetadataIO/metadataiocoordinator.h \
    ../../xpiks-qt/MetadataIO/metadataioservice.h \
    ../../xpiks-qt/MetadataIO/metadataiotask.h \
    ../../xpiks-qt/MetadataIO/metadataioworker.h \
    ../../xpiks-qt/MetadataIO/metadatareadinghub.h \
    ../../xpiks-qt/MetadataIO/originalmetadata.h \
    ../../xpiks-qt/Microstocks/fotoliaapiclient.h \
    ../../xpiks-qt/Microstocks/gettyapiclient.h \
    ../../xpiks-qt/Microstocks/imicrostockapiclient.h \
    ../../xpiks-qt/Microstocks/imicrostockapiclients.h \
    ../../xpiks-qt/Microstocks/imicrostockservice.h \
    ../../xpiks-qt/Microstocks/imicrostockservices.h \
    ../../xpiks-qt/Microstocks/microstockapiclients.h \
    ../../xpiks-qt/Microstocks/microstockenums.h \
    ../../xpiks-qt/Microstocks/microstockservice.h \
    ../../xpiks-qt/Microstocks/searchquery.h \
    ../../xpiks-qt/Microstocks/shutterstockapiclient.h \
    ../../xpiks-qt/Microstocks/stockftpoptions.h \
    ../../xpiks-qt/Microstocks/stocksftplistmodel.h \
    ../../xpiks-qt/Models/Artworks/artworkslistmodel.h \
    ../../xpiks-qt/Models/Artworks/artworkslistoperations.h \
    ../../xpiks-qt/Models/Artworks/artworksrepository.h \
    ../../xpiks-qt/Models/Artworks/artworksviewmodel.h \
    ../../xpiks-qt/Models/Artworks/filteredartworkslistmodel.h \
    ../../xpiks-qt/Models/Connectivity/abstractconfigupdatermodel.h \
    ../../xpiks-qt/Models/Connectivity/artworksuploader.h \
    ../../xpiks-qt/Models/Connectivity/proxysettings.h \
    ../../xpiks-qt/Models/Connectivity/uploadinfo.h \
    ../../xpiks-qt/Models/Connectivity/uploadinforepository.h \
    ../../xpiks-qt/Models/Connectivity/ziparchiver.h \
    ../../xpiks-qt/Models/Editing/artworkproxybase.h \
    ../../xpiks-qt/Models/Editing/artworkproxymodel.h \
    ../../xpiks-qt/Models/Editing/combinedartworksmodel.h \
    ../../xpiks-qt/Models/Editing/currenteditableartwork.h \
    ../../xpiks-qt/Models/Editing/currenteditablemodel.h \
    ../../xpiks-qt/Models/Editing/currenteditableproxyartwork.h \
    ../../xpiks-qt/Models/Editing/deletekeywordsviewmodel.h \
    ../../xpiks-qt/Models/Editing/findandreplacemodel.h \
    ../../xpiks-qt/Models/Editing/icurrenteditable.h \
    ../../xpiks-qt/Models/Editing/previewartworkelement.h \
    ../../xpiks-qt/Models/Editing/quickbuffer.h \
    ../../xpiks-qt/Models/Editing/quickbuffermessage.h \
    ../../xpiks-qt/Models/Session/recentdirectoriesmodel.h \
    ../../xpiks-qt/Models/Session/recentfilesmodel.h \
    ../../xpiks-qt/Models/Session/recentitemsmodel.h \
    ../../xpiks-qt/Models/Session/sessionmanager.h \
    ../../xpiks-qt/Models/exportinfo.h \
    ../../xpiks-qt/Models/keyvaluelist.h \
    ../../xpiks-qt/Models/languagesmodel.h \
    ../../xpiks-qt/Models/logsmodel.h \
    ../../xpiks-qt/Models/settingsmodel.h \
    ../../xpiks-qt/Models/switchermodel.h \
    ../../xpiks-qt/Plugins/ipluginaction.h \
    ../../xpiks-qt/Plugins/iuiprovider.h \
    ../../xpiks-qt/Plugins/pluginactionsmodel.h \
    ../../xpiks-qt/Plugins/plugindatabasemanager.h \
    ../../xpiks-qt/Plugins/pluginenvironment.h \
    ../../xpiks-qt/Plugins/pluginmanager.h \
    ../../xpiks-qt/Plugins/pluginwrapper.h \
    ../../xpiks-qt/Plugins/sandboxeddependencies.h \
    ../../xpiks-qt/Plugins/uiprovider.h \
    ../../xpiks-qt/Plugins/xpiksplugininterface.h \
    ../../xpiks-qt/Services/artworksupdatehub.h \
    ../../xpiks-qt/Services/artworkupdaterequest.h \
    ../../xpiks-qt/Services/iartworksupdater.h \
    ../../xpiks-qt/Services/AutoComplete/autocompletemodel.h \
    ../../xpiks-qt/Services/AutoComplete/autocompleteservice.h \
    ../../xpiks-qt/Services/AutoComplete/autocompleteworker.h \
    ../../xpiks-qt/Services/AutoComplete/completionenginebase.h \
    ../../xpiks-qt/Services/AutoComplete/completionitem.h \
    ../../xpiks-qt/Services/AutoComplete/completionquery.h \
    ../../xpiks-qt/Services/AutoComplete/icompletionsource.h \
    ../../xpiks-qt/Services/AutoComplete/keywordsautocompletemodel.h \
    ../../xpiks-qt/Services/AutoComplete/libfacecompletionengine.h \
    ../../xpiks-qt/Services/AutoComplete/presetscompletionengine.h \
    ../../xpiks-qt/Services/AutoComplete/stringsautocompletemodel.h \
    ../../xpiks-qt/Services/SpellCheck/duplicateshighlighter.h \
    ../../xpiks-qt/Services/SpellCheck/duplicatesreviewmodel.h \
    ../../xpiks-qt/Services/SpellCheck/ispellcheckable.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckerrorshighlighter.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckitem.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckinfo.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckservice.h \
    ../../xpiks-qt/Services/SpellCheck/spellchecksuggestionmodel.h \
    ../../xpiks-qt/Services/SpellCheck/spellcheckworker.h \
    ../../xpiks-qt/Services/SpellCheck/spellsuggestionsitem.h \
    ../../xpiks-qt/Services/SpellCheck/userdicteditmodel.h \
    ../../xpiks-qt/Services/SpellCheck/userdictionary.h \
    ../../xpiks-qt/Services/Translation/translationmanager.h \
    ../../xpiks-qt/Services/Translation/translationquery.h \
    ../../xpiks-qt/Services/Translation/translationservice.h \
    ../../xpiks-qt/Services/Translation/translationworker.h \
    ../../xpiks-qt/Services/Warnings/iwarningsitem.h \
    ../../xpiks-qt/Services/Warnings/iwarningssettings.h \
    ../../xpiks-qt/Services/Warnings/warningscheckingworker.h \
    ../../xpiks-qt/Services/Warnings/warningsitem.h \
    ../../xpiks-qt/Services/Warnings/warningsmodel.h \
    ../../xpiks-qt/Services/Warnings/warningsservice.h \
    ../../xpiks-qt/Services/Warnings/warningssettingsmodel.h \
    ../../xpiks-qt/Storage/database.h \
    ../../xpiks-qt/Storage/databasemanager.h \
    ../../xpiks-qt/Storage/idatabase.h \
    ../../xpiks-qt/Storage/idatabasemanager.h \
    ../../xpiks-qt/Storage/memorytable.h \
    ../../xpiks-qt/Storage/writeaheadlog.h \
    ../../xpiks-qt/Suggestion/fotoliasuggestionengine.h \
    ../../xpiks-qt/Suggestion/gettysuggestionengine.h \
    ../../xpiks-qt/Suggestion/isuggestionengine.h \
    ../../xpiks-qt/Suggestion/isuggestionsrepository.h \
    ../../xpiks-qt/Suggestion/keywordssuggestor.h \
    ../../xpiks-qt/Suggestion/locallibraryquery.h \
    ../../xpiks-qt/Suggestion/locallibraryqueryengine.h \
    ../../xpiks-qt/Suggestion/microstocksuggestionengine.h \
    ../../xpiks-qt/Suggestion/shutterstocksuggestionengine.h \
    ../../xpiks-qt/Suggestion/suggestionartwork.h \
    ../../xpiks-qt/Suggestion/suggestionresultsresponse.h \
    ../../xpiks-qt/UndoRedo/artworkmetadatabackup.h \
    ../../xpiks-qt/UndoRedo/iundoredomanager.h \
    ../../xpiks-qt/UndoRedo/undoredomanager.h \
    ../../xpiks-qt/xpiksapp.h \
    ../../xpiks-qt/QMLExtensions/uicommanddispatcher.h \
    ../../xpiks-qt/Commands/UI/generalcommands.h \
    ../../xpiks-qt/Commands/UI/selectedartworkscommands.h \
    ../../xpiks-qt/Commands/UI/singleeditablecommands.h \
    ../../xpiks-qt/Commands/UI/sourcetargetcommand.h \
    ../../xpiks-qt/Services/Maintenance/imaintenanceitem.h \
    ../../xpiks-qt/Services/Maintenance/initializedictionariesjobitem.h \
    ../../xpiks-qt/Services/Maintenance/launchexiftooljobitem.h \
    ../../xpiks-qt/Services/Maintenance/logscleanupjobitem.h \
    ../../xpiks-qt/Services/Maintenance/maintenanceservice.h \
    ../../xpiks-qt/Services/Maintenance/maintenanceworker.h \
    ../../xpiks-qt/Services/Maintenance/savesessionjobitem.h \
    ../../xpiks-qt/Services/Maintenance/updatebundlecleanupjobitem.h \
    ../../xpiks-qt/Services/Maintenance/updatescleanupjobitem.h \
    ../../xpiks-qt/Services/Maintenance/xpkscleanupjob.h \
    ../../xpiks-qt/Connectivity/updatescheckerworker.h \
    ../../xpiks-qt/Connectivity/updateservice.h \
    ../../xpiks-qt/Connectivity/uploadwatcher.h \
    ../../xpiks-qt/QMLExtensions/colorsmodel.h \
    ../../xpiks-qt/QMLExtensions/cachedimage.h \
    ../../xpiks-qt/QMLExtensions/cachedvideo.h \
    ../../xpiks-qt/QMLExtensions/dbcacheindex.h \
    ../../xpiks-qt/QMLExtensions/dbimagecacheindex.h \
    ../../xpiks-qt/QMLExtensions/dbvideocacheindex.h \
    ../../xpiks-qt/QMLExtensions/imagecacherequest.h \
    ../../xpiks-qt/QMLExtensions/imagecachingservice.h \
    ../../xpiks-qt/QMLExtensions/imagecachingworker.h \
    ../../xpiks-qt/QMLExtensions/videocacherequest.h \
    ../../xpiks-qt/QMLExtensions/videocachingservice.h \
    ../../xpiks-qt/QMLExtensions/videocachingworker.h \
    ../../xpiks-qt/Commands/Files/removefilescommand.h \
    ../../xpiks-qt/Models/uimanager.h \
    ../../xpiks-qt/Commands/artworksupdatetemplate.h \
    ../../xpiks-qt/QMLExtensions/tabsmodel.h \
    ../../xpiks-qt/Helpers/updatehelpers.h \
    ../../xpiks-qt/Connectivity/testconnection.h \
    ../../xpiks-qt/Helpers/loghighlighter.h \
    ../../xpiks-qt/Helpers/helpersqmlwrapper.h \
    ../../xpiks-qt/Helpers/hashhelpers.h \
    ../../xpiks-qt/Services/SpellCheck/imetadataduplicates.h \
    ../../xpiks-qt/Services/SpellCheck/ispellcheckservice.h \
    ../../xpiks-qt/Services/SpellCheck/ispellsuggestionstarget.h \
    ../../xpiks-qt/Services/SpellCheck/metadataduplicates.h \
    ../../xpiks-qt/Services/SpellCheck/spellsuggestionstarget.h \
    ../../xpiks-qt/Services/artworkseditinghub.h \
    xpiksuitestsapp.h \
    ../../xpiks-qt/QMLExtensions/triangleelement.h \
    ../../xpiks-qt/Helpers/clipboardhelper.h \
    ../xpiks-tests-integration/signalwaiter.h \
    ../xpiks-tests-integration/testshelpers.h \
    ../xpiks-tests-core/Mocks/filescollectionmock.h \
    fakeinitartworkstemplate.h \
    ../../xpiks-qt/Helpers/metadatahighlighter.h \
    ../../xpiks-qt/QMLExtensions/folderelement.h \
    ../../xpiks-qt/QMLExtensions/uicommandid.h \
    ../../xpiks-qt/Helpers/uihelpers.h \
    ../../xpiks-qt/QMLExtensions/uiaction.h \
    ../../xpiks-qt/QMLExtensions/uicommandlistener.h \
    ../../xpiks-qt/Commands/Base/actionmodelcommand.h \
    ../../xpiks-qt/Commands/Editing/clearactionmodeltemplate.h \
    ../../xpiks-qt/Commands/Base/compositecommand.h \
    ../../xpiks-qt/Commands/UI/currenteditablecommands.h \
    ../../xpiks-qt/QMLExtensions/iuicommandmiddleware.h \
    ../../xpiks-qt/QMLExtensions/proxyindexmiddlware.h \
    ../../xpiks-qt/Suggestion/suggestedkeywords.h \
    ../../xpiks-qt/Connectivity/curlhelpers.h \
    ../xpiks-tests-integration/integrationtestsenvironment.h

RESOURCES += \
    xpiks-tests-ui.qrc
