/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.2
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0
import xpiks 1.0
import "Constants" 1.0
import "Components"
import "StyledControls"
import "Dialogs"
import "StackViews"
import "Common.js" as Common
import "Constants/UIConfig.js" as UIConfig

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: uiManager.getAppWidth(1010)
    height: uiManager.getAppHeight(725)
    minimumHeight: 725
    minimumWidth: 930
    title: i18.n + (debug ? "Xpiks (Devel)" : qsTr("Xpiks"))
    property int openedDialogsCount: 0
    property bool showUpdateLink: false
    property bool needToCenter: true
    property bool listLayout: true
    property var spellCheckService: helpersWrapper.getSpellCheckerService()
    property bool leftSideCollapsed: false
    property bool actionsEnabled: mainStackView.areActionsAllowed && (openedDialogsCount == 0)

    onVisibleChanged: {
        if (needToCenter) {
            needToCenter = false
            applicationWindow.x = uiManager.getAppPosX((Screen.width - applicationWindow.width) / 2)
            applicationWindow.y = uiManager.getAppPosY((Screen.height - applicationWindow.height) / 2)
        }
    }

    function closeHandler(close) {
        console.info("closeHandler")

        if (artItemsModel.hasModifiedArtworks()) {
            console.debug("Modified artworks present")
            close.accepted = false
            configExitDialog.open()
        } else {
            console.debug("No modified artworks found. Exiting...")
            shutdownEverything()
            close.accepted = false
        }
    }

    function shutdownEverything() {
        applicationWindow.visibility = "Minimized"
        helpersWrapper.beforeDestruction();
        saveAppGeometry()
        closingTimer.start()
    }

    function saveAppGeometry() {
        console.debug("Saving application geometry")
        uiManager.setAppWidth(applicationWindow.width)
        uiManager.setAppHeight(applicationWindow.height)
        uiManager.setAppPosX(applicationWindow.x)
        uiManager.setAppPosY(applicationWindow.y)
        uiManager.sync()
    }

    onClosing: closeHandler(close)

    Timer {
        id: closingTimer
        interval: 1000
        running: false
        repeat: false
        onTriggered: Qt.quit()
    }

    Timer {
        id: openingTimer
        interval: 500
        running: false
        repeat: false
        onTriggered: {
            console.debug("Delayed onOpen timer triggered");
            helpersWrapper.setProgressIndicator(applicationWindow)

            if (settingsModel.needToShowWhatsNew()) {
                Common.launchDialog("Dialogs/WhatsNewDialog.qml", applicationWindow, {})
            } else if (settingsModel.needToShowTextWhatsNew()) {
                var text = settingsModel.whatsNewText;
                if (text.length > 0) {
                    Common.launchDialog("Dialogs/WhatsNewMinorDialog.qml",
                                        applicationWindow,
                                        {
                                            whatsNewText: text
                                        })
                }
            }

            if (settingsModel.needToShowTermsAndConditions()) {
                var licenseText = settingsModel.termsAndConditionsText;
                if (licenseText.length > 0) {
                    Common.launchDialog("Dialogs/TermsAndConditionsDialog.qml",
                                        applicationWindow,
                                        {
                                            termsText: licenseText
                                        })
                } else {
                    licenseMissingDialog.open()
                }
            } else {
                helpersWrapper.reportOpen()
            }
        }
    }

    function onDialogClosed() {
        openedDialogsCount -= 1
        console.debug("Dialog closed. Opened dialogs count is " + openedDialogsCount)
    }

    function mustUseConfirmation() {
        return settingsModel.mustUseConfirmations
    }

    function openUploadDialog(skipUploadItems) {
        if (settingsModel.mustUseMasterPassword) {
            var callbackObject = {
                onSuccess: function() { doOpenUploadDialog(true, skipUploadItems) },
                onFail: function() { doOpenUploadDialog(false, skipUploadItems) }
            }

            Common.launchDialog("Dialogs/EnterMasterPasswordDialog.qml",
                                applicationWindow,
                                {callbackObject: callbackObject})
        } else {
            doOpenUploadDialog(true, skipUploadItems)
        }
    }

    function doOpenUploadDialog(masterPasswordCorrectOrEmpty, skipUploadItems) {
        var artworkUploader = helpersWrapper.getArtworkUploader()
        artworkUploader.clearModel()

        if (!skipUploadItems) {
            filteredArtItemsModel.setSelectedForUpload()
            warningsModel.setShowSelected()
        }

        var uploadInfos = helpersWrapper.getUploadInfos();
        uploadInfos.initializeAccounts(masterPasswordCorrectOrEmpty)
        Common.launchDialog("Dialogs/UploadArtworks.qml",
                            applicationWindow,
                            {
                                componentParent: applicationWindow,
                                skipUploadItems: skipUploadItems
                            })
    }

    function openFindAndReplaceDialog() {
        Common.launchDialog("Dialogs/FindAndReplace.qml", applicationWindow, { componentParent: applicationWindow })
    }

    function openDeleteKeywordsDialog() {
        Common.launchDialog("Dialogs/DeleteKeywordsDialog.qml", applicationWindow, { componentParent: applicationWindow })
    }

    function collapseLeftPane() {
        leftDockingGroup.state = "collapsed"
        applicationWindow.leftSideCollapsed = true
    }

    function expandLeftPane() {
        leftDockingGroup.state = ""
        applicationWindow.leftSideCollapsed = false
    }

    function toggleLeftPane() {
        leftDockingGroup.state = (leftDockingGroup.state == "collapsed") ? "" : "collapsed"
        applicationWindow.leftSideCollapsed = !leftSideCollapsed
    }

    function startOneItemEditing(metadata, index, originalIndex, showInfoFirst) {
        var keywordsModel = filteredArtItemsModel.getBasicModel(index)
        artworkProxy.setSourceArtwork(metadata)
        var wasCollapsed = applicationWindow.leftSideCollapsed
        applicationWindow.collapseLeftPane()
        mainStackView.push({
                               item: "qrc:/StackViews/ArtworkEditView.qml",
                               properties: {
                                   artworkIndex: index,
                                   keywordsModel: keywordsModel,
                                   componentParent: applicationWindow,
                                   wasLeftSideCollapsed: wasCollapsed,
                                   showInfo: showInfoFirst
                               },
                               destroyOnPop: true
                           })
    }

    function launchImportDialog(importID, imagesCount, vectorsCount, reimport) {
        var latestDir = recentDirectories.getLatestItem()
        chooseArtworksDialog.folder = latestDir
        chooseDirectoryDialog.folder = latestDir

        if (imagesCount > 0) {
            if (!metadataIOCoordinator.hasImportFinished(importID)) {
                Common.launchDialog("Dialogs/ImportMetadata.qml", applicationWindow,
                                    {
                                        importID: importID,
                                        backupsEnabled: !reimport,
                                        reimport: reimport
                                    })
            } else {
                console.debug("UI::main # Import seems to be finished already")
            }
        } else if (vectorsCount > 0) {
            vectorsAttachedDialog.vectorsAttached = vectorsCount
            vectorsAttachedDialog.open()
        }
    }

    Component.onCompleted: {
        console.debug("onCompleted handler")
        openingTimer.start()
    }

    Action {
        id: openSettingsAction
        text: i18.n + qsTr("&Settings")
        shortcut: StandardKey.Preferences
        onTriggered: {
            Common.launchDialog("Dialogs/SettingsWindow.qml",
                                applicationWindow, {},
                                function(wnd) {wnd.show();});
        }
    }

    Action {
        id: upgradeAction
        text: i18.n + qsTr("&Upgrade Now!")
        enabled: helpersWrapper.isUpdateDownloaded && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            helpersWrapper.setUpgradeConsent()
            closeHandler({accepted: false})
        }
    }

    Action {
        id: quitAction
        text: i18.n + qsTr("Exit")
        shortcut: StandardKey.Quit
        onTriggered: closeHandler({accepted: false});
    }

    Action {
        id: editAction
        shortcut: "Ctrl+E"
        enabled: (artworkRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtItemsModel.selectedArtworksCount === 0) {
                mustSelectDialog.open()
            }
            else {
                var launched = false
                var index = filteredArtItemsModel.findSelectedItemIndex()

                if (index !== -1) {
                    var originalIndex = filteredArtItemsModel.getOriginalIndex(index)
                    var metadata = filteredArtItemsModel.getArtworkMetadata(index)
                    startOneItemEditing(metadata, index, originalIndex)
                    launched = true
                }

                if (!launched) {
                    // also as fallback in case of errors in findSelectedIndex
                    filteredArtItemsModel.combineSelectedArtworks();
                    mainStackView.push({
                                           item: "qrc:/StackViews/CombinedEditView.qml",
                                           properties: {
                                               componentParent: applicationWindow
                                           },
                                           destroyOnPop: true
                                       })
                }
            }
        }
    }

    Action {
        id: saveAction
        shortcut: StandardKey.Save
        enabled: (artworkRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtItemsModel.selectedArtworksCount == 0) {
                mustSelectDialog.open()
            } else {
                var modifiedSelectedCount = filteredArtItemsModel.getModifiedSelectedCount();

                if (filteredArtItemsModel.selectedArtworksCount > 0 && modifiedSelectedCount > 0) {
                    Common.launchDialog("Dialogs/ExportMetadata.qml", applicationWindow, {})
                } else {
                    if (modifiedSelectedCount === 0) {
                        alreadySavedDialog.open()
                    }
                }
            }
        }
    }

    Action {
        id: searchAndReplaceAction
        shortcut: "Shift+Ctrl+F"
        onTriggered: openFindAndReplaceDialog()
        enabled: (artworkRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
    }

    Action {
        id: removeAction
        shortcut: "Ctrl+Del"
        enabled: (artworkRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtItemsModel.selectedArtworksCount === 0) {
                mustSelectDialog.open()
            } else {
                var itemsCount = filteredArtItemsModel.selectedArtworksCount
                if (itemsCount > 0) {
                    if (mustUseConfirmation()) {
                        confirmRemoveSelectedDialog.itemsCount = itemsCount
                        confirmRemoveSelectedDialog.open()
                    } else {
                        doRemoveSelectedArtworks()
                    }
                }
            }
        }
    }

    Action {
        id: uploadAction
        shortcut: "Shift+Ctrl+U"
        enabled: (artworkRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtItemsModel.selectedArtworksCount === 0) {
                filteredArtItemsModel.selectFilteredArtworks();
            }

            if (filteredArtItemsModel.selectedArtworksCount > 0) {
                tryUploadArtworks();
            }
        }
    }

    Action {
        id: addFilesAction
        shortcut: StandardKey.Open
        onTriggered: chooseArtworksDialog.open()
        enabled: (applicationWindow.openedDialogsCount == 0)
    }

    Action {
        id: showLogsAction
        shortcut: "Shift+Ctrl+L"
        enabled: applicationWindow.openedDialogsCount == 0
        onTriggered: {
            var logsModel = helpersWrapper.getLogsModel()
            var allText = logsModel.getAllLogsText()
            Common.launchDialog("Dialogs/LogsDialog.qml",
                                applicationWindow,
                                {
                                    logText: allText
                                });
        }
    }

    Action {
        id: fixSpellingInSelectedAction
        text: i18.n + qsTr("&Fix spelling")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Fix spelling in selected triggered")
            filteredArtItemsModel.suggestCorrectionsForSelected()
            Common.launchDialog("Dialogs/SpellCheckSuggestionsDialog.qml",
                                applicationWindow,
                                {})
        }
    }

    Action {
        id: fixDuplicatesInSelectedAction
        text: i18.n + qsTr("&Show duplicates")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Fix duplicates in selected triggered")
            filteredArtItemsModel.reviewDuplicatesInSelected()

            var wasCollapsed = applicationWindow.leftSideCollapsed
            applicationWindow.collapseLeftPane()
            mainStackView.push({
                                   item: "qrc:/StackViews/DuplicatesReView.qml",
                                   properties: {
                                       componentParent: applicationWindow,
                                       wasLeftSideCollapsed: wasCollapsed
                                   },
                                   destroyOnPop: true
                               })
        }
    }

    Action {
        id: removeMetadataAction
        text: i18.n + qsTr("&Remove metadata")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Remove metadata from selected triggered")
            removeMetadataDialog.open()
        }
    }

    Action {
        id: deleteKeywordsAction
        text: i18.n + qsTr("&Delete keywords")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Delete keywords from selected triggered")
            filteredArtItemsModel.deleteKeywordsFromSelected()
            openDeleteKeywordsDialog()
        }
    }

    Action {
        id: detachVectorsAction
        text: i18.n + qsTr("&Detach vectors")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Detach vectors from selected triggered")
            filteredArtItemsModel.detachVectorFromSelected()
        }
    }

    Action {
        id: createArchivesAction
        text: i18.n + qsTr("&Create archives")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Zip archives triggered")

            filteredArtItemsModel.setSelectedForZipping()
            Common.launchDialog("Dialogs/ZipArtworksDialog.qml",
                                applicationWindow,
                                {});
        }
    }

    Action {
        id: exportToCsvAction
        text: i18.n + qsTr("&Export to CSV")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("CSV export triggered")

            filteredArtItemsModel.setSelectedForCsvExport()
            Common.launchDialog("Dialogs/CsvExportDialog.qml",
                                applicationWindow,
                                {});
        }
    }

    Action {
        id: reimportMetadataAction
        text: i18.n + qsTr("&Reimport metadata")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Reimport metadata triggered")
            reimportConfirmationDialog.open()
        }
    }

    Action {
        id: overwriteMetadataAction
        text: i18.n + qsTr("&Overwrite metadata")
        enabled: (filteredArtItemsModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Overwrite metadata triggered")
            Common.launchDialog("Dialogs/ExportMetadata.qml", applicationWindow, {overwriteAll: true})
        }
    }

    menuBar: MenuBar {
        Menu {
            title: i18.n + qsTr("&File")

            Menu {
                id: recentDirectoriesMenu
                title: i18.n + qsTr("&Recent directories")
                enabled: (applicationWindow.openedDialogsCount == 0) && (recentDirectories.count > 0)

                Instantiator {
                    model: recentDirectories
                    onObjectAdded: recentDirectoriesMenu.insertItem( index, object )
                    onObjectRemoved: recentDirectoriesMenu.removeItem( object )

                    delegate: MenuItem {
                        text: display
                        onTriggered: {
                            var filesAdded = artItemsModel.addRecentDirectory(display)
                            if (filesAdded === 0) {
                                noNewFilesDialog.open()
                            }
                        }
                    }
                }
            }

            Menu {
                id: recentFilesMenu
                title: i18.n + qsTr("&Recent files")
                enabled: (applicationWindow.openedDialogsCount == 0) && (recentFiles.count > 0)

                MenuItem {
                    text: i18.n + qsTr("Open all")

                    onTriggered: {
                        var filesAdded = artItemsModel.addAllRecentFiles()
                        if (filesAdded === 0) {
                            noNewFilesDialog.open()
                        }
                    }
                }

                MenuSeparator {
                    visible: recentFiles.count > 0
                }

                Instantiator {
                    model: recentFiles
                    onObjectAdded: recentFilesMenu.insertItem( index, object )
                    onObjectRemoved: recentFilesMenu.removeItem( object )

                    delegate: MenuItem {
                        text: display
                        onTriggered: {
                            var filesAdded = artItemsModel.addRecentFile(display)
                            if (filesAdded === 0) {
                                noNewFilesDialog.open()
                            }
                        }
                    }
                }
            }

            Menu {
                title: i18.n + qsTr("Session Manager")

                MenuItem {
                    id: saveSessionMenu
                    text: i18.n + qsTr("Restore last session on startup")
                    checkable: true

                    onCheckedChanged: {
                        settingsModel.updateSaveSession(checked);
                    }

                    Component.onCompleted: {
                        checked = settingsModel.saveSession
                    }
                }
            }

            MenuItem {
                action: upgradeAction
                visible: helpersWrapper.isUpdateDownloaded
            }

            MenuItem { action: openSettingsAction }

            MenuItem {
                text: i18.n + qsTr("&About")
                onTriggered: {
                    Common.launchDialog("Dialogs/AboutWindow.qml",
                                        applicationWindow, {},
                                        function(wnd) {wnd.show();});
                }
            }

            MenuItem { action: quitAction }
        }

        Menu {
            title: i18.n + qsTr("&Edit")

            MenuItem {
                text: i18.n + qsTr("&Presets")
                enabled: (applicationWindow.openedDialogsCount == 0)
                onTriggered: {
                    console.info("Presets triggered")
                    Common.launchDialog("Dialogs/PresetsEditDialog.qml", applicationWindow,
                                        {
                                            componentParent: applicationWindow
                                        })
                }
            }

            MenuItem {
                text: i18.n + qsTr("&Invert selection")
                enabled: (artworkRepository.artworksSourcesCount > 0) && applicationWindow.actionsEnabled
                onTriggered: {
                    console.info("Invert selection triggered")
                    if (filteredArtItemsModel.getItemsCount() > 0) {
                        filteredArtItemsModel.invertSelectionArtworks()
                    }
                }
            }

            MenuItem {
                text: i18.n + qsTr("&Sort by filename")
                enabled: (artworkRepository.artworksSourcesCount > 0) && applicationWindow.actionsEnabled
                checkable: true
                onToggled: {
                    console.info("Sort by filename")
                    if (filteredArtItemsModel.getItemsCount() > 0) {
                        filteredArtItemsModel.toggleSorted();
                    }
                }
            }

            MenuItem {
                text: i18.n + qsTr("&Find and replace")
                action: searchAndReplaceAction
            }

            Menu {
                title: i18.n + qsTr("&Selected artworks")

                MenuItem { action: fixSpellingInSelectedAction }
                MenuItem { action: fixDuplicatesInSelectedAction }

                MenuSeparator {}

                MenuItem { action: removeMetadataAction }
                MenuItem { action: deleteKeywordsAction }
                MenuItem { action: detachVectorsAction }

                MenuSeparator {}

                MenuItem { action: createArchivesAction }
                MenuItem { action: exportToCsvAction }

                MenuSeparator {}

                MenuItem { action: reimportMetadataAction }
                MenuItem { action: overwriteMetadataAction }
            }
        }

        Menu {
            title: i18.n + qsTr("&Tools")
            enabled: (applicationWindow.openedDialogsCount == 0) || debug

            Menu {
                title: i18.n + qsTr("&Change language")
                id: languagesMenu

                Instantiator {
                    model: languagesModel
                    onObjectAdded: languagesMenu.insertItem( index, object )
                    onObjectRemoved: languagesMenu.removeItem( object )

                    delegate: MenuItem {
                        checkable: true
                        checked: iscurrent
                        text: display
                        onTriggered: {
                            languagesModel.switchLanguage(index)
                        }
                    }
                }
            }

            Menu {
                title: i18.n + qsTr("&Plugins")
                id: pluginsMenu
                enabled: helpersWrapper.pluginsAvailable
                visible: helpersWrapper.pluginsAvailable

                Instantiator {
                    model: pluginsWithActions
                    onObjectAdded: pluginsMenu.insertItem( index, object )
                    onObjectRemoved: pluginsMenu.removeItem( object )

                    delegate: Menu {
                        id: pluginActionsMenu
                        title: model.prettyname
                        enabled: model.enabled
                        property int originalIndex: pluginsWithActions.getOriginalIndex(index)
                        property var actionsModel: pluginManager.getPluginActions(originalIndex)

                        Instantiator {
                            model: actionsModel
                            onObjectAdded: pluginActionsMenu.insertItem( index, object )
                            onObjectRemoved: pluginActionsMenu.removeItem( object )

                            delegate: MenuItem {
                                text: aname
                                onTriggered: {
                                    pluginManager.triggerPluginAction(pluginID, acode)
                                }
                            }
                        }
                    }
                }

                MenuSeparator {
                    visible: pluginsMenu.items.length > 2
                }

                MenuItem {
                    text: i18.n + qsTr("&Plugin manager")
                    onTriggered: {
                        Common.launchDialog("Dialogs/PluginsDialog.qml",
                                            applicationWindow, {});
                    }
                }
            }

            MenuItem {
                text: i18.n + qsTr("&Manage upload hosts")
                enabled: (applicationWindow.openedDialogsCount == 0)
                onTriggered: {
                    console.info("Manage upload hosts triggered")
                    openUploadDialog(true)
                }
            }

            Menu {
                title: i18.n + qsTr("&Advanced")

                MenuItem {
                    text: i18.n + qsTr("Manage &user dictionary")
                    onTriggered: {
                        console.info("User dictionary triggered")
                        userDictEditModel.initializeModel()
                        Common.launchDialog("Dialogs/UserDictEditDialog.qml", applicationWindow, {})
                    }
                }

                MenuItem {
                    text: i18.n + qsTr("&Wipe all metadata from files")
                    onTriggered: {
                        console.info("Wipe metadata triggered")
                        if (filteredArtItemsModel.selectedArtworksCount == 0) {
                            mustSelectDialog.open()
                        } else if (filteredArtItemsModel.selectedArtworksCount > 0) {
                            Common.launchDialog("Dialogs/WipeMetadata.qml", applicationWindow, {})
                        }
                    }
                }

                MenuItem {
                    text: i18.n + qsTr("Show logs")
                    action: showLogsAction
                }
            }
        }

        Menu {
            title: i18.n + qsTr("&Help")

            MenuItem {
                text: i18.n + qsTr("&User's guide")
                onTriggered: Qt.openUrlExternally("http://xpiksapp.com/tutorials/")
            }

            MenuItem {
                text: i18.n + qsTr("&FAQ")
                onTriggered: Qt.openUrlExternally("http://xpiksapp.com/faq/")
            }

            MenuItem {
                text: i18.n + qsTr("&Support")
                onTriggered: Qt.openUrlExternally("http://xpiksapp.com/support/")
            }
        }

        Menu {
            title: "Debug"
            visible: debug
            enabled: debug

            MenuItem {
                text: "Update all items"
                onTriggered: {
                    artItemsModel.updateAllItems()
                }
            }

            MenuItem {
                text: "Keywords Suggestion"
                onTriggered: {
                    var callbackObject = {
                        promoteKeywords: function(keywords) {
                            // bump
                        }
                    }

                    Common.launchDialog("Dialogs/KeywordsSuggestion.qml",
                                        applicationWindow,
                                        {callbackObject: callbackObject});
                }
            }

            MenuItem {
                text: "Update window"
                onTriggered: {
                    Common.launchDialog("Dialogs/UpdateWindow.qml",
                                        applicationWindow, {updateUrl: "http://xpiksapp.com/downloads/"},
                                        function(wnd) {wnd.show();});
                }
            }

            MenuItem {
                text: "What's new dialog"
                onTriggered: {
                    Common.launchDialog("Dialogs/WhatsNewDialog.qml", applicationWindow, {})
                }
            }

            MenuItem {
                text: "Legacy what's new dialog"
                onTriggered: {
                    var text = settingsModel.whatsNewText;
                    if (text.length > 0) {
                        Common.launchDialog("Dialogs/WhatsNewMinorDialog.qml",
                                            applicationWindow,
                                            {
                                                whatsNewText: text
                                            })
                    }
                }
            }

            MenuItem {
                text: "Terms and Conditions"
                onTriggered: {
                    var licenseText = settingsModel.termsAndConditionsText;
                    if (licenseText.length > 0) {
                        Common.launchDialog("Dialogs/TermsAndConditionsDialog.qml",
                                            applicationWindow,
                                            {
                                                termsText: licenseText
                                            })
                    }
                }
            }

            MenuItem {
                text: "Master password"
                onTriggered: {
                    var callbackObject = {
                        onSuccess: function() { },
                        onFail: function() { }
                    }

                    Common.launchDialog("Dialogs/EnterMasterPasswordDialog.qml",
                                        applicationWindow,
                                        {callbackObject: callbackObject})
                }
            }

            MenuItem {
                text: "Master password setup"
                onTriggered: {
                    var callbackObject = {
                        onCancel: function() { },
                        onSuccess: function() { }
                    }

                    Common.launchDialog("Dialogs/MasterPasswordSetupDialog.qml",
                                        applicationWindow,
                                        {
                                            componentParent: applicationWindow,
                                            firstTime: false,
                                            callbackObject: callbackObject
                                        });
                }
            }

            MenuItem {
                text: "Install update"
                onTriggered: {
                    Common.launchDialog("Dialogs/InstallUpdateDialog.qml", applicationWindow, {})
                }
            }

            MenuItem {
                text: "Donate dialog"
                onTriggered: {
                    Common.launchDialog("Dialogs/DonateDialog.qml", applicationWindow, {})
                }
            }

            MenuItem {
                text: "CSV export"
                onTriggered: {
                    Common.launchDialog("Dialogs/CsvExportDialog.qml", applicationWindow, {})
                }
            }

            MenuItem {
                text: "Translate dialog"
                onTriggered: {
                    Common.launchDialog("Dialogs/TranslationPreviewDialog.qml", applicationWindow, {})
                }
            }

            MenuItem {
                text: "Failed uploads dialog"
                onTriggered: {
                    Common.launchDialog("Dialogs/FailedUploadArtworks.qml", applicationWindow, {})
                }
            }

            MenuItem {
                text: "Proxy setup dialog"
                onTriggered: {
                    var callbackObject = {
                        onSuccess: function() {},
                        onCancel: function() {}
                    }
                    Common.launchDialog("Dialogs/ProxySetupDialog.qml",
                                        applicationWindow,
                                        {
                                            componentParent: applicationWindow,
                                            firstTime: false,
                                            callbackObject: callbackObject
                                        });
                }
            }
        }
    }

    Menu {
        id: artworkContextMenu
        property string filename
        property int index
        property bool hasVectorAttached

        MenuItem {
            text: i18.n + qsTr("Edit")
            onTriggered: {
                var originalIndex = filteredArtItemsModel.getOriginalIndex(artworkContextMenu.index)
                var metadata = filteredArtItemsModel.getArtworkMetadata(artworkContextMenu.index)
                startOneItemEditing(metadata, artworkContextMenu.index, originalIndex)
            }
        }

        MenuItem {
            text: i18.n + qsTr("Show info")
            onTriggered: {
                var originalIndex = filteredArtItemsModel.getOriginalIndex(artworkContextMenu.index)
                var metadata = filteredArtItemsModel.getArtworkMetadata(artworkContextMenu.index)
                startOneItemEditing(metadata, artworkContextMenu.index, originalIndex, true)
            }
        }

        MenuItem {
            text: i18.n + qsTr("Detach vector")
            enabled: artworkContextMenu.hasVectorAttached
            visible: artworkContextMenu.hasVectorAttached
            onTriggered: {
                filteredArtItemsModel.detachVectorFromArtwork(artworkContextMenu.index)
            }
        }

        MenuItem {
            text: i18.n + qsTr("Copy to Quick Buffer")
            onTriggered: {
                filteredArtItemsModel.copyToQuickBuffer(artworkContextMenu.index)
                uiManager.activateQuickBufferTab()
            }
        }

        MenuItem {
            text: i18.n + qsTr("Fill from Quick Buffer")
            onTriggered: filteredArtItemsModel.fillFromQuickBuffer(artworkContextMenu.index)
        }

        MenuItem {
            text: i18.n + qsTr("Show in folder")
            onTriggered: helpersWrapper.revealArtworkFile(artworkContextMenu.filename);
        }
    }

    MessageDialog {
        id: licenseMissingDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Xpiks installation is corrupted.\nPlease reinstall Xpiks and try again.")
        onAccepted: shutdownEverything()
    }

    MessageDialog {
        id: configExitDialog
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("You have some artworks modified. Really exit?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: shutdownEverything()
    }

    MessageDialog {
        id: unavailableArtworksDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Some files are not available anymore.\nThey will be removed from the workflow.")
        standardButtons: StandardButton.Ok
        onAccepted: {
            helpersWrapper.removeUnavailableFiles()
        }
    }

    MessageDialog {
        id: unavailableVectorsDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Some vectors are not available anymore.\nThey will be detached automatically.")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: removeMetadataDialog

        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Remove metadata from selected artworks?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            filteredArtItemsModel.removeMetadataInSelected()
        }
    }

    MessageDialog {
        id: confirmRemoveSelectedDialog
        property int itemsCount
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Are you sure you want to remove %1 item(s)?").arg(itemsCount)
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            doRemoveSelectedArtworks()
        }
    }

    function doRemoveSelectedArtworks() {
        filteredArtItemsModel.removeSelectedArtworks()
    }

    function tryUploadArtworks() {
        if (filteredArtItemsModel.areSelectedArtworksSaved()) {
            openUploadDialog(false)
        } else {
            mustSaveWarning.open()
        }
    }

    MessageDialog {
        id: reimportConfirmationDialog
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("You will lose all unsaved changes after reimport. Proceed?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            filteredArtItemsModel.reimportMetadataForSelected()
        }
    }

    MessageDialog {
        id: confirmRemoveDirectoryDialog
        property int directoryIndex
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Are you sure you want to remove this directory?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            filteredArtItemsModel.removeArtworksDirectory(directoryIndex)
        }
    }

    FileDialog {
        id: chooseArtworksDialog
        title: "Please choose artworks"
        selectExisting: true
        selectMultiple: true
        folder: shortcuts.pictures
        nameFilters: [ "All Xpiks files (*.jpg *.tiff *.tif *.eps *.ai *.mov *.mp4 *.avi *.mpeg *.qt *.vob *.wmv *.asf *.asx *.flv)",
            "Image files (*.jpg *.tiff *.tif *.eps *.ai)",
            "Video files (*.mov *.mp4 *.avi *.mpeg *.qt *.vob *.wmv *.asf *.asx *.flv)",
            "All files (*)" ]

        onAccepted: {
            console.debug("You chose: " + chooseArtworksDialog.fileUrls)
            var filesAdded = artItemsModel.addLocalArtworks(chooseArtworksDialog.fileUrls)
            if (filesAdded > 0) {
                console.debug("" + filesAdded + ' files via Open File(s)')
            } else {
                noNewFilesDialog.open()
            }
        }

        onRejected: {
            console.debug("Open files dialog canceled")
        }
    }

    FileDialog {
        id: chooseDirectoryDialog
        title: "Please choose artworks"
        selectExisting: true
        selectMultiple: false
        selectFolder: true
        folder: shortcuts.pictures

        onAccepted: {
            console.debug("You chose: " + chooseDirectoryDialog.fileUrls)
            var filesAdded = artItemsModel.addLocalDirectories(chooseDirectoryDialog.fileUrls)
            console.debug("" + filesAdded + ' files via Open Directory')
            if (filesAdded === 0) {
                noNewFilesDialog.open()
            }
        }

        onRejected: {
            console.debug("Directory dialog canceled")
        }
    }

    FileDialog {
        id: openDictionaryDialog
        title: "Select any dictionary file"
        selectExisting: true
        selectMultiple: false
        folder: shortcuts.home
        nameFilters: [ "StarDict files (*.ifo *.idx *.dict *.idx.dz *.dict.dz)"]

        onAccepted: {
            console.debug("You chose: " + openDictionaryDialog.fileUrl)
            var added = translationManager.addDictionary(openDictionaryDialog.fileUrl)
            if (added) {
                dictionaryAddedMB.open()
            }
        }

        onRejected: {
            console.debug("Add dictionary dialog canceled")
        }
    }

    MessageDialog {
        id: dictionaryAddedMB
        title: i18.n + qsTr("Info")
        text: i18.n + qsTr("Dictionary has been added")
    }

    MessageDialog {
        id: mustSaveWarning
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please, save selected items before upload")
    }

    MessageDialog {
        id: mustSelectDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please, select some artworks first")
    }

    MessageDialog {
        id: noNewFilesDialog
        title: i18.n + qsTr("Information")
        text: i18.n + qsTr("No new files were added")
    }

    MessageDialog {
        id: alreadySavedDialog
        title: i18.n + qsTr("Information")
        text: i18.n + qsTr("All selected files are already saved or read-only")
    }

    MessageDialog {
        id: vectorsAttachedDialog
        title: i18.n + qsTr("Information")
        property int vectorsAttached: 0
        property string originalText: vectorsAttached > 1 ? qsTr("%1 vectors attached").arg(vectorsAttached) : qsTr("1 vector attached")
        text: i18.n + originalText
    }

    Connections {
        target: settingsModel
        onSaveSessionChanged: {
            saveSessionMenu.checked = settingsModel.saveSession
        }
    }

    Connections {
        target: switcher
        onDonateDialogRequested: {
            console.debug("Donate dialog requested")
            donateTimer.start()
        }
    }

    Connections {
        target: artItemsModel

        onUnavailableArtworksFound: {
            console.debug("UI:onUnavailableArtworksFound")
            unavailableArtworksDialog.open()
        }

        onUnavailableVectorsFound: {
            console.debug("UI:onUnavailableVectorsFound")
            unavailableVectorsDialog.open()
        }

        onArtworksAdded: {
            if ((imagesCount === 0) && (vectorsCount === 0)) {
                noNewFilesDialog.open();
                return;
            }

            launchImportDialog(importID, imagesCount, vectorsCount, false)
        }

        onArtworksReimported: {
            launchImportDialog(importID, artworksCount, 0, true)
        }
    }

    Connections {
        target: helpersWrapper
        onUpdateAvailable: {
            Common.launchDialog("Dialogs/UpdateWindow.qml",
                                applicationWindow, {updateUrl: updateLink},
                                function(wnd) {wnd.show();});
            applicationWindow.showUpdateLink = true
        }

        onUpdateDownloaded: {
            if (applicationWindow.openedDialogsCount == 0) {
                Common.launchDialog("Dialogs/InstallUpdateDialog.qml", applicationWindow, {})
            } else {
                console.debug("Opened dialogs found. Postponing upgrade flow...");
                upgradeTimer.start()
            }
        }

        onUpgradeInitiated: {
            console.debug("UI:onUpgradeInitiated handler")
            closeHandler({accepted: false});
        }
    }

    Timer {
        id: upgradeTimer
        interval: 5000
        repeat: true
        running: false
        triggeredOnStart: false
        onTriggered: {
            if (applicationWindow.openedDialogsCount == 0) {
                upgradeTimer.stop()
                Common.launchDialog("Dialogs/InstallUpdateDialog.qml", applicationWindow, {})
            }
        }
    }

    Timer {
        id: donateTimer
        interval: 15000
        repeat: true
        running: false
        triggeredOnStart: false
        onTriggered: {
            if ((applicationWindow.openedDialogsCount == 0) &&
                    (mainStackView.areActionsAllowed) &&
                    (artworkRepository.artworksSourcesCount > 0) &&
                    (switcher.isDonationCampaign1Active) &&
                    (switcher.isDonateCampaign1Stage2On)) {
                donateTimer.stop()
                Common.launchDialog("Dialogs/DonateDialog.qml", applicationWindow, {})
            }

            if ((!switcher.isDonationCampaign1Active) ||
                    (!switcher.isDonateCampaign1Stage2On)) {
                console.debug("Donate campaing is over. Switching off donate timer")
                donateTimer.stop()
            }
        }
    }

    Rectangle {
        id: globalHost
        color: uiColors.defaultDarkColor
        anchors.fill: parent

        DropArea {
            enabled: applicationWindow.actionsEnabled
            anchors.fill: parent
            onDropped: {
                if (drop.hasUrls) {
                    var filesCount = artItemsModel.dropFiles(drop.urls)
                    console.debug(filesCount + ' files added via drag&drop')
                }
            }
        }

        Item {
            id: leftDockingGroup
            width: 250
            anchors.left: parent.left
            anchors.leftMargin: leftCollapser.width
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            states: [
                State {
                    name: "collapsed"

                    PropertyChanges {
                        target: leftDockingGroup
                        anchors.leftMargin: -leftDockingGroup.width + leftCollapser.width
                    }
                }
            ]

            Behavior on anchors.leftMargin {
                NumberAnimation {
                    duration: 100
                    easing.type: Easing.InQuad
                }
            }

            Row {
                id: tabsHolder
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                property int currentIndex: 0
                property bool moreTabsAvailable: tabsModel.tabsCount > 3
                property real expanderWidth: tabsHolder.moreTabsAvailable ? plusTab.width : 0
                height: 55
                spacing: 0

                Repeater {
                    id: tabsRepeater
                    model: activeTabs

                    delegate: CustomTab {
                        id: customTab
                        tabIndex: index
                        width: (tabsHolder.width - tabsHolder.expanderWidth) / tabsRepeater.count
                        isSelected: tabsHolder.currentIndex == tabIndex
                        hovered: (!isSelected) && tabMA.containsMouse

                        function activateThisTab() {
                            var tabIndex = customTab.tabIndex;
                            tabsHolder.currentIndex = tabIndex
                        }

                        Loader {
                            property bool isHighlighted: customTab.isSelected || customTab.hovered
                            property color parentBackground: customTab.color
                            anchors.centerIn: parent
                            source: tabicon
                        }

                        Connections {
                            target: activeTabs
                            onTabActivateRequested: {
                                console.log("On tab opened " + originalTabIndex)
                                if (activeTabs.getIndex(customTab.tabIndex) === originalTabIndex) {
                                    customTab.activateThisTab()
                                }
                            }
                        }

                        StyledText {
                            enabled: debugTabs
                            visible: debugTabs
                            anchors.top: parent.top
                            anchors.right: parent.right
                            text: cachetag
                            isActive: false
                        }

                        MouseArea {
                            id: tabMA
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: {
                                customTab.activateThisTab()
                                activeTabs.openTab(tabIndex)
                            }
                        }
                    }
                }

                CustomTab {
                    id: plusTab
                    enabled: false
                    visible: false
                    tabIndex: tabsRepeater.count
                    isSelected: tabsHolder.currentIndex == tabIndex
                    hovered: (!isSelected) && plusMA.containsMouse
                    width: 20
                    property bool isHighlighted: isSelected || hovered

                    Connections {
                        target: tabsModel
                        onTabsCountChanged: {
                            if (tabsModel.tabsCount <= 3) {
                                if (plusTab.isSelected) {
                                    activeTabs.reactivateMostRecentTab()
                                }
                            }

                            plusTab.enabled = tabsModel.tabsCount > 3
                            plusTab.visible = tabsModel.tabsCount > 3
                        }
                    }

                    TriangleElement {
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: isFlipped ? height*0.3 : 0
                        color: (enabled && (plusMA.containsMouse || plusTab.isHighlighted)) ? uiColors.labelActiveForeground : uiColors.labelInactiveForeground
                        isFlipped: !plusTab.isSelected
                        width: parent.width * 0.6
                        height: width * 0.5
                    }

                    MouseArea {
                        id: plusMA
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            var tabIndex = tabsRepeater.count;
                            tabsHolder.currentIndex = tabIndex
                        }
                    }
                }
            }

            Rectangle {
                color: uiColors.defaultControlColor
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: tabsHolder.bottom
                anchors.bottom: parent.bottom

                StackLayout {
                    id: mainTabView
                    anchors.fill: parent
                    currentIndex: tabsHolder.currentIndex

                    Repeater {
                        model: activeTabs

                        delegate: Loader {
                            id: tabLoader
                            source: tabcomponent
                            asynchronous: true
                            property int myIndex: index
                            property var tabModel: uiManager.retrieveTabsModel(tabid)
                            property int selectedArtworksCount: filteredArtItemsModel.selectedArtworksCount
                            property bool areActionsAllowed: mainStackView.areActionsAllowed

                            Connections {
                                target: mainTabView
                                onCurrentIndexChanged: {
                                    if (tabLoader.myIndex == mainTabView.currentIndex) {
                                        if (tabLoader.status == Loader.Ready) {
                                            if (typeof tabLoader.item.initializeTab !== "undefined") {
                                                tabLoader.item.initializeTab()
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Flow {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        anchors.topMargin: 5
                        spacing: 5

                        Repeater {
                            id: inactiveTabsRepeater
                            model: inactiveTabs

                            delegate: CustomTab {
                                anchors.top: undefined
                                anchors.bottom: undefined
                                id: customInactiveTab
                                tabIndex: index
                                width: (tabsHolder.width - plusTab.width) / tabsRepeater.count
                                height: 45
                                isSelected: false
                                hovered: (!isSelected) && inactiveTabMA.containsMouse

                                Loader {
                                    property bool isHighlighted: customInactiveTab.isSelected || customInactiveTab.hovered
                                    property color parentBackground: customInactiveTab.color
                                    anchors.centerIn: parent
                                    source: tabicon
                                }

                                StyledText {
                                    enabled: debugTabs
                                    visible: debugTabs
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    text: cachetag
                                    isActive: false
                                }

                                MouseArea {
                                    id: inactiveTabMA
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        inactiveTabs.openTab(tabIndex)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // hack for visual order of components (slider will be created after left panel)
        // in order not to deal with Z animation/settings
        Rectangle {
            id: leftCollapser
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            color: uiColors.leftSliderColor
            width: 20

            TriangleElement {
                width: leftCollapseMA.pressed ? 6 : 7
                height: leftCollapseMA.pressed ? 12 : 14
                isVertical: true
                isFlipped: !applicationWindow.leftSideCollapsed
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: isFlipped ? -1 : 0
                color: {
                    if (leftCollapseMA.pressed) {
                        return uiColors.whiteColor
                    } else {
                        return leftCollapseMA.containsMouse ? uiColors.labelInactiveForeground : uiColors.inputBackgroundColor
                    }
                }
            }

            MouseArea {
                id: leftCollapseMA
                hoverEnabled: true
                anchors.fill: parent
                onClicked: toggleLeftPane()
            }
        }

        StackView {
            id: mainStackView
            anchors.left: leftDockingGroup.right
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            focus: true
            property bool areActionsAllowed: depth <= 1

            initialItem: MainGrid {
            }

            delegate: StackViewDelegate {
                function transitionFinished(properties)
                {
                    properties.exitItem.opacity = 1
                }

                pushTransition: StackViewTransition {
                    PropertyAnimation {
                        target: enterItem
                        property: "opacity"
                        from: 0
                        to: 1
                        duration: 100
                    }

                    PropertyAnimation {
                        target: exitItem
                        property: "opacity"
                        from: 1
                        to: 0
                        duration: 100
                    }
                }
            }
        }
    }

    statusBar: StatusBar {
        RowLayout {
            width: parent.width
            height: 20
            implicitHeight: 20
            spacing: 5

            Item {
                width: 20
            }

            StyledLink {
                text: i18.n + qsTr("Check warnings")
                enabled: mainStackView.areActionsAllowed && (warningsModel.warningsCount > 0)
                normalLinkColor: uiColors.labelActiveForeground
                onClicked: {
                    warningsModel.update()
                    //filteredArtItemsModel.checkForWarnings()

                    var wasCollapsed = applicationWindow.leftSideCollapsed
                    applicationWindow.collapseLeftPane()
                    mainStackView.push({
                                           item: "qrc:/StackViews/WarningsView.qml",
                                           properties: {
                                               componentParent: applicationWindow,
                                               wasLeftSideCollapsed: wasCollapsed
                                           },
                                           destroyOnPop: true
                                       })
                }
            }

            StyledText {
                visible: applicationWindow.showUpdateLink
                text: "|"
                isActive: false
                verticalAlignment: Text.AlignVCenter
            }

            StyledLink {
                visible: applicationWindow.showUpdateLink
                enabled: applicationWindow.showUpdateLink
                text: i18.n + qsTr("Update available!")
                color: isPressed ? uiColors.linkClickedColor : uiColors.greenColor
                onClicked: {
                    Qt.openUrlExternally("http://xpiksapp.com/downloads/")
                }
            }

            Item {
                Layout.fillWidth: true
            }

            StyledText {
                id: filteredCountText
                text: i18.n + qsTr("No items available")
                color: uiColors.labelInactiveForeground
                verticalAlignment: Text.AlignVCenter
                visible: artItemsModel.modifiedArtworksCount == 0
                enabled: artItemsModel.modifiedArtworksCount == 0

                function updateText() {
                    var itemsCount = filteredArtItemsModel.getItemsCount()
                    if (itemsCount > 0) {
                        text = itemsCount > 1 ? qsTr("%1 items available").arg(itemsCount) : qsTr("1 item available")
                    } else {
                        text = qsTr("No items available")
                    }
                }

                Component.onCompleted: updateText()
                Connections {
                    target: filteredArtItemsModel
                    onRowsInserted: filteredCountText.updateText()
                    onRowsRemoved: filteredCountText.updateText()
                }

                Connections {
                    target: languagesModel
                    onLanguageChanged: filteredCountText.updateText()
                }
            }

            StyledText {
                text: i18.n + getOriginalText()
                verticalAlignment: Text.AlignVCenter
                color: uiColors.artworkModifiedColor
                visible: artItemsModel.modifiedArtworksCount > 0
                enabled: artItemsModel.modifiedArtworksCount > 0

                function getOriginalText() {
                    return artItemsModel.modifiedArtworksCount > 1 ? qsTr("%1 modified items").arg(artItemsModel.modifiedArtworksCount) : (artItemsModel.modifiedArtworksCount === 1 ? qsTr("1 modified item") : qsTr("No modified items"))
                }

                MouseArea {
                    id: selectModifiedMA
                    anchors.fill: parent
                    enabled: mainStackView.areActionsAllowed
                    cursorShape: artItemsModel.modifiedArtworksCount > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (artItemsModel.modifiedArtworksCount > 0) {
                            filteredArtItemsModel.searchTerm = "x:modified"
                        }
                    }
                }
            }

            Item {
                width: 20
            }
        }

        style: StatusBarStyle {
            background: Rectangle {
                implicitHeight: 20
                implicitWidth: 200
                color: uiColors.statusBarColor
            }
        }
    }
}
