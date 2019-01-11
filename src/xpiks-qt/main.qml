/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2019 Taras Kushnir <kushnirTV@gmail.com>
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
    property bool actionsEnabled: appHost.areActionsAllowed && (openedDialogsCount == 0)

    onVisibleChanged: {
        if (needToCenter) {
            needToCenter = false
            applicationWindow.x = uiManager.getAppPosX(
                        (Screen.width - applicationWindow.width) / 2, // default
                        (Screen.desktopAvailableWidth - applicationWindow.width)) // max
            applicationWindow.y = uiManager.getAppPosY(
                        (Screen.height - applicationWindow.height) / 2, // default
                        (Screen.desktopAvailableHeight - applicationWindow.height)) // max
        }
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
                Common.launchDialog("Dialogs/TermsAndConditionsDialog.qml", applicationWindow, {})
            } else {
                helpersWrapper.reportOpen()
            }
        }
    }

    function closeHandler(close) {
        console.info("closeHandler")

        if (artworksListModel.modifiedArtworksCount > 0) {
            console.debug("Modified artworks present")
            close.accepted = false
            confirmExitDialog.open()
        } else {
            console.debug("No modified artworks found. Exiting...")
            shutdownEverything()
            close.accepted = false
        }
    }

    function shutdownEverything() {
        applicationWindow.visibility = "Minimized"
        xpiksApp.shutdown();
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

    function onDialogCreated(name) {
        openedDialogsCount += 1
        console.info("Opened dialog " + name + ". Opened dialogs count is " + openedDialogsCount);
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
        dispatcher.dispatch(UICommand.InitUploadHosts, masterPasswordCorrectOrEmpty)
        dispatcher.dispatch(UICommand.SetupUpload, skipUploadItems)
    }

    function launchImportDialog(importID, reimport) {
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
    }

    function tryUploadArtworks() {
        if (filteredArtworksListModel.areSelectedArtworksSaved()) {
            openUploadDialog(false)
        } else {
            mustSaveWarning.open()
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
        enabled: xpiksApp.isUpdateDownloaded && (applicationWindow.openedDialogsCount == 0)
        onTriggered: xpiksApp.upgradeNow()
    }

    Action {
        id: quitAction
        text: i18.n + qsTr("Exit")
        shortcut: StandardKey.Quit
        onTriggered: closeHandler({accepted: false});
    }

    Action {
        id: saveAction
        shortcut: StandardKey.Save
        enabled: (artworksRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtworksListModel.selectedArtworksCount == 0) {
                mustSelectDialog.open()
            } else {
                var modifiedSelectedCount = filteredArtworksListModel.getModifiedSelectedCount();

                if (filteredArtworksListModel.selectedArtworksCount > 0 && modifiedSelectedCount > 0) {
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
        onTriggered: dispatcher.dispatch(UICommand.SetupFindInArtworks, {})
        enabled: (artworksRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
    }

    Action {
        id: removeAction
        shortcut: "Ctrl+Del"
        enabled: (artworksRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtworksListModel.selectedArtworksCount === 0) {
                mustSelectDialog.open()
            } else {
                var itemsCount = filteredArtworksListModel.selectedArtworksCount
                if (itemsCount > 0) {
                    if (mustUseConfirmation()) {
                        confirmRemoveSelectedDialog.itemsCount = itemsCount
                        confirmRemoveSelectedDialog.open()
                    } else {
                        dispatcher.dispatch(UICommand.RemoveSelected, {})
                    }
                }
            }
        }
    }

    Action {
        id: uploadAction
        shortcut: "Shift+Ctrl+U"
        enabled: (artworksRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtworksListModel.selectedArtworksCount === 0) {
                filteredArtworksListModel.selectFilteredArtworks();
            }

            if (filteredArtworksListModel.selectedArtworksCount > 0) {
                tryUploadArtworks();
            }
        }
    }

    Action {
        id: addFilesAction
        shortcut: StandardKey.Open
        onTriggered: chooseArtworksDialog.open()
        enabled: applicationWindow.openedDialogsCount == 0
    }

    Action {
        id: showLogsAction
        shortcut: "Shift+Ctrl+L"
        enabled: applicationWindow.openedDialogsCount == 0
        onTriggered: {
            dispatcher.dispatch(UICommand.UpdateLogs, false)
            Common.launchDialog("Dialogs/LogsDialog.qml", applicationWindow, {})
        }
    }

    Action {
        id: fixSpellingInSelectedAction
        text: i18.n + qsTr("&Fix spelling")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Fix spelling in selected triggered")
            dispatcher.dispatch(UICommand.ReviewSpellingInSelected, {})
        }
    }

    Action {
        id: fixDuplicatesInSelectedAction
        text: i18.n + qsTr("&Show duplicates")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Fix duplicates in selected triggered")
            dispatcher.dispatch(UICommand.ReviewDuplicatesInSelected, true)
        }
    }

    Action {
        id: removeMetadataAction
        text: i18.n + qsTr("&Remove metadata")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Remove metadata from selected triggered")
            removeMetadataDialog.open()
        }
    }

    Action {
        id: deleteKeywordsAction
        text: i18.n + qsTr("&Delete keywords")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Delete keywords from selected triggered")
            dispatcher.dispatch(UICommand.SetupDeleteKeywordsInSelected, {})
        }
    }

    Action {
        id: detachVectorsAction
        text: i18.n + qsTr("&Detach vectors")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Detach vectors from selected triggered")
            filteredArtworksListModel.detachVectorFromSelected()
        }
    }

    Action {
        id: createArchivesAction
        text: i18.n + qsTr("&Create archives")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Zip archives triggered")
            dispatcher.dispatch(UICommand.SetupCreatingArchives, {})
        }
    }

    Action {
        id: exportToCsvAction
        text: i18.n + qsTr("&Export to CSV")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("CSV export triggered")
            dispatcher.dispatch(UICommand.SetupCSVExportForSelected, {})
        }
    }

    Action {
        id: reimportMetadataAction
        text: i18.n + qsTr("&Reimport metadata")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
        onTriggered: {
            console.info("Reimport metadata triggered")
            reimportConfirmationDialog.open()
        }
    }

    Action {
        id: overwriteMetadataAction
        text: i18.n + qsTr("&Overwrite metadata")
        enabled: (filteredArtworksListModel.selectedArtworksCount > 0) && applicationWindow.actionsEnabled
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
                        onTriggered: xpiksApp.addDirectories([display])
                    }
                }
            }

            Menu {
                id: recentFilesMenu
                title: i18.n + qsTr("&Recent files")
                enabled: (applicationWindow.openedDialogsCount == 0) && (recentFiles.count > 0)

                MenuItem {
                    text: i18.n + qsTr("Open all")
                    onTriggered: xpiksApp.addFiles(recentFiles.getAllRecentFiles())
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
                        onTriggered: xpiksApp.addFiles([display])
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
                        settingsModel.saveSession = checked
                        // TODO: save session command if checked
                    }

                    Component.onCompleted: {
                        checked = settingsModel.saveSession
                    }
                }
            }

            MenuItem {
                action: upgradeAction
                visible: xpiksApp.isUpdateDownloaded
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
                enabled: (artworksRepository.artworksSourcesCount > 0) && applicationWindow.actionsEnabled
                onTriggered: {
                    console.info("Invert selection triggered")
                    if (filteredArtworksListModel.getItemsCount() > 0) {
                        filteredArtworksListModel.invertSelectionArtworks()
                    }
                }
            }

            MenuItem {
                text: i18.n + qsTr("&Sort by filename")
                enabled: (artworksRepository.artworksSourcesCount > 0) && applicationWindow.actionsEnabled
                checkable: true
                onToggled: {
                    console.info("Sort by filename")
                    if (filteredArtworksListModel.getItemsCount() > 0) {
                        filteredArtworksListModel.toggleSorted();
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
                enabled: xpiksApp.pluginsAvailable
                visible: xpiksApp.pluginsAvailable

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
                        dispatcher.dispatch(UICommand.InitUserDictionary, {})
                        Common.launchDialog("Dialogs/UserDictEditDialog.qml", applicationWindow, {})
                    }
                }

                MenuItem {
                    text: i18.n + qsTr("&Wipe all metadata from files")
                    onTriggered: {
                        console.info("Wipe metadata triggered")
                        if (filteredArtworksListModel.selectedArtworksCount == 0) {
                            mustSelectDialog.open()
                        } else if (filteredArtworksListModel.selectedArtworksCount > 0) {
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
                onTriggered: Qt.openUrlExternally("https://xpiksapp.com/tutorials/")
            }

            MenuItem {
                text: i18.n + qsTr("&FAQ")
                onTriggered: Qt.openUrlExternally("https://xpiksapp.com/faq/")
            }

            MenuItem {
                text: i18.n + qsTr("&Support")
                onTriggered: Qt.openUrlExternally("https://xpiksapp.com/support/")
            }
        }

        Menu {
            title: "Debug"
            visible: debug
            enabled: debug

            MenuItem {
                text: "Update all items"
                onTriggered: {
                    filteredArtworksListModel.updateFilter()
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
                                        applicationWindow, {updateUrl: "https://xpiksapp.com/downloads/"},
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
                    Common.launchDialog("Dialogs/TermsAndConditionsDialog.qml", applicationWindow, {})
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

            MenuItem {
                text: "Crash"
                onTriggered: xpiksApp.debugCrash()
            }
        }
    }

    MessageDialog {
        id: confirmExitDialog
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
        onAccepted: xpiksApp.removeUnavailableFiles()
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
        onYes: filteredArtworksListModel.removeMetadataInSelected()
    }

    MessageDialog {
        id: confirmRemoveSelectedDialog
        property int itemsCount
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Are you sure you want to remove %1 item(s)?").arg(itemsCount)
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: dispatcher.dispatch(UICommand.RemoveSelected, {})
    }

    MessageDialog {
        id: reimportConfirmationDialog
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("You will lose all unsaved changes after reimport. Proceed?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: dispatcher.dispatch(UICommand.SetupReimportMetadata, {})
    }

    FileDialog {
        id: chooseArtworksDialog
        title: "Please choose artworks"
        selectExisting: true
        selectMultiple: true
        folder: shortcuts.pictures
        nameFilters: [ "All Xpiks files (*.jpg *.jpeg *.tiff *.tif *.eps *.ai *.mov *.mp4 *.avi *.mpeg *.qt *.vob *.wmv *.asf *.asx *.flv)",
            "Image files (*.jpg *.jpeg *.tiff *.tif *.eps *.ai)",
            "Video files (*.mov *.mp4 *.avi *.mpeg *.qt *.vob *.wmv *.asf *.asx *.flv)",
            "All files (*)" ]

        onAccepted: {
            console.debug("You chose: " + chooseArtworksDialog.fileUrls)
            var filesAdded = xpiksApp.addFiles(chooseArtworksDialog.fileUrls)
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
            var filesAdded = xpiksApp.addDirectories(chooseDirectoryDialog.fileUrls)
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
        target: artworksListModel

        onUnavailableArtworksFound: {
            console.debug("UI:onUnavailableArtworksFound")
            unavailableArtworksDialog.open()
        }

        onUnavailableVectorsFound: {
            console.debug("UI:onUnavailableVectorsFound")
            unavailableVectorsDialog.open()
        }
    }

    Connections {
        target: xpiksApp

        onArtworksAdded: {
            if ((imagesCount === 0) && (vectorsCount === 0)) {
                noNewFilesDialog.open();
                return;
            } else if ((imagesCount === 0) && (vectorsCount > 0)) {
                vectorsAttachedDialog.vectorsAttached = vectorsCount
                vectorsAttachedDialog.open()
            }

            var latestDir = recentDirectories.getLatestItem()
            chooseArtworksDialog.folder = latestDir
            chooseDirectoryDialog.folder = latestDir
        }

        onUpgradeInitiated: {
            console.debug("UI:onUpgradeInitiated handler")
            closeHandler({accepted: false});
        }
    }

    Connections {
        target: metadataIOCoordinator
        onImportStarted: launchImportDialog(importID, reimport)
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
                    (applicationWindow.actionsEnabled) &&
                    (artworksRepository.artworksSourcesCount > 0) &&
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

    AppHost {
        id: appHost
        componentParent: applicationWindow
        anchors.fill: parent
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
                enabled: applicationWindow.actionsEnabled && (warningsModel.warningsCount > 0)
                normalLinkColor: uiColors.labelActiveForeground
                onClicked: dispatcher.dispatch(UICommand.CheckWarnings, {})
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
                onClicked: Qt.openUrlExternally("https://xpiksapp.com/downloads/")
            }

            Item {
                Layout.fillWidth: true
            }

            StyledText {
                id: filteredCountText
                text: i18.n + qsTr("No items available")
                color: uiColors.labelInactiveForeground
                verticalAlignment: Text.AlignVCenter
                visible: artworksListModel.modifiedArtworksCount == 0
                enabled: artworksListModel.modifiedArtworksCount == 0

                function updateText() {
                    var itemsCount = filteredArtworksListModel.getItemsCount()
                    if (itemsCount > 0) {
                        text = itemsCount > 1 ? qsTr("%1 items available").arg(itemsCount) : qsTr("1 item available")
                    } else {
                        text = qsTr("No items available")
                    }
                }

                Component.onCompleted: updateText()
                Connections {
                    target: filteredArtworksListModel
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
                visible: artworksListModel.modifiedArtworksCount > 0
                enabled: artworksListModel.modifiedArtworksCount > 0

                function getOriginalText() {
                    return artworksListModel.modifiedArtworksCount > 1 ?
                                qsTr("%1 modified items").arg(artworksListModel.modifiedArtworksCount) :
                                (artworksListModel.modifiedArtworksCount === 1 ? qsTr("1 modified item") :
                                                                             qsTr("No modified items"))
                }

                MouseArea {
                    id: selectModifiedMA
                    anchors.fill: parent
                    enabled: applicationWindow.actionsEnabled
                    cursorShape: artworksListModel.modifiedArtworksCount > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (artworksListModel.modifiedArtworksCount > 0) {
                            filteredArtworksListModel.searchTerm = "x:modified"
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
