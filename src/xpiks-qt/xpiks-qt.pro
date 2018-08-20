TEMPLATE = app

QMAKE_MAC_SDK = macosx10.11

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
    Helpers/globalimageprovider.cpp \
    Helpers/indiceshelper.cpp \
    ../../vendors/tiny-aes/aes.cpp \
    ../../vendors/sqlite/sqlite3.c \
    Encryption/secretsmanager.cpp \
    Helpers/stringhelper.cpp \
    Commands/commandmanager.cpp \
    UndoRedo/undoredomanager.cpp \
    UndoRedo/artworkmetadatabackup.cpp \
    Helpers/runguard.cpp \
    Encryption/aes-qt.cpp \
    Helpers/ziphelper.cpp \
    Suggestion/keywordssuggestor.cpp \
    Helpers/loggingworker.cpp \
    Helpers/logger.cpp \
    Helpers/helpersqmlwrapper.cpp \
    Connectivity/updateservice.cpp \
    Services/SpellCheck/spellcheckitem.cpp \
    Services/SpellCheck/spellcheckworker.cpp \
    Services/SpellCheck/spellchecksuggestionmodel.cpp \
    Services/SpellCheck/spellcheckerrorshighlighter.cpp \
    Services/SpellCheck/spellsuggestionsitem.cpp \
    Connectivity/telemetryservice.cpp \
    Connectivity/updatescheckerworker.cpp \
    Services/Warnings/warningscheckingworker.cpp \
    MetadataIO/metadataiocoordinator.cpp \
    Connectivity/testconnection.cpp \
    Connectivity/ftphelpers.cpp \
    Plugins/pluginmanager.cpp \
    Plugins/pluginwrapper.cpp \
    Plugins/pluginactionsmodel.cpp \
    Plugins/uiprovider.cpp \
    Services/Warnings/warningsservice.cpp \
    Helpers/loghighlighter.cpp \
    Services/Warnings/warningsmodel.cpp \
    Helpers/filterhelpers.cpp \
    QMLExtensions/triangleelement.cpp \
    Suggestion/locallibraryqueryengine.cpp \
    QMLExtensions/colorsmodel.cpp \
    Helpers/remoteconfig.cpp \
    Helpers/localconfig.cpp \
    Helpers/jsonhelper.cpp \
    Services/AutoComplete/autocompleteworker.cpp \
    Services/AutoComplete/autocompleteservice.cpp \
    Microstocks/stocksftplistmodel.cpp \
    QMLExtensions/imagecachingworker.cpp \
    QMLExtensions/imagecachingservice.cpp \
    QMLExtensions/cachingimageprovider.cpp \
    Helpers/metadatahighlighter.cpp \
    Helpers/keywordshelpers.cpp \
    Connectivity/uploadwatcher.cpp \
    Connectivity/telemetryworker.cpp \
    Services/Warnings/warningssettingsmodel.cpp \
    Connectivity/simplecurlrequest.cpp \
    Connectivity/curlinithelper.cpp \
    Connectivity/simplecurldownloader.cpp \
    Helpers/updatehelpers.cpp \
    KeywordsPresets/presetkeywordsmodel.cpp \
    KeywordsPresets/presetkeywordsmodelconfig.cpp \
    QMLExtensions/folderelement.cpp \
    Services/Translation/translationservice.cpp \
    Services/Translation/translationmanager.cpp \
    Services/Translation/translationworker.cpp \
    Services/Translation/translationquery.cpp \
    Plugins/sandboxeddependencies.cpp \
    Services/SpellCheck/userdicteditmodel.cpp \
    Helpers/asynccoordinator.cpp \
    QMLExtensions/tabsmodel.cpp \
    Services/Maintenance/maintenanceworker.cpp \
    Services/Maintenance/maintenanceservice.cpp \
    Services/Maintenance/logscleanupjobitem.cpp \
    Services/Maintenance/updatescleanupjobitem.cpp \
    Services/Maintenance/launchexiftooljobitem.cpp \
    Services/Maintenance/initializedictionariesjobitem.cpp \
    QMLExtensions/videocachingservice.cpp \
    QMLExtensions/videocachingworker.cpp \
    Helpers/filehelpers.cpp \
    Helpers/artworkshelpers.cpp \
    Services/Maintenance/savesessionjobitem.cpp \
    Connectivity/switcherconfig.cpp \
    Connectivity/requestsworker.cpp \
    Connectivity/requestsservice.cpp \
    Storage/database.cpp \
    QMLExtensions/cachedimage.cpp \
    QMLExtensions/dbimagecacheindex.cpp \
    QMLExtensions/cachedvideo.cpp \
    QMLExtensions/dbvideocacheindex.cpp \
    MetadataIO/cachedartwork.cpp \
    MetadataIO/metadatacache.cpp \
    MetadataIO/metadataioworker.cpp \
    MetadataIO/metadataioservice.cpp \
    MetadataIO/metadatareadinghub.cpp \
    Services/AutoComplete/libfacecompletionengine.cpp \
    Services/AutoComplete/autocompletemodel.cpp \
    Services/AutoComplete/keywordsautocompletemodel.cpp \
    Services/AutoComplete/stringsautocompletemodel.cpp \
    Services/AutoComplete/presetscompletionengine.cpp \
    Services/SpellCheck/duplicatesreviewmodel.cpp \
    Services/SpellCheck/duplicateshighlighter.cpp \
    MetadataIO/csvexportworker.cpp \
    MetadataIO/csvexportplansmodel.cpp \
    MetadataIO/csvexportproperties.cpp \
    MetadataIO/csvexportmodel.cpp \
    Helpers/threadhelpers.cpp \
    KeywordsPresets/presetgroupsmodel.cpp \
    Services/Maintenance/xpkscleanupjob.cpp \
    Services/Warnings/warningsitem.cpp \
    Services/Maintenance/updatebundlecleanupjobitem.cpp \
    Plugins/pluginenvironment.cpp \
    Plugins/plugindatabasemanager.cpp \
    Storage/databasemanager.cpp \
    Connectivity/configrequest.cpp \
    Suggestion/suggestionresultsresponse.cpp \
    Microstocks/shutterstockapiclient.cpp \
    Microstocks/fotoliaapiclient.cpp \
    Microstocks/gettyapiclient.cpp \
    Suggestion/shutterstocksuggestionengine.cpp \
    Suggestion/fotoliasuggestionengine.cpp \
    Suggestion/gettysuggestionengine.cpp \
    Microstocks/microstockservice.cpp \
    Storage/memorytable.cpp \
    xpiksapp.cpp \
    Encryption/obfuscation.cpp \
    Filesystem/filescollection.cpp \
    Filesystem/directoriescollection.cpp \
    Filesystem/filesdirectoriescollection.cpp \
    Helpers/indicesranges.cpp \
    Artworks/artworkssnapshot.cpp \
    Artworks/artworkmetadata.cpp \
    Artworks/imageartwork.cpp \
    Artworks/videoartwork.cpp \
    Artworks/basickeywordsmodel.cpp \
    Artworks/basickeywordsmodelimpl.cpp \
    Artworks/basicmetadatamodel.cpp \
    Services/artworksupdatehub.cpp \
    Services/SpellCheck/userdictionary.cpp \
    Commands/Services/autoimportmetadatacommand.cpp \
    Commands/Services/cleanuplegacybackupscommand.cpp \
    Commands/Services/generatethumbnailstemplate.cpp \
    Commands/Services/savebackupstemplate.cpp \
    Commands/Services/savesessioncommand.cpp \
    Commands/Files/addfilescommand.cpp \
    Commands/Files/addtorecenttemplate.cpp \
    Commands/Files/removedirectorycommand.cpp \
    Commands/Files/removefilescommandbase.cpp \
    Commands/Editing/editartworkstemplate.cpp \
    Commands/Editing/expandpresettemplate.cpp \
    Commands/Editing/keywordedittemplate.cpp \
    Commands/Editing/readmetadatatemplate.cpp \
    Models/Artworks/artworkslistmodel.cpp \
    Models/Artworks/artworksrepository.cpp \
    Models/Artworks/artworksviewmodel.cpp \
    Models/Artworks/filteredartworkslistmodel.cpp \
    Models/Connectivity/abstractconfigupdatermodel.cpp \
    Models/Connectivity/proxysettings.cpp \
    Models/Connectivity/uploadinforepository.cpp \
    Models/Connectivity/ziparchiver.cpp \
    Models/Editing/artworkproxybase.cpp \
    Models/Editing/artworkproxymodel.cpp \
    Models/Editing/combinedartworksmodel.cpp \
    Models/Editing/deletekeywordsviewmodel.cpp \
    Models/Editing/findandreplacemodel.cpp \
    Models/Session/recentdirectoriesmodel.cpp \
    Models/Session/recentfilesmodel.cpp \
    Models/Session/recentitemsmodel.cpp \
    Models/Session/sessionmanager.cpp \
    Commands/artworksupdatetemplate.cpp \
    Models/keyvaluelist.cpp \
    Models/languagesmodel.cpp \
    Models/logsmodel.cpp \
    Models/settingsmodel.cpp \
    Models/switchermodel.cpp \
    Models/uimanager.cpp \
    Common/statefulentity.cpp \
    Common/systemenvironment.cpp \
    Models/Editing/currenteditableartwork.cpp \
    Models/Editing/currenteditableproxyartwork.cpp \
    Commands/Editing/deletekeywordstemplate.cpp \
    Models/Editing/currenteditablemodel.cpp \
    Models/Editing/quickbuffer.cpp \
    Commands/Editing/findandreplacetemplate.cpp \
    Services/SpellCheck/spellcheckservice.cpp \
    Services/artworkseditinghub.cpp \
    QMLExtensions/uicommanddispatcher.cpp \
    Commands/UI/selectedartworkscommands.cpp \
    Commands/UI/generalcommands.cpp \
    Commands/UI/singleeditablecommands.cpp \
    Common/logging.cpp \
    Models/Connectivity/artworksuploader.cpp \
    Commands/Files/removefilescommand.cpp \
    Commands/Files/removeselectedfilescommand.cpp \
    Services/SpellCheck/metadataduplicates.cpp \
    Services/SpellCheck/spellsuggestionstarget.cpp \
    Services/SpellCheck/spellcheckinfo.cpp

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
    Helpers/globalimageprovider.h \
    Helpers/indiceshelper.h \
    Helpers/clipboardhelper.h \
    Helpers/constants.h \
    Encryption/aes-qt.h \
    ../../vendors/tiny-aes/aes.h \
    ../../vendors/sqlite/sqlite3.h \
    Encryption/secretsmanager.h \
    Helpers/stringhelper.h \
    Helpers/logger.h \
    Commands/commandmanager.h \
    UndoRedo/undoredomanager.h \
    UndoRedo/artworkmetadatabackup.h \
    Helpers/runguard.h \
    Helpers/ziphelper.h \
    Suggestion/keywordssuggestor.h \
    Suggestion/suggestionartwork.h \
    Helpers/loggingworker.h \
    Helpers/helpersqmlwrapper.h \
    Connectivity/updateservice.h \
    Services/SpellCheck/spellcheckitem.h \
    Services/SpellCheck/spellcheckworker.h \
    Services/SpellCheck/spellchecksuggestionmodel.h \
    Services/SpellCheck/spellcheckerrorshighlighter.h \
    Services/SpellCheck/spellsuggestionsitem.h \
    Connectivity/analyticsuserevent.h \
    Connectivity/telemetryservice.h \
    Connectivity/updatescheckerworker.h \
    Services/Warnings/warningscheckingworker.h \
    Services/Warnings/warningsitem.h \
    MetadataIO/metadataiocoordinator.h \
    Connectivity/testconnection.h \
    Connectivity/ftphelpers.h \
    Plugins/xpiksplugininterface.h \
    UndoRedo/iundoredomanager.h \
    Plugins/pluginmanager.h \
    Plugins/pluginwrapper.h \
    Plugins/ipluginaction.h \
    Plugins/pluginactionsmodel.h \
    Plugins/uiprovider.h \
    Plugins/iuiprovider.h \
    Services/Warnings/warningsservice.h \
    Helpers/loghighlighter.h \
    Services/Warnings/warningsmodel.h \
    Helpers/filterhelpers.h \
    Connectivity/iftpcoordinator.h \
    QMLExtensions/triangleelement.h \
    Suggestion/locallibraryqueryengine.h \
    QMLExtensions/colorsmodel.h \
    Helpers/remoteconfig.h \
    Helpers/localconfig.h \
    Helpers/jsonhelper.h \
    Helpers/comparevaluesjson.h \
    Services/AutoComplete/autocompleteworker.h \
    Services/AutoComplete/completionquery.h \
    Services/AutoComplete/autocompleteservice.h \
    Microstocks/stocksftplistmodel.h \
    QMLExtensions/imagecachingworker.h \
    QMLExtensions/imagecacherequest.h \
    QMLExtensions/imagecachingservice.h \
    QMLExtensions/cachingimageprovider.h \
    Helpers/metadatahighlighter.h \
    Helpers/keywordshelpers.h \
    Connectivity/uploadwatcher.h \
    Connectivity/telemetryworker.h \
    Services/Warnings/warningssettingsmodel.h \
    Connectivity/simplecurlrequest.h \
    Connectivity/curlinithelper.h \
    Connectivity/simplecurldownloader.h \
    Connectivity/apimanager.h \
    Helpers/updatehelpers.h \
    KeywordsPresets/presetkeywordsmodel.h \
    KeywordsPresets/presetkeywordsmodelconfig.h \
    QMLExtensions/folderelement.h \
    Services/Translation/translationservice.h \
    Services/Translation/translationmanager.h \
    Services/Translation/translationworker.h \
    Services/Translation/translationquery.h \
    Plugins/sandboxeddependencies.h \
    KeywordsPresets/ipresetsmanager.h \
    Services/SpellCheck/userdicteditmodel.h \
    Helpers/asynccoordinator.h \
    QMLExtensions/tabsmodel.h \
    Services/Maintenance/maintenanceworker.h \
    Services/Maintenance/maintenanceservice.h \
    Services/Maintenance/imaintenanceitem.h \
    Services/Maintenance/logscleanupjobitem.h \
    Services/Maintenance/updatescleanupjobitem.h \
    Services/Maintenance/launchexiftooljobitem.h \
    Services/Maintenance/initializedictionariesjobitem.h \
    QMLExtensions/videocachingservice.h \
    QMLExtensions/videocachingworker.h \
    QMLExtensions/videocacherequest.h \
    Helpers/filehelpers.h \
    Helpers/artworkshelpers.h \
    Services/Maintenance/savesessionjobitem.h \
    Connectivity/switcherconfig.h \
    Connectivity/requestsworker.h \
    Connectivity/requestsservice.h \
    Services/Warnings/iwarningsitem.h \
    Services/AutoComplete/completionitem.h \
    Storage/database.h \
    Services/AutoComplete/completionitem.h \
    QMLExtensions/previewstorage.h \
    QMLExtensions/cachedimage.h \
    QMLExtensions/dbimagecacheindex.h \
    QMLExtensions/dbcacheindex.h \
    QMLExtensions/cachedvideo.h \
    QMLExtensions/dbvideocacheindex.h \
    MetadataIO/cachedartwork.h \
    MetadataIO/metadatacache.h \
    MetadataIO/metadataioworker.h \
    MetadataIO/metadataiotask.h \
    MetadataIO/metadataioservice.h \
    MetadataIO/originalmetadata.h \
    Microstocks/searchquery.h \
    Suggestion/locallibraryquery.h \
    MetadataIO/metadatareadinghub.h \
    Services/AutoComplete/completionenginebase.h \
    Services/AutoComplete/libfacecompletionengine.h \
    Services/AutoComplete/autocompletemodel.h \
    Services/AutoComplete/keywordsautocompletemodel.h \
    Services/AutoComplete/stringsautocompletemodel.h \
    Services/AutoComplete/presetscompletionengine.h \
    Services/SpellCheck/duplicatesreviewmodel.h \
    Services/SpellCheck/duplicateshighlighter.h \
    MetadataIO/csvexportworker.h \
    MetadataIO/csvexportproperties.h \
    MetadataIO/csvexportplansmodel.h \
    MetadataIO/csvexportmodel.h \
    Helpers/threadhelpers.h \
    KeywordsPresets/presetgroupsmodel.h \
    Services/Maintenance/xpkscleanupjob.h \
    KeywordsPresets/presetmodel.h \
    KeywordsPresets/groupmodel.h \
    Services/Warnings/iwarningssettings.h \
    Services/Maintenance/updatebundlecleanupjobitem.h \
    Storage/idatabasemanager.h \
    Plugins/pluginenvironment.h \
    Plugins/plugindatabasemanager.h \
    Helpers/jsonobjectmap.h \
    Storage/databasemanager.h \
    Storage/idatabase.h \
    Storage/writeaheadlog.h \
    Connectivity/iconnectivityrequest.h \
    Connectivity/configrequest.h \
    Connectivity/iconnectivityresponse.h \
    Suggestion/suggestionresultsresponse.h \
    Microstocks/imicrostockapiclient.h \
    Microstocks/shutterstockapiclient.h \
    Connectivity/simpleapirequest.h \
    Suggestion/microstocksuggestionengine.h \
    Suggestion/isuggestionsrepository.h \
    Suggestion/isuggestionengine.h \
    Microstocks/fotoliaapiclient.h \
    Microstocks/gettyapiclient.h \
    Suggestion/shutterstocksuggestionengine.h \
    Suggestion/fotoliasuggestionengine.h \
    Suggestion/gettysuggestionengine.h \
    Microstocks/imicrostockservice.h \
    Microstocks/microstockservice.h \
    Microstocks/imicrostockservices.h \
    Encryption/secretpair.h \
    Encryption/isecretsstorage.h \
    Storage/memorytable.h \
    xpiksapp.h \
    Microstocks/stockftpoptions.h \
    Encryption/obfuscation.h \
    Filesystem/filescollection.h \
    Filesystem/ifilescollection.h \
    Filesystem/directoriescollection.h \
    Filesystem/filesdirectoriescollection.h \
    Helpers/indicesranges.h \
    Artworks/artworkssnapshot.h \
    Artworks/artworkmetadata.h \
    Artworks/imageartwork.h \
    Artworks/videoartwork.h \
    Artworks/artworkelement.h \
    Artworks/basickeywordsmodel.h \
    Artworks/basickeywordsmodelimpl.h \
    Artworks/basicmetadatamodel.h \
    Artworks/keyword.h \
    Services/artworksupdatehub.h \
    Services/artworkupdaterequest.h \
    Helpers/cpphelpers.h \
    Services/SpellCheck/userdictionary.h \
    Services/AutoComplete/icompletionsource.h \
    QMLExtensions/uicommandid.h \
    Commands/Services/autoimportmetadatacommand.h \
    Commands/Services/cleanuplegacybackupscommand.h \
    Commands/Services/generatethumbnailstemplate.h \
    Commands/Services/savebackupstemplate.h \
    Commands/Services/savesessioncommand.h \
    Commands/Files/addfilescommand.h \
    Commands/Files/addtorecenttemplate.h \
    Commands/Files/removedirectorycommand.h \
    Commands/Files/removefilescommandbase.h \
    Commands/Base/compositecommandtemplate.h \
    Commands/Base/emptycommand.h \
    Commands/Base/icommand.h \
    Commands/Base/icommandmanager.h \
    Commands/Base/icommandtemplate.h \
    Commands/Base/iuicommandtemplate.h \
    Commands/Base/templatedcommand.h \
    Commands/Base/templateduicommand.h \
    Commands/Editing/editartworkstemplate.h \
    Commands/Editing/expandpresettemplate.h \
    Commands/Editing/keywordedittemplate.h \
    Commands/Editing/modifyartworkscommand.h \
    Commands/Editing/readmetadatatemplate.h \
    Models/Artworks/artworkslistmodel.h \
    Models/Artworks/artworksrepository.h \
    Models/Artworks/artworksviewmodel.h \
    Models/Artworks/filteredartworkslistmodel.h \
    Models/Connectivity/abstractconfigupdatermodel.h \
    Models/Connectivity/proxysettings.h \
    Models/Connectivity/uploadinfo.h \
    Models/Connectivity/uploadinforepository.h \
    Models/Connectivity/ziparchiver.h \
    Models/Editing/artworkproxybase.h \
    Models/Editing/artworkproxymodel.h \
    Models/Editing/combinedartworksmodel.h \
    Models/Editing/deletekeywordsviewmodel.h \
    Models/Editing/findandreplacemodel.h \
    Models/Editing/previewartworkelement.h \
    Models/Session/recentdirectoriesmodel.h \
    Models/Session/recentfilesmodel.h \
    Models/Session/recentitemsmodel.h \
    Models/Session/sessionmanager.h \
    Commands/artworksupdatetemplate.h \
    Models/exportinfo.h \
    Models/keyvaluelist.h \
    Models/languagesmodel.h \
    Models/logsmodel.h \
    Models/settingsmodel.h \
    Models/switchermodel.h \
    Models/uimanager.h \
    Common/abstractlistmodel.h \
    Common/defines.h \
    Common/delayedactionentity.h \
    Common/flags.h \
    Common/iflagsprovider.h \
    Common/isystemenvironment.h \
    Common/itemprocessingworker.h \
    Common/logging.h \
    Common/readerwriterqueue.h \
    Common/statefulentity.h \
    Common/systemenvironment.h \
    Common/types.h \
    Common/version.h \
    Common/wordanalysisresult.h \
    Models/Editing/currenteditableartwork.h \
    Models/Editing/currenteditableproxyartwork.h \
    Models/Editing/icurrenteditable.h \
    Commands/Editing/deletekeywordstemplate.h \
    Commands/Base/simplecommand.h \
    Models/Editing/currenteditablemodel.h \
    QMLExtensions/iuicommanddispatcher.h \
    Models/Editing/quickbuffer.h \
    Commands/Editing/findandreplacetemplate.h \
    Services/SpellCheck/spellcheckservice.h \
    Services/artworkseditinghub.h \
    Common/messages.h \
    Models/Editing/quickbuffermessage.h \
    Services/SpellCheck/ispellcheckable.h \
    Artworks/iartworkmetadata.h \
    QMLExtensions/uicommanddispatcher.h \
    Commands/UI/selectedartworkscommands.h \
    Commands/UI/generalcommands.h \
    Artworks/iselectedartworkssource.h \
    Artworks/iselectedindicessource.h \
    Commands/UI/singleeditablecommands.h \
    Commands/Base/commanduiwrapper.h \
    Models/Artworks/artworkslistoperations.h \
    Models/Connectivity/artworksuploader.h \
    Commands/UI/sourcetargetcommand.h \
    Services/iartworksupdater.h \
    Connectivity/irequestsservice.h \
    Microstocks/imicrostockapiclients.h \
    Microstocks/microstockenums.h \
    Microstocks/microstockapiclients.h \
    Commands/Files/removefilescommand.h \
    Commands/Files/removeselectedfilescommand.h \
    Services/SpellCheck/imetadataduplicates.h \
    Services/SpellCheck/metadataduplicates.h \
    Services/SpellCheck/ispellsuggestionstarget.h \
    Services/SpellCheck/spellsuggestionstarget.h \
    Services/SpellCheck/ispellcheckservice.h \
    Artworks/ibasicmodelsource.h \
    Services/SpellCheck/spellcheckinfo.h \
    Artworks/basicmodelsource.h

DISTFILES += \
    Components/CloseIcon.qml \
    Components/EditableTags.qml \
    StyledControls/SimpleProgressBar.qml \
    StyledControls/StyledButton.qml \
    StyledControls/StyledCheckbox.qml \
    StyledControls/StyledScrollView.qml \
    StyledControls/StyledText.qml \
    StyledControls/StyledTextInput.qml \
    Dialogs/ExportMetadata.qml \
    Dialogs/ImportMetadata.qml \
    Dialogs/SettingsWindow.qml \
    Dialogs/UploadArtworks.qml \
    Dialogs/LogsDialog.qml \
    StyledControls/StyledTextEdit.qml \
    Dialogs/WarningsDialog.qml \
    xpiks-qt.ico \
    Dialogs/AboutWindow.qml \
    StyledControls/StyledAddHostButton.qml \
    Dialogs/EnterMasterPasswordDialog.qml \
    Dialogs/MasterPasswordSetupDialog.qml \
    Components/CheckedComponent.qml \
    Dialogs/ZipArtworksDialog.qml \
    Components/AddIcon.qml \
    Dialogs/KeywordsSuggestion.qml \
    Components/LargeAddIcon.qml \
    StyledControls/StyledTabView.qml \
    Components/CustomBorder.qml \
    StyledControls/StyledBlackButton.qml \
    StyledControls/StyledSlider.qml \
    Components/KeywordWrapper.qml \
    Components/CustomScrollbar.qml \
    Dialogs/UpdateWindow.qml \
    Dialogs/SpellCheckSuggestionsDialog.qml \
    Components/SuggestionWrapper.qml \
    Dialogs/EditKeywordDialog.qml \
    Dialogs/PlainTextKeywordsDialog.qml \
    Dialogs/WhatsNewDialog.qml \
    Dialogs/TermsAndConditionsDialog.qml \
    Dialogs/PluginsDialog.qml \
    Components/LayoutButton.qml \
    Graphics/vector-icon.svg \
    Constants/UIConfig.js \
    Components/SelectedIcon.qml \
    Components/CompletionBox.qml \
    Components/EditIcon.qml \
    uncrustify.cfg \
    Components/PresentationSlide.qml \
    Graphics/faileduploads.png \
    Graphics/gears.png \
    Graphics/findandreplace.png \
    Graphics/presets.png \
    Graphics/quickbuffer.png \
    Graphics/translator.png \
    Dialogs/ProxySetupDialog.qml \
    Dialogs/WhatsNewMinorDialog.qml \
    Dialogs/FindAndReplace.qml \
    Dialogs/ReplacePreview.qml \
    Dialogs/DeleteKeywordsDialog.qml \
    Dialogs/FailedUploadArtworks.qml \
    Dialogs/InstallUpdateDialog.qml \
    Components/CustomTab.qml \
    StackViews/MainGrid.qml \
    StackViews/ArtworkEditView.qml \
    Components/ZoomAmplifier.qml \
    StackViews/WarningsView.qml \
    Components/GlyphButton.qml \
    Components/BackGlyphButton.qml \
    StackViews/CombinedEditView.qml \
    Dialogs/PresetsEditDialog.qml \
    Dialogs/TranslationPreviewDialog.qml \
    CollapserTabs/FilesFoldersTab.qml \
    CollapserTabs/TranslatorTab.qml \
    CollapserTabs/FilesFoldersIcon.qml \
    CollapserTabs/TranslatorIcon.qml \
    CollapserTabs/QuickBufferIcon.qml \
    CollapserTabs/QuickBufferTab.qml \
    Dialogs/UserDictEditDialog.qml \
    Components/LoaderIcon.qml \
    Graphics/black/Icon_donate.svg \
    Graphics/slategray/Icon_donate.svg \
    Dialogs/DonateDialog.qml \
    Components/DonateComponent.qml \
    StackViews/DuplicatesReView.qml \
    StyledControls/StyledLink.qml \
    Dialogs/CsvExportDialog.qml \
    Components/DotsButton.qml \
    Dialogs/AddPresetGroupDialog.qml \
    Components/DropdownPopup.qml \
    Components/ComboBoxPopup.qml \
    Components/ToolButton.qml \
    Components/CustomTooltip.qml \
    Graphics/black/Edit_icon_clicked.svg \
    Graphics/black/Edit_icon_disabled.svg \
    Graphics/black/Edit_icon_hovered.svg \
    Graphics/black/Edit_icon_normal.svg \
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
    Graphics/slategray/Edit_icon_clicked.svg \
    Graphics/slategray/Edit_icon_disabled.svg \
    Graphics/slategray/Edit_icon_hovered.svg \
    Graphics/slategray/Edit_icon_normal.svg \
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
    Graphics/slategray/More_icon_clicked.svg \
    Graphics/slategray/More_icon_disabled.svg \
    Graphics/slategray/More_icon_hovered.svg \
    Graphics/slategray/More_icon_normal.svg \
    Dialogs/WipeMetadata.qml \
    Dialogs/SimplePreview.qml \
    ../xpiks-common/xpiks-common.pri \
    Components/DraggableKeywordWrapper.qml

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
INCLUDEPATH += ../../vendors/tiny-aes
INCLUDEPATH += ../../vendors/cpp-libface
INCLUDEPATH += ../../vendors/ssdll/src/ssdll
INCLUDEPATH += ../../vendors/hunspell-repo/src
INCLUDEPATH += ../../vendors/libthmbnlr
INCLUDEPATH += ../../vendors/libxpks
INCLUDEPATH += ../../vendors/chillout/src/chillout

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

    INCLUDEPATH += "../../vendors/quazip"
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
    INCLUDEPATH += "../../vendors/quazip"
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
    INCLUDEPATH += "../../vendors/quazip"
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
    LIBS -= -lz
    LIBS += /usr/lib/x86_64-linux-gnu/libz.so
    LIBS += -ldl
    DEFINES += TRAVIS_CI
    INCLUDEPATH += "../../vendors/quazip"
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
