TEMPLATE = app

macx { QMAKE_MAC_SDK = macosx }

QT += qml quick widgets concurrent svg
CONFIG += qtquickcompiler
CONFIG += c++14
TARGET = Xpiks

!win32 {
    QMAKE_CXXFLAGS += -std=c++14
}
#CONFIG += force_debug_info

CONFIG(release, debug|release)  {
    CONFIG += separate_debug_info
}

VERSION = 1.5.2.2
QMAKE_TARGET_PRODUCT = Xpiks
QMAKE_TARGET_DESCRIPTION = "Cross-Platform Image Keywording Software"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2014-2018 Taras Kushnir"

SOURCES += main.cpp \
    ../../vendors/sqlite/sqlite3.c \
    ../../vendors/tiny-aes/aes.cpp \
    Artworks/artworkmetadata.cpp \
    Artworks/artworkssnapshot.cpp \
    Artworks/basickeywordsmodel.cpp \
    Artworks/basickeywordsmodelimpl.cpp \
    Artworks/basicmetadatamodel.cpp \
    Artworks/imageartwork.cpp \
    Artworks/videoartwork.cpp \
    Commands/artworksupdatetemplate.cpp \
    Commands/Base/actionmodelcommand.cpp \
    Commands/Base/compositecommand.cpp \
    Commands/commandmanager.cpp \
    Commands/Editing/clearactionmodeltemplate.cpp \
    Commands/Editing/deletekeywordstemplate.cpp \
    Commands/Editing/editartworkstemplate.cpp \
    Commands/Editing/expandpresettemplate.cpp \
    Commands/Editing/findandreplacetemplate.cpp \
    Commands/Editing/keywordedittemplate.cpp \
    Commands/Editing/readmetadatatemplate.cpp \
    Commands/Files/addfilescommand.cpp \
    Commands/Files/addtorecenttemplate.cpp \
    Commands/Files/removedirectorycommand.cpp \
    Commands/Files/removefilescommand.cpp \
    Commands/Files/removefilescommandbase.cpp \
    Commands/Files/removeselectedfilescommand.cpp \
    Commands/Services/autoimportmetadatacommand.cpp \
    Commands/Services/cleanuplegacybackupscommand.cpp \
    Commands/Services/generatethumbnailstemplate.cpp \
    Commands/Services/savebackupstemplate.cpp \
    Commands/Services/savesessioncommand.cpp \
    Commands/UI/currenteditablecommands.cpp \
    Commands/UI/generalcommands.cpp \
    Commands/UI/selectedartworkscommands.cpp \
    Commands/UI/singleeditablecommands.cpp \
    Common/logging.cpp \
    Common/statefulentity.cpp \
    Common/systemenvironment.cpp \
    Connectivity/configrequest.cpp \
    Connectivity/curlinithelper.cpp \
    Connectivity/ftphelpers.cpp \
    Connectivity/requestsservice.cpp \
    Connectivity/requestsworker.cpp \
    Connectivity/simplecurldownloader.cpp \
    Connectivity/simplecurlrequest.cpp \
    Connectivity/switcherconfig.cpp \
    Connectivity/telemetryservice.cpp \
    Connectivity/telemetryworker.cpp \
    Connectivity/testconnection.cpp \
    Connectivity/updatescheckerworker.cpp \
    Connectivity/updateservice.cpp \
    Connectivity/uploadwatcher.cpp \
    Encryption/aes-qt.cpp \
    Encryption/obfuscation.cpp \
    Encryption/secretsmanager.cpp \
    Filesystem/directoriescollection.cpp \
    Filesystem/filescollection.cpp \
    Filesystem/filesdirectoriescollection.cpp \
    Helpers/artworkshelpers.cpp \
    Helpers/asynccoordinator.cpp \
    Helpers/filehelpers.cpp \
    Helpers/filterhelpers.cpp \
    Helpers/globalimageprovider.cpp \
    Helpers/helpersqmlwrapper.cpp \
    Helpers/indiceshelper.cpp \
    Helpers/indicesranges.cpp \
    Helpers/jsonhelper.cpp \
    Helpers/keywordshelpers.cpp \
    Helpers/localconfig.cpp \
    Helpers/logger.cpp \
    Helpers/loggingworker.cpp \
    Helpers/loghighlighter.cpp \
    Helpers/metadatahighlighter.cpp \
    Helpers/remoteconfig.cpp \
    Helpers/runguard.cpp \
    Helpers/stringhelper.cpp \
    Helpers/threadhelpers.cpp \
    Helpers/uihelpers.cpp \
    Helpers/updatehelpers.cpp \
    Helpers/ziphelper.cpp \
    KeywordsPresets/presetgroupsmodel.cpp \
    KeywordsPresets/presetkeywordsmodel.cpp \
    KeywordsPresets/presetkeywordsmodelconfig.cpp \
    MetadataIO/cachedartwork.cpp \
    MetadataIO/csvexportmodel.cpp \
    MetadataIO/csvexportplansmodel.cpp \
    MetadataIO/csvexportproperties.cpp \
    MetadataIO/csvexportworker.cpp \
    MetadataIO/metadatacache.cpp \
    MetadataIO/metadataiocoordinator.cpp \
    MetadataIO/metadataioservice.cpp \
    MetadataIO/metadataioworker.cpp \
    MetadataIO/metadatareadinghub.cpp \
    Microstocks/fotoliaapiclient.cpp \
    Microstocks/gettyapiclient.cpp \
    Microstocks/microstockservice.cpp \
    Microstocks/shutterstockapiclient.cpp \
    Microstocks/stocksftplistmodel.cpp \
    Models/Artworks/artworkslistmodel.cpp \
    Models/Artworks/artworksrepository.cpp \
    Models/Artworks/artworksviewmodel.cpp \
    Models/Artworks/filteredartworkslistmodel.cpp \
    Models/Connectivity/abstractconfigupdatermodel.cpp \
    Models/Connectivity/artworksuploader.cpp \
    Models/Connectivity/proxysettings.cpp \
    Models/Connectivity/uploadinforepository.cpp \
    Models/Connectivity/ziparchiver.cpp \
    Models/Editing/artworkproxybase.cpp \
    Models/Editing/artworkproxymodel.cpp \
    Models/Editing/combinedartworksmodel.cpp \
    Models/Editing/currenteditableartwork.cpp \
    Models/Editing/currenteditablemodel.cpp \
    Models/Editing/currenteditableproxyartwork.cpp \
    Models/Editing/deletekeywordsviewmodel.cpp \
    Models/Editing/findandreplacemodel.cpp \
    Models/Editing/quickbuffer.cpp \
    Models/keyvaluelist.cpp \
    Models/languagesmodel.cpp \
    Models/logsmodel.cpp \
    Models/Session/recentdirectoriesmodel.cpp \
    Models/Session/recentfilesmodel.cpp \
    Models/Session/recentitemsmodel.cpp \
    Models/Session/sessionmanager.cpp \
    Models/settingsmodel.cpp \
    Models/switchermodel.cpp \
    Models/uimanager.cpp \
    Plugins/pluginactionsmodel.cpp \
    Plugins/plugindatabasemanager.cpp \
    Plugins/pluginenvironment.cpp \
    Plugins/pluginmanager.cpp \
    Plugins/pluginwrapper.cpp \
    Plugins/sandboxeddependencies.cpp \
    Plugins/uiprovider.cpp \
    QMLExtensions/cachedimage.cpp \
    QMLExtensions/cachedvideo.cpp \
    QMLExtensions/cachingimageprovider.cpp \
    QMLExtensions/colorsmodel.cpp \
    QMLExtensions/dbimagecacheindex.cpp \
    QMLExtensions/dbvideocacheindex.cpp \
    QMLExtensions/folderelement.cpp \
    QMLExtensions/imagecachingservice.cpp \
    QMLExtensions/imagecachingworker.cpp \
    QMLExtensions/proxyindexmiddlware.cpp \
    QMLExtensions/tabsmodel.cpp \
    QMLExtensions/triangleelement.cpp \
    QMLExtensions/uicommanddispatcher.cpp \
    QMLExtensions/uicommandlistener.cpp \
    QMLExtensions/videocachingservice.cpp \
    QMLExtensions/videocachingworker.cpp \
    Services/artworkseditinghub.cpp \
    Services/artworksupdatehub.cpp \
    Services/AutoComplete/autocompletemodel.cpp \
    Services/AutoComplete/autocompleteservice.cpp \
    Services/AutoComplete/autocompleteworker.cpp \
    Services/AutoComplete/keywordsautocompletemodel.cpp \
    Services/AutoComplete/libfacecompletionengine.cpp \
    Services/AutoComplete/presetscompletionengine.cpp \
    Services/AutoComplete/stringsautocompletemodel.cpp \
    Services/Maintenance/initializedictionariesjobitem.cpp \
    Services/Maintenance/launchexiftooljobitem.cpp \
    Services/Maintenance/logscleanupjobitem.cpp \
    Services/Maintenance/maintenanceservice.cpp \
    Services/Maintenance/maintenanceworker.cpp \
    Services/Maintenance/savesessionjobitem.cpp \
    Services/Maintenance/updatebundlecleanupjobitem.cpp \
    Services/Maintenance/updatescleanupjobitem.cpp \
    Services/Maintenance/xpkscleanupjob.cpp \
    Services/SpellCheck/duplicateshighlighter.cpp \
    Services/SpellCheck/duplicatesreviewmodel.cpp \
    Services/SpellCheck/metadataduplicates.cpp \
    Services/SpellCheck/spellcheckerrorshighlighter.cpp \
    Services/SpellCheck/spellcheckinfo.cpp \
    Services/SpellCheck/spellcheckitem.cpp \
    Services/SpellCheck/spellcheckservice.cpp \
    Services/SpellCheck/spellchecksuggestionmodel.cpp \
    Services/SpellCheck/spellcheckworker.cpp \
    Services/SpellCheck/spellsuggestionsitem.cpp \
    Services/SpellCheck/spellsuggestionstarget.cpp \
    Services/SpellCheck/userdicteditmodel.cpp \
    Services/SpellCheck/userdictionary.cpp \
    Services/Translation/translationmanager.cpp \
    Services/Translation/translationquery.cpp \
    Services/Translation/translationservice.cpp \
    Services/Translation/translationworker.cpp \
    Services/Warnings/warningscheckingworker.cpp \
    Services/Warnings/warningsitem.cpp \
    Services/Warnings/warningsmodel.cpp \
    Services/Warnings/warningsservice.cpp \
    Services/Warnings/warningssettingsmodel.cpp \
    Storage/database.cpp \
    Storage/databasemanager.cpp \
    Storage/memorytable.cpp \
    Suggestion/fotoliasuggestionengine.cpp \
    Suggestion/gettysuggestionengine.cpp \
    Suggestion/keywordssuggestor.cpp \
    Suggestion/locallibraryqueryengine.cpp \
    Suggestion/shutterstocksuggestionengine.cpp \
    Suggestion/suggestionresultsresponse.cpp \
    UndoRedo/artworkmetadatabackup.cpp \
    UndoRedo/undoredomanager.cpp \
    xpiksapp.cpp

RESOURCES += qml.qrc

DEFINES += QT_NO_CAST_TO_ASCII \
           QT_RESTRICTED_CAST_FROM_ASCII \
           QT_NO_CAST_FROM_BYTEARRAY
DEFINES += HUNSPELL_STATIC
DEFINES += QT_MESSAGELOGCONTEXT

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

CONFIG(debug, debug|release)  {
    ICON = Graphics/xpiks-qt-dev.icns
    RC_ICONS = Graphics/xpiks-qt-dev.ico
} else {
    ICON = Graphics/xpiks-qt.icns
    RC_ICONS = Graphics/xpiks-qt.ico
}

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    Artworks/artworkelement.h \
    Artworks/artworkmetadata.h \
    Artworks/artworkssnapshot.h \
    Artworks/basickeywordsmodel.h \
    Artworks/basickeywordsmodelimpl.h \
    Artworks/basicmetadatamodel.h \
    Artworks/basicmodelsource.h \
    Artworks/iartworkmetadata.h \
    Artworks/ibasicmodelsource.h \
    Artworks/imageartwork.h \
    Artworks/iselectedartworkssource.h \
    Artworks/iselectedindicessource.h \
    Artworks/keyword.h \
    Artworks/videoartwork.h \
    Commands/artworksupdatetemplate.h \
    Commands/Base/actionmodelcommand.h \
    Commands/Base/callbackcommand.h \
    Commands/Base/commanduiwrapper.h \
    Commands/Base/compositecommand.h \
    Commands/Base/compositecommandtemplate.h \
    Commands/Base/emptycommand.h \
    Commands/Base/icommand.h \
    Commands/Base/icommandmanager.h \
    Commands/Base/icommandtemplate.h \
    Commands/Base/iuicommandtemplate.h \
    Commands/Base/templatedcommand.h \
    Commands/Base/templateduicommand.h \
    Commands/commandmanager.h \
    Commands/Editing/clearactionmodeltemplate.h \
    Commands/Editing/deletekeywordstemplate.h \
    Commands/Editing/editartworkstemplate.h \
    Commands/Editing/expandpresettemplate.h \
    Commands/Editing/findandreplacetemplate.h \
    Commands/Editing/keywordedittemplate.h \
    Commands/Editing/modifyartworkscommand.h \
    Commands/Editing/readmetadatatemplate.h \
    Commands/Files/addfilescommand.h \
    Commands/Files/addtorecenttemplate.h \
    Commands/Files/removedirectorycommand.h \
    Commands/Files/removefilescommand.h \
    Commands/Files/removefilescommandbase.h \
    Commands/Files/removeselectedfilescommand.h \
    Commands/Services/autoimportmetadatacommand.h \
    Commands/Services/cleanuplegacybackupscommand.h \
    Commands/Services/generatethumbnailstemplate.h \
    Commands/Services/savebackupstemplate.h \
    Commands/Services/savesessioncommand.h \
    Commands/UI/currenteditablecommands.h \
    Commands/UI/generalcommands.h \
    Commands/UI/selectedartworkscommands.h \
    Commands/UI/singleeditablecommands.h \
    Commands/UI/sourcetargetcommand.h \
    Common/abstractlistmodel.h \
    Common/defines.h \
    Common/delayedactionentity.h \
    Common/flags.h \
    Common/iflagsprovider.h \
    Common/isystemenvironment.h \
    Common/itemprocessingworker.h \
    Common/logging.h \
    Common/lrucache.h \
    Common/messages.h \
    Common/readerwriterqueue.h \
    Common/statefulentity.h \
    Common/systemenvironment.h \
    Common/types.h \
    Common/version.h \
    Common/wordanalysisresult.h \
    Connectivity/analyticsuserevent.h \
    Connectivity/apimanager.h \
    Connectivity/configrequest.h \
    Connectivity/curlinithelper.h \
    Connectivity/ftphelpers.h \
    Connectivity/iconnectivityrequest.h \
    Connectivity/iconnectivityresponse.h \
    Connectivity/iftpcoordinator.h \
    Connectivity/irequestsservice.h \
    Connectivity/requestsservice.h \
    Connectivity/requestsworker.h \
    Connectivity/simpleapirequest.h \
    Connectivity/simplecurldownloader.h \
    Connectivity/simplecurlrequest.h \
    Connectivity/switcherconfig.h \
    Connectivity/telemetryservice.h \
    Connectivity/telemetryworker.h \
    Connectivity/testconnection.h \
    Connectivity/updatescheckerworker.h \
    Connectivity/updateservice.h \
    Connectivity/uploadwatcher.h \
    Encryption/aes-qt.h \
    Encryption/isecretsstorage.h \
    Encryption/obfuscation.h \
    Encryption/secretpair.h \
    Encryption/secretsmanager.h \
    Filesystem/directoriescollection.h \
    Filesystem/filescollection.h \
    Filesystem/filesdirectoriescollection.h \
    Filesystem/ifilescollection.h \
    Helpers/artworkshelpers.h \
    Helpers/asynccoordinator.h \
    Helpers/clipboardhelper.h \
    Helpers/comparevaluesjson.h \
    Helpers/constants.h \
    Helpers/cpphelpers.h \
    Helpers/filehelpers.h \
    Helpers/filterhelpers.h \
    Helpers/globalimageprovider.h \
    Helpers/hashhelpers.h \
    Helpers/helpersqmlwrapper.h \
    Helpers/indiceshelper.h \
    Helpers/indicesranges.h \
    Helpers/jsonhelper.h \
    Helpers/jsonobjectmap.h \
    Helpers/keywordshelpers.h \
    Helpers/localconfig.h \
    Helpers/logger.h \
    Helpers/loggingworker.h \
    Helpers/loghighlighter.h \
    Helpers/metadatahighlighter.h \
    Helpers/remoteconfig.h \
    Helpers/runguard.h \
    Helpers/stringhelper.h \
    Helpers/threadhelpers.h \
    Helpers/uihelpers.h \
    Helpers/updatehelpers.h \
    Helpers/ziphelper.h \
    KeywordsPresets/groupmodel.h \
    KeywordsPresets/ipresetsmanager.h \
    KeywordsPresets/presetgroupsmodel.h \
    KeywordsPresets/presetkeywordsmodel.h \
    KeywordsPresets/presetkeywordsmodelconfig.h \
    KeywordsPresets/presetmodel.h \
    MetadataIO/cachedartwork.h \
    MetadataIO/csvexportmodel.h \
    MetadataIO/csvexportplansmodel.h \
    MetadataIO/csvexportproperties.h \
    MetadataIO/csvexportworker.h \
    MetadataIO/metadatacache.h \
    MetadataIO/metadataiocoordinator.h \
    MetadataIO/metadataioservice.h \
    MetadataIO/metadataiotask.h \
    MetadataIO/metadataioworker.h \
    MetadataIO/metadatareadinghub.h \
    MetadataIO/originalmetadata.h \
    Microstocks/fotoliaapiclient.h \
    Microstocks/gettyapiclient.h \
    Microstocks/imicrostockapiclient.h \
    Microstocks/imicrostockapiclients.h \
    Microstocks/imicrostockservice.h \
    Microstocks/imicrostockservices.h \
    Microstocks/microstockapiclients.h \
    Microstocks/microstockenums.h \
    Microstocks/microstockservice.h \
    Microstocks/searchquery.h \
    Microstocks/shutterstockapiclient.h \
    Microstocks/stockftpoptions.h \
    Microstocks/stocksftplistmodel.h \
    Models/Artworks/artworkslistmodel.h \
    Models/Artworks/artworkslistoperations.h \
    Models/Artworks/artworksrepository.h \
    Models/Artworks/artworksviewmodel.h \
    Models/Artworks/filteredartworkslistmodel.h \
    Models/Connectivity/abstractconfigupdatermodel.h \
    Models/Connectivity/artworksuploader.h \
    Models/Connectivity/proxysettings.h \
    Models/Connectivity/uploadinfo.h \
    Models/Connectivity/uploadinforepository.h \
    Models/Connectivity/ziparchiver.h \
    Models/Editing/artworkproxybase.h \
    Models/Editing/artworkproxymodel.h \
    Models/Editing/combinedartworksmodel.h \
    Models/Editing/currenteditableartwork.h \
    Models/Editing/currenteditablemodel.h \
    Models/Editing/currenteditableproxyartwork.h \
    Models/Editing/deletekeywordsviewmodel.h \
    Models/Editing/findandreplacemodel.h \
    Models/Editing/icurrenteditable.h \
    Models/Editing/previewartworkelement.h \
    Models/Editing/quickbuffer.h \
    Models/Editing/quickbuffermessage.h \
    Models/exportinfo.h \
    Models/iactionmodel.h \
    Models/keyvaluelist.h \
    Models/languagesmodel.h \
    Models/logsmodel.h \
    Models/Session/recentdirectoriesmodel.h \
    Models/Session/recentfilesmodel.h \
    Models/Session/recentitemsmodel.h \
    Models/Session/sessionmanager.h \
    Models/settingsmodel.h \
    Models/switchermodel.h \
    Models/uimanager.h \
    Plugins/ipluginaction.h \
    Plugins/iuiprovider.h \
    Plugins/pluginactionsmodel.h \
    Plugins/plugindatabasemanager.h \
    Plugins/pluginenvironment.h \
    Plugins/pluginmanager.h \
    Plugins/pluginwrapper.h \
    Plugins/sandboxeddependencies.h \
    Plugins/uiprovider.h \
    Plugins/xpiksplugininterface.h \
    QMLExtensions/cachedimage.h \
    QMLExtensions/cachedvideo.h \
    QMLExtensions/cachingimageprovider.h \
    QMLExtensions/colorsmodel.h \
    QMLExtensions/dbcacheindex.h \
    QMLExtensions/dbimagecacheindex.h \
    QMLExtensions/dbvideocacheindex.h \
    QMLExtensions/folderelement.h \
    QMLExtensions/imagecacherequest.h \
    QMLExtensions/imagecachingservice.h \
    QMLExtensions/imagecachingworker.h \
    QMLExtensions/iuicommandmiddleware.h \
    QMLExtensions/previewstorage.h \
    QMLExtensions/proxyindexmiddlware.h \
    QMLExtensions/tabsmodel.h \
    QMLExtensions/triangleelement.h \
    QMLExtensions/uiaction.h \
    QMLExtensions/uicommanddispatcher.h \
    QMLExtensions/uicommandid.h \
    QMLExtensions/uicommandlistener.h \
    QMLExtensions/videocacherequest.h \
    QMLExtensions/videocachingservice.h \
    QMLExtensions/videocachingworker.h \
    Services/artworkseditinghub.h \
    Services/artworksupdatehub.h \
    Services/artworkupdaterequest.h \
    Services/AutoComplete/autocompletemodel.h \
    Services/AutoComplete/autocompleteservice.h \
    Services/AutoComplete/autocompleteworker.h \
    Services/AutoComplete/completionenginebase.h \
    Services/AutoComplete/completionitem.h \
    Services/AutoComplete/completionquery.h \
    Services/AutoComplete/icompletionsource.h \
    Services/AutoComplete/keywordsautocompletemodel.h \
    Services/AutoComplete/libfacecompletionengine.h \
    Services/AutoComplete/presetscompletionengine.h \
    Services/AutoComplete/stringsautocompletemodel.h \
    Services/iartworksupdater.h \
    Services/Maintenance/imaintenanceitem.h \
    Services/Maintenance/initializedictionariesjobitem.h \
    Services/Maintenance/launchexiftooljobitem.h \
    Services/Maintenance/logscleanupjobitem.h \
    Services/Maintenance/maintenanceservice.h \
    Services/Maintenance/maintenanceworker.h \
    Services/Maintenance/savesessionjobitem.h \
    Services/Maintenance/updatebundlecleanupjobitem.h \
    Services/Maintenance/updatescleanupjobitem.h \
    Services/Maintenance/xpkscleanupjob.h \
    Services/SpellCheck/duplicateshighlighter.h \
    Services/SpellCheck/duplicatesreviewmodel.h \
    Services/SpellCheck/imetadataduplicates.h \
    Services/SpellCheck/ispellcheckable.h \
    Services/SpellCheck/ispellcheckservice.h \
    Services/SpellCheck/ispellsuggestionstarget.h \
    Services/SpellCheck/metadataduplicates.h \
    Services/SpellCheck/spellcheckerrorshighlighter.h \
    Services/SpellCheck/spellcheckinfo.h \
    Services/SpellCheck/spellcheckitem.h \
    Services/SpellCheck/spellcheckservice.h \
    Services/SpellCheck/spellchecksuggestionmodel.h \
    Services/SpellCheck/spellcheckworker.h \
    Services/SpellCheck/spellsuggestionsitem.h \
    Services/SpellCheck/spellsuggestionstarget.h \
    Services/SpellCheck/userdicteditmodel.h \
    Services/SpellCheck/userdictionary.h \
    Services/Translation/translationmanager.h \
    Services/Translation/translationquery.h \
    Services/Translation/translationservice.h \
    Services/Translation/translationworker.h \
    Services/Warnings/iwarningsitem.h \
    Services/Warnings/iwarningssettings.h \
    Services/Warnings/warningscheckingworker.h \
    Services/Warnings/warningsitem.h \
    Services/Warnings/warningsmodel.h \
    Services/Warnings/warningsservice.h \
    Services/Warnings/warningssettingsmodel.h \
    Storage/database.h \
    Storage/databasemanager.h \
    Storage/idatabase.h \
    Storage/idatabasemanager.h \
    Storage/memorytable.h \
    Storage/writeaheadlog.h \
    Suggestion/fotoliasuggestionengine.h \
    Suggestion/gettysuggestionengine.h \
    Suggestion/isuggestionengine.h \
    Suggestion/isuggestionsrepository.h \
    Suggestion/keywordssuggestor.h \
    Suggestion/locallibraryquery.h \
    Suggestion/locallibraryqueryengine.h \
    Suggestion/microstocksuggestionengine.h \
    Suggestion/shutterstocksuggestionengine.h \
    Suggestion/suggestionartwork.h \
    Suggestion/suggestionresultsresponse.h \
    UndoRedo/artworkmetadatabackup.h \
    UndoRedo/iundoredomanager.h \
    UndoRedo/undoredomanager.h \
    xpiksapp.h

DISTFILES += \
    ../xpiks-common/xpiks-common.pri \
    AppHost.qml \
    CollapserTabs/FilesFoldersIcon.qml \
    CollapserTabs/FilesFoldersTab.qml \
    CollapserTabs/QuickBufferIcon.qml \
    CollapserTabs/QuickBufferTab.qml \
    CollapserTabs/TranslatorIcon.qml \
    CollapserTabs/TranslatorTab.qml \
    Components/AddIcon.qml \
    Components/BackGlyphButton.qml \
    Components/CheckedComponent.qml \
    Components/CloseIcon.qml \
    Components/ComboBoxPopup.qml \
    Components/CompletionBox.qml \
    Components/CustomBorder.qml \
    Components/CustomScrollbar.qml \
    Components/CustomTab.qml \
    Components/CustomTooltip.qml \
    Components/DonateComponent.qml \
    Components/DotsButton.qml \
    Components/DraggableKeywordWrapper.qml \
    Components/DropdownPopup.qml \
    Components/EditIcon.qml \
    Components/EditableTags.qml \
    Components/GlyphButton.qml \
    Components/KeywordWrapper.qml \
    Components/LargeAddIcon.qml \
    Components/LayoutButton.qml \
    Components/LoaderIcon.qml \
    Components/PresentationSlide.qml \
    Components/SelectedIcon.qml \
    Components/SuggestionWrapper.qml \
    Components/ToolButton.qml \
    Components/ZoomAmplifier.qml \
    Constants/UIConfig.js \
    Dialogs/AboutWindow.qml \
    Dialogs/AddPresetGroupDialog.qml \
    Dialogs/CsvExportDialog.qml \
    Dialogs/DeleteKeywordsDialog.qml \
    Dialogs/DonateDialog.qml \
    Dialogs/EditKeywordDialog.qml \
    Dialogs/EnterMasterPasswordDialog.qml \
    Dialogs/ExportMetadata.qml \
    Dialogs/FailedUploadArtworks.qml \
    Dialogs/FindAndReplace.qml \
    Dialogs/ImportMetadata.qml \
    Dialogs/InstallUpdateDialog.qml \
    Dialogs/KeywordsSuggestion.qml \
    Dialogs/LogsDialog.qml \
    Dialogs/MasterPasswordSetupDialog.qml \
    Dialogs/PlainTextKeywordsDialog.qml \
    Dialogs/PluginsDialog.qml \
    Dialogs/PresetsEditDialog.qml \
    Dialogs/ProxySetupDialog.qml \
    Dialogs/ReplacePreview.qml \
    Dialogs/SettingsWindow.qml \
    Dialogs/SimplePreview.qml \
    Dialogs/SpellCheckSuggestionsDialog.qml \
    Dialogs/TermsAndConditionsDialog.qml \
    Dialogs/TranslationPreviewDialog.qml \
    Dialogs/UpdateWindow.qml \
    Dialogs/UploadArtworks.qml \
    Dialogs/UserDictEditDialog.qml \
    Dialogs/WarningsDialog.qml \
    Dialogs/WhatsNewDialog.qml \
    Dialogs/WhatsNewMinorDialog.qml \
    Dialogs/WipeMetadata.qml \
    Dialogs/ZipArtworksDialog.qml \
    Graphics/black/Edit_icon_clicked.svg \
    Graphics/black/Edit_icon_disabled.svg \
    Graphics/black/Edit_icon_hovered.svg \
    Graphics/black/Edit_icon_normal.svg \
    Graphics/black/Icon_donate.svg \
    Graphics/black/Remove_icon_clicked.svg \
    Graphics/black/Remove_icon_disabled.svg \
    Graphics/black/Remove_icon_hovered.svg \
    Graphics/black/Remove_icon_normal.svg \
    Graphics/black/Save_icon_clicked.svg \
    Graphics/black/Save_icon_disabled.svg \
    Graphics/black/Save_icon_hovered.svg \
    Graphics/black/Save_icon_normal.svg \
    Graphics/black/Upload_icon_clicked.svg \
    Graphics/black/Upload_icon_disabled.svg \
    Graphics/black/Upload_icon_hovered.svg \
    Graphics/black/Upload_icon_normal.svg \
    Graphics/faileduploads.png \
    Graphics/findandreplace.png \
    Graphics/gears.png \
    Graphics/presets.png \
    Graphics/quickbuffer.png \
    Graphics/slategray/Edit_icon_clicked.svg \
    Graphics/slategray/Edit_icon_disabled.svg \
    Graphics/slategray/Edit_icon_hovered.svg \
    Graphics/slategray/Edit_icon_normal.svg \
    Graphics/slategray/Icon_donate.svg \
    Graphics/slategray/More_icon_clicked.svg \
    Graphics/slategray/More_icon_disabled.svg \
    Graphics/slategray/More_icon_hovered.svg \
    Graphics/slategray/More_icon_normal.svg \
    Graphics/slategray/Remove_icon_clicked.svg \
    Graphics/slategray/Remove_icon_disabled.svg \
    Graphics/slategray/Remove_icon_hovered.svg \
    Graphics/slategray/Remove_icon_normal.svg \
    Graphics/slategray/Save_icon_clicked.svg \
    Graphics/slategray/Save_icon_disabled.svg \
    Graphics/slategray/Save_icon_hovered.svg \
    Graphics/slategray/Save_icon_normal.svg \
    Graphics/slategray/Upload_icon_clicked.svg \
    Graphics/slategray/Upload_icon_disabled.svg \
    Graphics/slategray/Upload_icon_hovered.svg \
    Graphics/slategray/Upload_icon_normal.svg \
    Graphics/translator.png \
    Graphics/vector-icon.svg \
    StackViews/ArtworkEditView.qml \
    StackViews/CombinedEditView.qml \
    StackViews/DuplicatesReView.qml \
    StackViews/MainGrid.qml \
    StackViews/WarningsView.qml \
    StyledControls/SimpleProgressBar.qml \
    StyledControls/StyledAddHostButton.qml \
    StyledControls/StyledBlackButton.qml \
    StyledControls/StyledButton.qml \
    StyledControls/StyledCheckbox.qml \
    StyledControls/StyledLink.qml \
    StyledControls/StyledScrollView.qml \
    StyledControls/StyledSlider.qml \
    StyledControls/StyledTabView.qml \
    StyledControls/StyledText.qml \
    StyledControls/StyledTextEdit.qml \
    StyledControls/StyledTextInput.qml \
    uncrustify.cfg \
    xpiks-qt.ico \
    Components/BaseDialog.qml

lupdate_only {
SOURCES += *.qml \
          *.js \
          Components/*.qml \
          Dialogs/*.qml \
          StyledControls/*.qml \
          CollapserTabs/*.qml \
          StackViews/*.qml
}

INCLUDEPATH += ./
INCLUDEPATH += ../../

CONFIG(debug, debug|release)  {
    LIBS += -L"$$PWD/../../libs/debug"
} else {
    LIBS += -L"$$PWD/../../libs/release"
}

LIBS += -lhunspell
LIBS += -lcurl
LIBS += -lface
LIBS += -lssdll
LIBS += -lquazip
LIBS += -lz
LIBS += -lthmbnlr
LIBS += -lxpks
LIBS += -lchillout

BUILDNO=$$system(git log -n 1 --pretty=format:"%h")
BRANCH_NAME=$$system(git rev-parse --abbrev-ref HEAD)

include(../xpiks-common/xpiks-common.pri)

DEFINES += WITH_UPDATES
DEFINES += WITH_LOGS

CONFIG(debug, debug|release)  {
    message("Building debug")
    DEFINES += WITH_STDOUT_LOGS
    DEFINES += WITH_PLUGINS
    #QMAKE_CXXFLAGS += -fsanitize=thread
} else {
    message("Building release")
}

macx {
    QMAKE_TARGET_BUNDLE_PREFIX = com.xpiksapp
    QMAKE_BUNDLE = Xpiks

    LIBS += -ldl

    INCLUDEPATH += "../../vendors/libcurl/include"

    LIBS += -lavcodec.57
    LIBS += -lavfilter.6
    LIBS += -lavformat.57
    LIBS += -lavutil.55
    LIBS += -lswscale.4

    HUNSPELL_DICT_FILES.files = deps/dict/en_US.aff deps/dict/en_US.dic deps/dict/license.txt deps/dict/README_en_US.txt
    HUNSPELL_DICT_FILES.path = Contents/Resources/dict
    QMAKE_BUNDLE_DATA += HUNSPELL_DICT_FILES

    WHATS_NEW.files = deps/whatsnew.txt
    WHATS_NEW.path = Contents/Resources
    QMAKE_BUNDLE_DATA += WHATS_NEW

    TRANSLATIONS_FILES_LIST = $$system(ls $$PWD/deps/translations/*.qm)
    XPIKS_TRANSLATIONS.files = $$TRANSLATIONS_FILES_LIST
    XPIKS_TRANSLATIONS.path = Contents/Resources/translations
    QMAKE_BUNDLE_DATA += XPIKS_TRANSLATIONS

    FREQ_TABLES.files = deps/ac_sources/en_wordlist.tsv
    FREQ_TABLES.path = Contents/Resources/ac_sources
    QMAKE_BUNDLE_DATA += FREQ_TABLES

    RECOVERTY.files = deps/recoverty/Recoverty.app
    RECOVERTY.path = Contents/MacOS/
    QMAKE_BUNDLE_DATA += RECOVERTY
}

win32 {
    DEFINES += QT_NO_PROCESS_COMBINED_ARGUMENT_START
    QT += winextras
    DEFINES += ZLIB_WINAPI \
               ZLIB_DLL
    INCLUDEPATH += "../../vendors/zlib-1.2.11"
    INCLUDEPATH += "../../vendors/libcurl/include"

    LIBS -= -lcurl

    LIBS += -lavcodec
    LIBS += -lavfilter
    LIBS += -lavformat
    LIBS += -lavutil
    LIBS += -lswscale

    CONFIG(debug, debug|release) {
        LIBS += -llibcurl_debug
        LIBS -= -lquazip
        LIBS += -lquazipd
    }

    CONFIG(release, debug|release) {
        LIBS += -llibcurl
    }

    LIBS += -lmman
    # chillout deps
    LIBS += -lAdvapi32 -lDbgHelp

    # recoverty steps
    RECOVERTY_DIR = recoverty
    copyrecoverty.commands = $(COPY_FILE) \"$$shell_path($$DEPS_DIR/$$RECOVERTY_DIR/Recoverty*)\" \"$$shell_path($$OUT_PWD/$$EXE_DIR/)\"
    QMAKE_EXTRA_TARGETS += copyrecoverty
    POST_TARGETDEPS += copyrecoverty
}

linux {
    message("for Linux")
    BUILDNO = $$system($$PWD/../../scripts/build/buildno.sh)

    LIBS += -ldl

    LIBS += -lavcodec
    LIBS += -lavfilter
    LIBS += -lavformat
    LIBS += -lavutil
    LIBS += -lswscale

    #QML_IMPORT_PATH += /usr/lib/x86_64-linux-gnu/qt5/imports/
    #LIBS += -L/lib/x86_64-linux-gnu/
}

travis-ci {
    message("for Travis CI")
    DEFINES += TRAVIS_CI

    linux {
        LIBS -= -lz
        LIBS += /usr/lib/x86_64-linux-gnu/libz.so
    }

    macx {
        CONFIG += sdk_no_version_check
    }
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
    SOURCES += ../../vendors/libthmbnlr/thumbnailcreator_stub.cpp
}

DEFINES += BUILDNUMBER=$${BUILDNO}
DEFINES += BRANCHNAME=$${BRANCH_NAME}
