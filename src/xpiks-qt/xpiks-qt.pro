TEMPLATE = app

QMAKE_MAC_SDK = macosx10.11

QT += qml quick widgets concurrent svg
CONFIG += qtquickcompiler
CONFIG += c++11
TARGET = Xpiks

#CONFIG += force_debug_info

CONFIG(release, debug|release)  {
    CONFIG += separate_debug_info
}

VERSION = 1.5.1.1
QMAKE_TARGET_PRODUCT = Xpiks
QMAKE_TARGET_DESCRIPTION = "Cross-Platform Image Keywording Software"
QMAKE_TARGET_COPYRIGHT = "Copyright (C) 2014-2018 Taras Kushnir"

SOURCES += main.cpp \
    Models/artitemsmodel.cpp \
    Models/artworkmetadata.cpp \
    Helpers/globalimageprovider.cpp \
    Models/artworksrepository.cpp \
    Models/combinedartworksmodel.cpp \
    Helpers/indiceshelper.cpp \
    Models/artworkuploader.cpp \
    Models/uploadinforepository.cpp \
    ../../vendors/tiny-aes/aes.cpp \
    ../../vendors/sqlite/sqlite3.c \
    Encryption/secretsmanager.cpp \
    Helpers/stringhelper.cpp \
    Commands/commandmanager.cpp \
    UndoRedo/undoredomanager.cpp \
    Commands/addartworkscommand.cpp \
    UndoRedo/addartworksitem.cpp \
    Commands/removeartworkscommand.cpp \
    UndoRedo/removeartworksitem.cpp \
    UndoRedo/artworkmetadatabackup.cpp \
    UndoRedo/modifyartworkshistoryitem.cpp \
    Commands/combinededitcommand.cpp \
    Commands/pastekeywordscommand.cpp \
    Helpers/runguard.cpp \
    Encryption/aes-qt.cpp \
    Models/ziparchiver.cpp \
    Helpers/ziphelper.cpp \
    Suggestion/keywordssuggestor.cpp \
    Models/settingsmodel.cpp \
    Helpers/loggingworker.cpp \
    Helpers/logger.cpp \
    Models/logsmodel.cpp \
    Models/filteredartitemsproxymodel.cpp \
    Helpers/helpersqmlwrapper.cpp \
    Models/recentdirectoriesmodel.cpp \
    Connectivity/updateservice.cpp \
    SpellCheck/spellcheckerservice.cpp \
    SpellCheck/spellcheckitem.cpp \
    SpellCheck/spellcheckworker.cpp \
    SpellCheck/spellchecksuggestionmodel.cpp \
    Common/basickeywordsmodel.cpp \
    SpellCheck/spellcheckerrorshighlighter.cpp \
    SpellCheck/spellcheckiteminfo.cpp \
    SpellCheck/spellsuggestionsitem.cpp \
    Connectivity/telemetryservice.cpp \
    Connectivity/updatescheckerworker.cpp \
    Warnings/warningscheckingworker.cpp \
    MetadataIO/metadataiocoordinator.cpp \
    Connectivity/testconnection.cpp \
    Connectivity/ftphelpers.cpp \
    Plugins/pluginmanager.cpp \
    Plugins/pluginwrapper.cpp \
    Plugins/pluginactionsmodel.cpp \
    Plugins/uiprovider.cpp \
    Warnings/warningsservice.cpp \
    Helpers/loghighlighter.cpp \
    Warnings/warningsmodel.cpp \
    Models/languagesmodel.cpp \
    Helpers/filterhelpers.cpp \
    QMLExtensions/triangleelement.cpp \
    Suggestion/locallibraryqueryengine.cpp \
    QMLExtensions/colorsmodel.cpp \
    Helpers/remoteconfig.cpp \
    Helpers/localconfig.cpp \
    Helpers/jsonhelper.cpp \
    AutoComplete/autocompleteworker.cpp \
    AutoComplete/autocompleteservice.cpp \
    Models/abstractconfigupdatermodel.cpp \
    Microstocks/stocksftplistmodel.cpp \
    Models/imageartwork.cpp \
    Models/proxysettings.cpp \
    QMLExtensions/imagecachingworker.cpp \
    QMLExtensions/imagecachingservice.cpp \
    QMLExtensions/cachingimageprovider.cpp \
    Commands/findandreplacecommand.cpp \
    Helpers/metadatahighlighter.cpp \
    Models/findandreplacemodel.cpp \
    Commands/deletekeywordscommand.cpp \
    Models/deletekeywordsviewmodel.cpp \
    Models/artworksviewmodel.cpp \
    Helpers/keywordshelpers.cpp \
    Connectivity/uploadwatcher.cpp \
    Connectivity/telemetryworker.cpp \
    Warnings/warningssettingsmodel.cpp \
    Connectivity/simplecurlrequest.cpp \
    Connectivity/curlinithelper.cpp \
    Connectivity/simplecurldownloader.cpp \
    Helpers/updatehelpers.cpp \
    Common/basicmetadatamodel.cpp \
    KeywordsPresets/presetkeywordsmodel.cpp \
    KeywordsPresets/presetkeywordsmodelconfig.cpp \
    QMLExtensions/folderelement.cpp \
    Models/artworkproxymodel.cpp \
    Models/artworkproxybase.cpp \
    Translation/translationservice.cpp \
    Translation/translationmanager.cpp \
    Translation/translationworker.cpp \
    Translation/translationquery.cpp \
    Models/uimanager.cpp \
    Plugins/sandboxeddependencies.cpp \
    Commands/expandpresetcommand.cpp \
    QuickBuffer/currenteditableartwork.cpp \
    QuickBuffer/currenteditableproxyartwork.cpp \
    QuickBuffer/quickbuffer.cpp \
    SpellCheck/userdicteditmodel.cpp \
    Helpers/asynccoordinator.cpp \
    QMLExtensions/tabsmodel.cpp \
    Models/recentitemsmodel.cpp \
    Models/recentfilesmodel.cpp \
    Models/videoartwork.cpp \
    Maintenance/maintenanceworker.cpp \
    Maintenance/maintenanceservice.cpp \
    Maintenance/logscleanupjobitem.cpp \
    Maintenance/updatescleanupjobitem.cpp \
    Maintenance/launchexiftooljobitem.cpp \
    Maintenance/initializedictionariesjobitem.cpp \
    Maintenance/movesettingsjobitem.cpp \
    QMLExtensions/videocachingservice.cpp \
    QMLExtensions/videocachingworker.cpp \
    QMLExtensions/artworksupdatehub.cpp \
    Models/keyvaluelist.cpp \
    Helpers/filehelpers.cpp \
    Helpers/artworkshelpers.cpp \
    Models/sessionmanager.cpp \
    Maintenance/savesessionjobitem.cpp \
    Connectivity/switcherconfig.cpp \
    Models/switchermodel.cpp \
    Connectivity/requestsworker.cpp \
    Connectivity/requestsservice.cpp \
    Storage/database.cpp \
    Common/statefulentity.cpp \
    QMLExtensions/cachedimage.cpp \
    QMLExtensions/dbimagecacheindex.cpp \
    Maintenance/moveimagecachejobitem.cpp \
    QMLExtensions/cachedvideo.cpp \
    QMLExtensions/dbvideocacheindex.cpp \
    MetadataIO/cachedartwork.cpp \
    MetadataIO/metadatacache.cpp \
    MetadataIO/metadataioworker.cpp \
    MetadataIO/metadataioservice.cpp \
    MetadataIO/artworkssnapshot.cpp \
    MetadataIO/metadatareadinghub.cpp \
    AutoComplete/libfacecompletionengine.cpp \
    AutoComplete/autocompletemodel.cpp \
    AutoComplete/keywordsautocompletemodel.cpp \
    AutoComplete/stringsautocompletemodel.cpp \
    AutoComplete/presetscompletionengine.cpp \
    SpellCheck/duplicatesreviewmodel.cpp \
    SpellCheck/duplicateshighlighter.cpp \
    MetadataIO/csvexportworker.cpp \
    MetadataIO/csvexportplansmodel.cpp \
    MetadataIO/csvexportproperties.cpp \
    MetadataIO/csvexportmodel.cpp \
    Helpers/threadhelpers.cpp \
    KeywordsPresets/presetgroupsmodel.cpp \
    UndoRedo/removedirectoryitem.cpp \
    Common/basickeywordsmodelimpl.cpp \
    Maintenance/xpkscleanupjob.cpp \
    Commands/maindelegator.cpp \
    Common/baseentity.cpp \
    Warnings/warningsitem.cpp \
    Maintenance/updatebundlecleanupjobitem.cpp \
    Common/systemenvironment.cpp \
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
    Models/artitemsmodel.h \
    Models/artworkmetadata.h \
    Helpers/globalimageprovider.h \
    Models/artworksrepository.h \
    Helpers/indiceshelper.h \
    Helpers/clipboardhelper.h \
    Models/combinedartworksmodel.h \
    Common/abstractlistmodel.h \
    Helpers/constants.h \
    Models/artworkuploader.h \
    Models/uploadinfo.h \
    Models/exportinfo.h \
    Models/uploadinforepository.h \
    Models/logsmodel.h \
    Encryption/aes-qt.h \
    ../../vendors/tiny-aes/aes.h \
    ../../vendors/sqlite/sqlite3.h \
    Encryption/secretsmanager.h \
    Helpers/stringhelper.h \
    Helpers/logger.h \
    Commands/commandmanager.h \
    UndoRedo/historyitem.h \
    UndoRedo/undoredomanager.h \
    UndoRedo/addartworksitem.h \
    Commands/commandbase.h \
    Commands/addartworkscommand.h \
    Common/baseentity.h \
    Commands/removeartworkscommand.h \
    UndoRedo/removeartworksitem.h \
    UndoRedo/artworkmetadatabackup.h \
    UndoRedo/modifyartworkshistoryitem.h \
    Commands/combinededitcommand.h \
    Commands/pastekeywordscommand.h \
    Helpers/runguard.h \
    Models/ziparchiver.h \
    Helpers/ziphelper.h \
    Common/basickeywordsmodel.h \
    Suggestion/keywordssuggestor.h \
    Suggestion/suggestionartwork.h \
    Models/settingsmodel.h \
    Helpers/loggingworker.h \
    Common/defines.h \
    Models/filteredartitemsproxymodel.h \
    Common/flags.h \
    Helpers/helpersqmlwrapper.h \
    Models/recentdirectoriesmodel.h \
    Common/version.h \
    Connectivity/updateservice.h \
    SpellCheck/spellcheckerservice.h \
    SpellCheck/spellcheckitem.h \
    SpellCheck/spellcheckworker.h \
    SpellCheck/spellchecksuggestionmodel.h \
    SpellCheck/spellcheckerrorshighlighter.h \
    SpellCheck/spellcheckiteminfo.h \
    Common/itemprocessingworker.h \
    SpellCheck/spellsuggestionsitem.h \
    Connectivity/analyticsuserevent.h \
    Connectivity/telemetryservice.h \
    Connectivity/updatescheckerworker.h \
    Warnings/warningscheckingworker.h \
    Warnings/warningsitem.h \
    MetadataIO/metadataiocoordinator.h \
    Connectivity/testconnection.h \
    Connectivity/ftphelpers.h \
    Plugins/xpiksplugininterface.h \
    Commands/icommandmanager.h \
    Commands/icommandbase.h \
    UndoRedo/ihistoryitem.h \
    UndoRedo/iundoredomanager.h \
    Plugins/pluginmanager.h \
    Plugins/pluginwrapper.h \
    Plugins/ipluginaction.h \
    Plugins/pluginactionsmodel.h \
    Plugins/uiprovider.h \
    Plugins/iuiprovider.h \
    Common/ibasicartwork.h \
    Common/iartworkssource.h \
    Warnings/warningsservice.h \
    Common/iservicebase.h \
    Helpers/loghighlighter.h \
    Warnings/warningsmodel.h \
    Models/languagesmodel.h \
    Helpers/filterhelpers.h \
    Connectivity/iftpcoordinator.h \
    QMLExtensions/triangleelement.h \
    Suggestion/locallibraryqueryengine.h \
    Helpers/ifilenotavailablemodel.h \
    QMLExtensions/colorsmodel.h \
    Helpers/remoteconfig.h \
    Helpers/localconfig.h \
    Helpers/jsonhelper.h \
    Helpers/comparevaluesjson.h \
    AutoComplete/autocompleteworker.h \
    AutoComplete/completionquery.h \
    AutoComplete/autocompleteservice.h \
    Models/abstractconfigupdatermodel.h \
    Microstocks/stocksftplistmodel.h \
    Models/imageartwork.h \
    Common/hold.h \
    Models/proxysettings.h \
    QMLExtensions/imagecachingworker.h \
    QMLExtensions/imagecacherequest.h \
    QMLExtensions/imagecachingservice.h \
    QMLExtensions/cachingimageprovider.h \
    Commands/findandreplacecommand.h \
    Helpers/metadatahighlighter.h \
    Models/findandreplacemodel.h \
    Commands/deletekeywordscommand.h \
    Models/deletekeywordsviewmodel.h \
    Models/artworksviewmodel.h \
    Helpers/keywordshelpers.h \
    Connectivity/uploadwatcher.h \
    Common/iflagsprovider.h \
    Connectivity/telemetryworker.h \
    Warnings/warningssettingsmodel.h \
    Connectivity/simplecurlrequest.h \
    Connectivity/curlinithelper.h \
    Connectivity/simplecurldownloader.h \
    Connectivity/apimanager.h \
    Helpers/updatehelpers.h \
    Common/basicmetadatamodel.h \
    KeywordsPresets/presetkeywordsmodel.h \
    KeywordsPresets/presetkeywordsmodelconfig.h \
    QMLExtensions/folderelement.h \
    Models/artworkproxymodel.h \
    Models/artworkproxybase.h \
    Common/imetadataoperator.h \
    Translation/translationservice.h \
    Translation/translationmanager.h \
    Translation/translationworker.h \
    Translation/translationquery.h \
    Models/uimanager.h \
    Plugins/sandboxeddependencies.h \
    Commands/expandpresetcommand.h \
    QuickBuffer/icurrenteditable.h \
    QuickBuffer/currenteditableartwork.h \
    QuickBuffer/currenteditableproxyartwork.h \
    QuickBuffer/quickbuffer.h \
    KeywordsPresets/ipresetsmanager.h \
    SpellCheck/userdicteditmodel.h \
    Helpers/asynccoordinator.h \
    QMLExtensions/tabsmodel.h \
    Models/recentitemsmodel.h \
    Models/recentfilesmodel.h \
    Models/videoartwork.h \
    Maintenance/maintenanceworker.h \
    Maintenance/maintenanceservice.h \
    Maintenance/imaintenanceitem.h \
    Maintenance/logscleanupjobitem.h \
    Maintenance/updatescleanupjobitem.h \
    Maintenance/launchexiftooljobitem.h \
    Maintenance/initializedictionariesjobitem.h \
    Maintenance/movesettingsjobitem.h \
    QMLExtensions/videocachingservice.h \
    QMLExtensions/videocachingworker.h \
    QMLExtensions/videocacherequest.h \
    QMLExtensions/artworksupdatehub.h \
    QMLExtensions/artworkupdaterequest.h \
    Models/keyvaluelist.h \
    Helpers/filehelpers.h \
    Helpers/artworkshelpers.h \
    Models/sessionmanager.h \
    Maintenance/savesessionjobitem.h \
    Connectivity/switcherconfig.h \
    Models/switchermodel.h \
    Connectivity/requestsworker.h \
    Connectivity/requestsservice.h \
    Warnings/iwarningsitem.h \
    AutoComplete/completionitem.h \
    Storage/database.h \
    AutoComplete/completionitem.h \
    Common/statefulentity.h \
    QMLExtensions/previewstorage.h \
    QMLExtensions/cachedimage.h \
    QMLExtensions/dbimagecacheindex.h \
    Maintenance/moveimagecachejobitem.h \
    QMLExtensions/dbcacheindex.h \
    QMLExtensions/cachedvideo.h \
    QMLExtensions/dbvideocacheindex.h \
    MetadataIO/cachedartwork.h \
    MetadataIO/metadatacache.h \
    Common/readerwriterqueue.h \
    MetadataIO/metadataioworker.h \
    MetadataIO/metadataiotask.h \
    MetadataIO/metadataioservice.h \
    MetadataIO/originalmetadata.h \
    Microstocks/searchquery.h \
    Suggestion/locallibraryquery.h \
    MetadataIO/artworkssnapshot.h \
    MetadataIO/metadatareadinghub.h \
    AutoComplete/completionenginebase.h \
    AutoComplete/libfacecompletionengine.h \
    Common/wordanalysisresult.h \
    AutoComplete/autocompletemodel.h \
    AutoComplete/keywordsautocompletemodel.h \
    AutoComplete/stringsautocompletemodel.h \
    AutoComplete/presetscompletionengine.h \
    Common/keyword.h \
    SpellCheck/duplicatesreviewmodel.h \
    SpellCheck/duplicateshighlighter.h \
    MetadataIO/csvexportworker.h \
    MetadataIO/csvexportproperties.h \
    MetadataIO/csvexportplansmodel.h \
    MetadataIO/csvexportmodel.h \
    Common/delayedactionentity.h \
    Models/artworkelement.h \
    Models/previewartworkelement.h \
    Helpers/threadhelpers.h \
    KeywordsPresets/presetgroupsmodel.h \
    UndoRedo/removedirectoryitem.h \
    Common/basickeywordsmodelimpl.h \
    Maintenance/xpkscleanupjob.h \
    Commands/maindelegator.h \
    KeywordsPresets/presetmodel.h \
    KeywordsPresets/groupmodel.h \
    Warnings/iwarningssettings.h \
    Maintenance/updatebundlecleanupjobitem.h \
    Common/systemenvironment.h \
    Common/isystemenvironment.h \
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
    Microstocks/microstockapiclients.h \
    Suggestion/shutterstocksuggestionengine.h \
    Suggestion/fotoliasuggestionengine.h \
    Suggestion/gettysuggestionengine.h \
    Microstocks/imicrostockservice.h \
    Microstocks/microstockservice.h \
    Microstocks/imicrostockservices.h \
    Encryption/secretpair.h \
    Encryption/isecretsstorage.h \
    Microstocks/apisecrets.h \
    Storage/memorytable.h \
    xpiksapp.h \
    Microstocks/stockftpoptions.h

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

INCLUDEPATH += ../../vendors/tiny-aes
INCLUDEPATH += ../../vendors/cpp-libface
INCLUDEPATH += ../../vendors/ssdll/src/ssdll
INCLUDEPATH += ../../vendors/hunspell-repo/src
INCLUDEPATH += ../../vendors/libthmbnlr
INCLUDEPATH += ../../vendors/libxpks

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

BUILDNO=$$system(git log -n 1 --pretty=format:"%h")
BRANCH_NAME=$$system(git rev-parse --abbrev-ref HEAD)

include(../xpiks-common/xpiks-common.pri)

DEFINES += WITH_UPDATES

CONFIG(debug, debug|release)  {
    message("Building debug")
    DEFINES += WITH_PLUGINS
    DEFINES += WITH_LOGS
    DEFINES += WITH_STDOUT_LOGS
    #QMAKE_CXXFLAGS += -fsanitize=thread
} else {
    DEFINES += WITH_LOGS
    message("Building release")
}

macx {
    QMAKE_TARGET_BUNDLE_PREFIX = com.xpiksapp
    QMAKE_BUNDLE = Xpiks

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

    TERMS_AND_CONDITIONS.files = deps/terms_and_conditions.txt
    TERMS_AND_CONDITIONS.path = Contents/Resources
    QMAKE_BUNDLE_DATA += TERMS_AND_CONDITIONS

    TRANSLATIONS_FILES_LIST = $$system(ls $$PWD/deps/translations/*.qm)
    XPIKS_TRANSLATIONS.files = $$TRANSLATIONS_FILES_LIST
    XPIKS_TRANSLATIONS.path = Contents/Resources/translations
    QMAKE_BUNDLE_DATA += XPIKS_TRANSLATIONS

    FREQ_TABLES.files = deps/ac_sources/en_wordlist.tsv
    FREQ_TABLES.path = Contents/Resources/ac_sources
    QMAKE_BUNDLE_DATA += FREQ_TABLES
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

    # SetProcessDPIAware API
    LIBS += -lShcore
}

linux {
    message("for Linux")
    INCLUDEPATH += "../../vendors/quazip"
    BUILDNO = $$system($$PWD/buildno.sh)

    LIBS += -ldl

    LIBS += -lavcodec
    LIBS += -lavfilter
    LIBS += -lavformat
    LIBS += -lavutil
    LIBS += -lswscale

    #QML_IMPORT_PATH += /usr/lib/x86_64-linux-gnu/qt5/imports/
    #LIBS += -L/lib/x86_64-linux-gnu/
}

appveyor {
    DEFINES += WITH_LOGS
}

travis-ci {
    message("for Travis CI")
    LIBS -= -lz
    LIBS += /usr/lib/x86_64-linux-gnu/libz.so
    LIBS += -ldl
    DEFINES += TRAVIS_CI
    INCLUDEPATH += "../../vendors/quazip"
}

linux-qtcreator {
    message("in QtCreator")
    BUILDNO = $$system($$PWD/buildno.sh)
    LIBS += -L/usr/lib64/
    LIBS += -L/lib/x86_64-linux-gnu/
    copywhatsnew.commands = $(COPY_FILE) "$$PWD/deps/whatsnew.txt" "$$OUT_PWD/"
    copyterms.commands = $(COPY_FILE) "$$PWD/deps/terms_and_conditions.txt" "$$OUT_PWD/"
    QMAKE_EXTRA_TARGETS += copywhatsnew copyterms
    POST_TARGETDEPS += copywhatsnew copyterms
}

linux-static {
    CONFIG += static
    QTPLUGIN += qt5quick
    DEFINES += STATIC
    message("Static build.")
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
