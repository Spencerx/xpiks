/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2016 Taras Kushnir <kushnirTV@gmail.com>
 *
 * Xpiks is distributed under the GNU General Public License, version 3.0
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.2
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtQuick.Window 2.0
import xpiks 1.0
import "Constants"
import "Constants/Colors.js" as Colors
import "Components"
import "StyledControls"
import "Dialogs"
import "Common.js" as Common

ApplicationWindow {
    id: applicationWindow
    visible: true
    width: appSettings.getAppWidth(900)
    height: appSettings.getAppHeight(725)
    minimumHeight: 670
    minimumWidth: 900
    title: qsTr("Xpiks")
    property int openedDialogsCount: 0
    property bool showUpdateLink: false
    property bool needToCenter: true
    property bool listLayout: true

    onVisibleChanged: {
        if (needToCenter) {
            needToCenter = false
            applicationWindow.x = appSettings.getAppPosX((Screen.width - applicationWindow.width) / 2)
            applicationWindow.y = appSettings.getAppPosY((Screen.height - applicationWindow.height) / 2)
        }
    }

    function saveRecentDirectories() {
        appSettings.setValue(appSettings.recentDirectoriesKey, recentDirectories.serializeForSettings())
    }

    function closeHandler(close) {
        console.info("UI::main # closeHandler")
        saveRecentDirectories()

        if (artItemsModel.modifiedArtworksCount > 0) {
            close.accepted = false
            configExitDialog.open()
        } else {
            console.debug("UI::main # No modified artworks found. Exiting...")
            applicationWindow.visibility = "Minimized"
            helpersWrapper.beforeDestruction();
            appSettings.protectTelemetry();
            close.accepted = false
            saveAppGeometry()
            closingTimer.start()
        }
    }

    function saveAppGeometry() {
        console.debug("UI::main # Saving application geometry")
        appSettings.setAppWidth(applicationWindow.width)
        appSettings.setAppHeight(applicationWindow.height)
        appSettings.setAppPosX(applicationWindow.x)
        appSettings.setAppPosY(applicationWindow.y)
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
            console.debug("UI::main # Delayed onOpen timer triggered");
            helpersWrapper.afterConstruction()
            helpersWrapper.setProgressIndicator(applicationWindow)

            if (appSettings.needToShowWhatsNew()) {
                var text = appSettings.whatsNewText;
                if (text.length > 0) {
                    Common.launchDialog("Dialogs/WhatsNewDialog.qml",
                                        applicationWindow,
                                        {
                                            whatsNewText: text
                                        })
                }
            }

            if (appSettings.needToShowTermsAndConditions()) {
                var licenseText = appSettings.termsAndConditionsText;
                if (licenseText.length > 0) {
                    Common.launchDialog("Dialogs/TermsAndConditionsDialog.qml",
                                        applicationWindow,
                                        {
                                            termsText: licenseText
                                        })
                }
            } else {
                helpersWrapper.reportOpen()
            }
        }
    }

    function onDialogClosed() {
        openedDialogsCount -= 1
        console.debug("UI::main # Dialog closed. Opened dialogs count is " + openedDialogsCount)
    }

    function mustUseConfirmation() {
        var mustUse = appSettings.boolValue(appSettings.useConfirmationDialogsKey, true)
        return mustUse
    }

    function openUploadDialog() {
        if (appSettings.boolValue(appSettings.mustUseMasterPasswordKey, false)) {
            var callbackObject = {
                onSuccess: function() { doOpenUploadDialog(true) },
                onFail: function() { doOpenUploadDialog(false) }
            }

            Common.launchDialog("Dialogs/EnterMasterPasswordDialog.qml",
                         applicationWindow,
                         {callbackObject: callbackObject})
        } else {
            doOpenUploadDialog(true)
        }
    }

    function doOpenUploadDialog(masterPasswordCorrectOrEmpty) {
        artworkUploader.resetModel()
        filteredArtItemsModel.setSelectedForUpload()
        uploadInfos.initializeAccounts(masterPasswordCorrectOrEmpty)

        warningsModel.setShowSelected()
        Common.launchDialog("Dialogs/UploadArtworks.qml",
                     applicationWindow,
                     {componentParent: applicationWindow})
    }

    function clearFilter() {
        filteredArtItemsModel.searchTerm = ''
        if (filterText.length > 0) {
            filterText.text = ''
        }
        filterClearTimer.start()
    }

    Component.onCompleted: {
        console.debug("UI::main # onCompleted handler")
        openingTimer.start()
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("&File")

            Menu {
                id: recentDirectoriesMenu
                title: qsTr("&Recent directories")

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

            MenuItem {
                text: qsTr("&Settings")
                onTriggered: {
                    settingsModel.readAllValues()
                    Common.launchDialog("Dialogs/SettingsWindow.qml",
                                    applicationWindow, {},
                                    function(wnd) {wnd.show();});
                }
            }

            MenuItem {
                text: qsTr("&About")
                onTriggered: {
                    Common.launchDialog("Dialogs/AboutWindow.qml",
                                    applicationWindow, {},
                                    function(wnd) {wnd.show();});
                }
            }

            MenuItem {
                text: qsTr("&Exit")
                onTriggered: closeHandler({accepted: false});
            }
        }

        Menu {
            title: qsTr("&Edit")

            MenuItem {
                text: qsTr("&Invert selection")
                enabled: imagesListView.count > 0
                onTriggered: {
                    console.info("UI::main # Invert selection triggered")
                    filteredArtItemsModel.invertSelectionArtworks()
                }
            }

            MenuItem {
                text: qsTr("&Sort by filename")
                enabled: imagesListView.count > 0
                checkable: true
                onToggled: {
                    console.info("UI::main # Sort by filename")
                    filteredArtItemsModel.toggleSorted();
                }
            }

            MenuItem {
                text: qsTr("&Remove metadata from selected")
                enabled: filteredArtItemsModel.selectedArtworksCount > 0
                onTriggered: {
                    console.info("UI::main # Remove metadata from selected triggered")
                    removeMetadataDialog.open()
                }
            }

            MenuItem {
                text: qsTr("&Detach vectors from selected")
                enabled: filteredArtItemsModel.selectedArtworksCount > 0
                onTriggered: {
                    console.info("UI::main # Detach vectors from selected triggered")
                    filteredArtItemsModel.detachVectorFromSelected()
                }
            }

            MenuItem {
                text: qsTr("&Cleanup local library in background")
                onTriggered: {
                    console.info("UI::main # Cleanup local library triggered")
                    helpersWrapper.cleanupLocalLibrary()
                }
            }

            MenuItem {
                text: qsTr("&Manage upload hosts")
                onTriggered: {
                    console.info("UI::main # Manage upload hosts triggered")
                    openUploadDialog()
                }
            }

        }

        Menu {
            title: qsTr("&Plugins")
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
                    property var actionsModel: pluginManager.getPluginActions(index)

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
                text: qsTr("&Plugin manager")
                onTriggered: {
                    Common.launchDialog("Dialogs/PluginsDialog.qml",
                                        applicationWindow, {});
                }
            }
        }

        Menu {
            title: qsTr("&Tools")
            enabled: applicationWindow.openedDialogsCount == 0

            MenuItem {
                text: qsTr("&Zip selected artworks")
                enabled: filteredArtItemsModel.selectedArtworksCount > 0
                onTriggered: {
                    console.info("UI::main # Zip archives triggered")

                    filteredArtItemsModel.setSelectedForZipping()
                    Common.launchDialog("Dialogs/ZipArtworksDialog.qml",
                                    applicationWindow,
                                    {});
                }
            }

            MenuItem {
                text: qsTr("&Import metadata from selected")
                enabled: filteredArtItemsModel.selectedArtworksCount > 0
                onTriggered: {
                    console.info("UI::main # Reimport archives triggered")
                    filteredArtItemsModel.reimportMetadataForSelected()
                }
            }

            MenuItem {
                text: qsTr("&Overwrite metadata in selected")
                enabled: filteredArtItemsModel.selectedArtworksCount > 0
                onTriggered: {
                    console.info("UI::main # Overwrite metadata triggered")
                    Common.launchDialog("Dialogs/ExportMetadata.qml", applicationWindow, {overwriteAll: true})
                }
            }

            MenuItem {
                text: qsTr("&Check spelling in selected")
                enabled: filteredArtItemsModel.selectedArtworksCount > 0
                onTriggered: {
                    console.info("UI::main # Spell check in selected")
                    filteredArtItemsModel.spellCheckSelected()
                    Common.launchDialog("Dialogs/SpellCheckDialog.qml",
                                        applicationWindow, {});
                }
            }
        }

        Menu {
            title: qsTr("&Help")

            MenuItem {
                text: qsTr("&Video tutorials")
                onTriggered: Qt.openUrlExternally("http://ribtoks.github.io/xpiks/blog/2016/videos")
            }

            MenuItem {
                text: qsTr("&FAQ")
                onTriggered: Qt.openUrlExternally("http://ribtoks.github.io/xpiks/faq/")
            }

            MenuItem {
                text: qsTr("&Support")
                onTriggered: Qt.openUrlExternally("http://ribtoks.github.io/xpiks/support/")
            }
        }
    }

    MessageDialog {
        id: configExitDialog

        title: "Confirmation"
        text: qsTr("You have some artworks modified. Really exit?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            applicationWindow.visibility = "Minimized"
            helpersWrapper.beforeDestruction();
            saveAppGeometry()
            closingTimer.start()
        }
    }

    MessageDialog {
        id: removeMetadataDialog

        title: "Confirmation"
        text: qsTr("Remove metadata from selected artworks?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            filteredArtItemsModel.removeMetadataInSelected()
        }
    }

    MessageDialog {
        id: confirmRemoveSelectedDialog
        property int itemsCount
        title: "Confirmation"
        text: qsTr("Are you sure you want to remove %1 item(s)?").arg(itemsCount)
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
            openUploadDialog()
        } else {
            mustSaveWarning.open()
        }
    }

    MessageDialog {
        id: confirmRemoveDirectoryDialog
        property int directoryIndex
        title: "Confirmation"
        text: qsTr("Are you sure you want to remove this directory?")
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
        nameFilters: [ "Image files (*.jpg *.tiff)", "All files (*)" ]

        onAccepted: {
            console.debug("UI::main # You chose: " + chooseArtworksDialog.fileUrls)
            var filesAdded = artItemsModel.addLocalArtworks(chooseArtworksDialog.fileUrls)
            if (filesAdded > 0) {
                saveRecentDirectories()
                console.debug("UI::main # " + filesAdded + ' files via Open File(s)')
            } else {
                noNewFilesDialog.open()
            }
        }

        onRejected: {
            console.debug("UI::main # Open files dialog canceled")
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
            console.debug("UI::main # You chose: " + chooseDirectoryDialog.fileUrls)
            var filesAdded = artItemsModel.addLocalDirectories(chooseDirectoryDialog.fileUrls)
            if (filesAdded > 0) {
                saveRecentDirectories()
                console.debug("UI::main # " + filesAdded + ' files via Open Directory')
            } else {
                noNewFilesDialog.open()
            }
        }

        onRejected: {
            console.debug("UI::main # Directory dialog canceled")
        }
    }

    MessageDialog {
        id: mustSaveWarning
        title: "Warning"
        text: qsTr("Please, save selected items before upload")
    }

    MessageDialog {
        id: mustSelectDialog
        title: "Warning"
        text: qsTr("Please, select some artworks first")
    }

    MessageDialog {
        id: noNewFilesDialog
        title: "Information"
        text: qsTr("No new files were added")
    }

    MessageDialog {
        id: alreadySavedDialog
        title: "Information"
        text: qsTr("All selected items are already saved")
    }

    MessageDialog {
        id: vectorsAttachedDialog
        title: "Information"
        property int vectorsAttached: 0
        text: vectorsAttached > 1 ? qsTr("%1 vectors attached").arg(vectorsAttached) : qsTr("1 vector attached")
    }

    Connections {
        target: artItemsModel
        onArtworksAdded: {
            if ((imagesCount === 0) && (vectorsCount === 0)) {
                noNewFilesDialog.open();
                return;
            }

            var latestDir = recentDirectories.getLatestDirectory()
            chooseArtworksDialog.folder = latestDir
            chooseDirectoryDialog.folder = latestDir

            if (imagesCount > 0) {
                Common.launchDialog("Dialogs/ImportMetadata.qml", applicationWindow, {})
            } else if (vectorsCount > 0) {
                vectorsAttachedDialog.vectorsAttached = vectorsCount
                vectorsAttachedDialog.open()
            }
        }
    }

    Connections {
        target: updateService
        onUpdateAvailable: {
            Common.launchDialog("Dialogs/UpdateWindow.qml",
                                applicationWindow, {updateUrl: updateLink},
                                function(wnd) {wnd.show();});
            applicationWindow.showUpdateLink = true
        }
    }

    Rectangle {
        color: Colors.defaultDarkColor
        anchors.fill: parent

        DropArea {
            enabled: applicationWindow.openedDialogsCount == 0
            anchors.fill: parent
            onDropped: {
                if (drop.hasUrls) {
                    var filesCount = artItemsModel.dropFiles(drop.urls)
                    if (filesCount > 0) {
                        saveRecentDirectories()
                        console.debug(filesCount + ' files added via drag&drop')
                    }
                }
            }
        }

        ColumnLayout {
            id: repositoriesColumnLayout
            width: 250
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            spacing: 0

            Rectangle {
                width: parent.width
                height: 45
                color: Colors.defaultDarkColor

                StyledButton {
                    anchors.left: parent.left
                    anchors.leftMargin: 10
                    anchors.verticalCenter: parent.verticalCenter
                    id: addDirectoryButton
                    text: qsTr("Add directory")
                    width: 120
                    onClicked: chooseDirectoryDialog.open()
                }

                StyledButton {
                    anchors.leftMargin: 10
                    anchors.left: addDirectoryButton.right
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Add files")
                    width: 100
                    onClicked: chooseArtworksDialog.open()
                }
            }

            Rectangle {
                height: 1
                width: parent.width
                color: Colors.itemsSourceSelected
            }

            Rectangle {
                Layout.fillHeight: true
                width: 250

                color: Colors.defaultControlColor

                StyledScrollView {
                    anchors.fill: parent
                    anchors.topMargin: 5

                    ListView {
                        id: sourcesListView
                        model: artworkRepository
                        boundsBehavior: Flickable.StopAtBounds
                        anchors.fill: parent
                        anchors.rightMargin: 10
                        anchors.leftMargin: 10

                        spacing: 10

                        displaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        addDisplaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        removeDisplaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        delegate: Rectangle {
                            id: sourceWrapper
                            property int delegateIndex: index
                            color: isselected ? Colors.itemsSourceSelected : Colors.itemsSourceBackground
                            width: parent.width
                            height: 31
                            Layout.minimumWidth: 237

                            /*MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    filteredArtItemsModel.selectDirectory(sourceWrapper.delegateIndex)
                                }
                            }*/

                            RowLayout {
                                spacing: 10
                                anchors.fill: parent

                                Item {
                                    id: placeholder1
                                    width: 1
                                }

                                StyledText {
                                    id: directoryPath
                                    Layout.fillWidth: true
                                    anchors.verticalCenter: parent.verticalCenter
                                    height: 31
                                    color: Colors.itemsSourceForeground
                                    text: path + " (" + usedimagescount + ")"
                                    elide: Text.ElideMiddle
                                    font.bold: true
                                }

                                CloseIcon {
                                    width: 14
                                    height: 14
                                    anchors.verticalCenter: parent.verticalCenter
                                    isActive: false

                                    onItemClicked: {
                                        if (mustUseConfirmation()) {
                                            confirmRemoveDirectoryDialog.directoryIndex = sourceWrapper.delegateIndex
                                            confirmRemoveDirectoryDialog.open()
                                        } else {
                                            filteredArtItemsModel.removeArtworksDirectory(sourceWrapper.delegateIndex)
                                        }
                                    }
                                }

                                Item {
                                    id: placeholder2
                                    width: 1
                                }
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            anchors.left: repositoriesColumnLayout.right
            anchors.leftMargin: 2
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            spacing: 0

            Rectangle {
                height: 45
                color: Colors.defaultDarkColor
                z: 2000
                anchors.left: parent.left
                anchors.right: parent.right

                RowLayout {
                    spacing: 10
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.leftMargin: 0
                    anchors.rightMargin: mainScrollView.areScrollbarsVisible ? 30 : 20

                    Item {
                        width: 5
                    }

                    LayoutButton {
                        enabled: artworkRepository.artworksSourcesCount > 0
                        isListLayout: applicationWindow.listLayout
                        onLayoutChanged: applicationWindow.listLayout = !applicationWindow.listLayout
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledButton {
                        text: qsTr("Remove")
                        enabled: artworkRepository.artworksSourcesCount > 0
                        width: 80
                        onClicked: {
                            if (filteredArtItemsModel.selectedArtworksCount === 0) {
                                mustSelectDialog.open()
                            }
                            else {
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

                    StyledButton {
                        text: qsTr("Edit")
                        width: mainScrollView.areScrollbarsVisible ? 72 : 82
                        enabled: artworkRepository.artworksSourcesCount > 0
                        onClicked: {
                            if (filteredArtItemsModel.selectedArtworksCount === 0) {
                                mustSelectDialog.open()
                            }
                            else {
                                var launched = false

                                if (filteredArtItemsModel.selectedArtworksCount == 1) {
                                    var index = filteredArtItemsModel.findSelectedItemIndex()

                                    if (index !== -1) {
                                        Common.launchItemEditing(index, applicationWindow, {
                                                                     applyCallback: function() {}
                                                                 })
                                        launched = true
                                    }
                                }

                                if (!launched) {
                                    // also as fallback in case of errors in findSelectedIndex
                                    filteredArtItemsModel.combineSelectedArtworks();
                                    Common.launchDialog("Dialogs/CombinedArtworksDialog.qml",
                                                        applicationWindow, {componentParent: applicationWindow});
                                }
                            }
                        }
                    }

                    StyledButton {
                        text: qsTr("Save")
                        width: 80
                        enabled: artworkRepository.artworksSourcesCount > 0
                        onClicked: {
                            if (filteredArtItemsModel.selectedArtworksCount == 0) {
                                mustSelectDialog.open()
                            }
                            else {
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

                    StyledButton {
                        text: qsTr("Upload")
                        width: 90
                        enabled: artworkRepository.artworksSourcesCount > 0
                        onClicked: {
                            if (filteredArtItemsModel.selectedArtworksCount === 0) {
                                filteredArtItemsModel.selectFilteredArtworks();
                            }

                            if (filteredArtItemsModel.selectedArtworksCount > 0) {
                                tryUploadArtworks();
                            }
                        }
                    }
                }
            }

            Rectangle {
                height: 1
                anchors.left: parent.left
                anchors.right: parent.right
                color: Colors.itemsSourceSelected
            }

            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                Layout.fillHeight: true
                color: Colors.defaultControlColor

                Item {
                    anchors.topMargin: 10
                    anchors.fill: parent

                    Item {
                        id: filterRect
                        anchors.top: parent.top
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 24

                        StyledCheckbox {
                            id: selectAllCheckbox
                            anchors.left: parent.left
                            anchors.leftMargin: 20
                            anchors.verticalCenter: parent.verticalCenter
                            isContrast: true
                            enabled: artworkRepository.artworksSourcesCount > 0
                            text: filteredArtItemsModel.selectedArtworksCount === 0 ? qsTr("Select all") : qsTr("Select none")
                            checked: filteredArtItemsModel.selectedArtworksCount > 0

                            onClicked: {
                                if (checked) {
                                    filteredArtItemsModel.selectFilteredArtworks();
                                }
                                else {
                                    filteredArtItemsModel.unselectFilteredArtworks();
                                }
                            }

                            Connections {
                                target: filteredArtItemsModel
                                onSelectedArtworksCountChanged: {
                                    selectAllCheckbox.checked = filteredArtItemsModel.selectedArtworksCount > 0
                                }
                            }
                        }

                        Rectangle {
                            anchors.right: searchButton.left
                            anchors.rightMargin: 10
                            anchors.verticalCenter: parent.verticalCenter
                            color: Colors.defaultDarkColor
                            width: (artworkRepository.artworksSourcesCount > 0 && mainScrollView.areScrollbarsVisible) ? 252 : 262
                            height: 24

                            StyledTextInput {
                                id: filterText
                                width: (artworkRepository.artworksSourcesCount > 0 && mainScrollView.areScrollbarsVisible) ? 220 : 230
                                height: 24
                                clip: true
                                anchors.left: parent.left
                                anchors.leftMargin: 5
                                color: Colors.defaultInputBackground
                                enabled: artworkRepository.artworksSourcesCount > 0

                                onAccepted: {
                                    filteredArtItemsModel.searchTerm = text
                                }

                                Connections {
                                    target: filteredArtItemsModel
                                    onSearchTermChanged: filterText.text = filteredArtItemsModel.searchTerm
                                }
                            }

                            CloseIcon {
                                width: 14
                                height: 14
                                anchors.right: parent.right
                                anchors.rightMargin: 5
                                enabled: filterText.length > 0
                                anchors.verticalCenter: parent.verticalCenter
                                onItemClicked: clearFilter()
                            }

                            StyledText {
                                text: qsTr("Search...   x:empty  x:modified")
                                color: Colors.defaultInputBackground
                                opacity: (filterClearTimer.running || filterText.activeFocus || (filterText.length > 0)) ? 0 : 0.1
                                anchors.left: parent.left
                                anchors.leftMargin: 7
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Timer {
                                id: filterClearTimer
                                running: false
                                interval: 750
                                repeat: false
                            }
                        }

                        StyledBlackButton {
                            id: searchButton
                            anchors.right: parent.right
                            anchors.verticalCenter: parent.verticalCenter
                            anchors.rightMargin:  mainScrollView.areScrollbarsVisible ? 30 : 20
                            width: 90
                            text: qsTr("Search")
                            enabled: artworkRepository.artworksSourcesCount > 0
                            onClicked: filteredArtItemsModel.searchTerm = filterText.text
                        }
                    }

                    Rectangle {
                        id: undoRedoRect
                        color: Colors.defaultDarkColor
                        width: parent.width
                        height: 4
                        opacity: 0
                        anchors.topMargin: 0
                        anchors.top: filterRect.bottom

                        states: [
                            State {
                                name: "canundo"
                                when: undoRedoManager.canUndo
                                PropertyChanges {
                                    target: undoRedoRect
                                    height: 40
                                }
                            }
                        ]

                        transitions: [
                            Transition {
                                from: ""; to: "canundo"
                                ParallelAnimation {
                                    NumberAnimation {
                                        target: undoRedoRect
                                        properties: "height";
                                        from: 4
                                        to: 40
                                        easing.type: "InOutQuad";
                                        duration: 250
                                    }
                                    NumberAnimation {
                                        target: undoRedoRect
                                        properties: "anchors.topMargin";
                                        from: 0
                                        to: 4
                                        easing.type: "InOutQuad";
                                        duration: 250
                                    }
                                    NumberAnimation {
                                        target: undoRedoRect
                                        properties: "opacity";
                                        from: 0
                                        to: 1
                                        easing.type: "InOutQuad";
                                        duration: 250
                                    }
                                }
                            },
                            Transition {
                                from: "canundo"; to: ""
                                ParallelAnimation {
                                    NumberAnimation {
                                        target: undoRedoRect
                                        properties: "height";
                                        from: 40
                                        to: 4
                                        easing.type: "InOutQuad";
                                        duration: 250
                                    }
                                    NumberAnimation {
                                        target: undoRedoRect
                                        properties: "anchors.topMargin";
                                        from: 4
                                        to: 0
                                        easing.type: "InOutQuad";
                                        duration: 250
                                    }
                                    NumberAnimation {
                                        target: undoRedoRect
                                        properties: "opacity";
                                        from: 1
                                        to: 0
                                        easing.type: "InOutQuad";
                                        duration: 250
                                    }
                                }
                            }
                        ]

                        Rectangle {
                            anchors.left: parent.left
                            color: Colors.artworkSavedColor
                            width: applicationWindow.listLayout ? 6 : 3
                            height: parent.height
                        }

                        RowLayout {
                            anchors.centerIn: parent

                            StyledText {
                                id: undoDescription
                                text: undoRedoManager.undoDescription
                            }

                            Item {
                                width: 10
                            }

                            StyledText {
                                text: qsTr("Undo")
                                color: undoMA.pressed ? Colors.defaultLightColor : Colors.artworkActiveColor

                                MouseArea {
                                    id: undoMA
                                    anchors.fill: parent
                                    enabled: undoRedoManager.canUndo
                                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        undoRedoManager.undoLastAction()
                                        filteredArtItemsModel.updateFilter()
                                    }
                                }
                            }

                            StyledText {
                                text: qsTr("Dismiss (%1)").arg(settingsModel.dismissDuration - (autoDismissTimer.iterations % (settingsModel.dismissDuration + 1)))
                                color: dismissUndoMA.pressed ? Colors.defaultLightColor : Colors.defaultInputBackground

                                MouseArea {
                                    id: dismissUndoMA
                                    anchors.fill: parent
                                    enabled: undoRedoManager.canUndo
                                    cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor
                                    onClicked: {
                                        undoRedoManager.discardLastAction()
                                    }
                                }
                            }

                        }

                        Timer {
                            id: autoDismissTimer
                            property int iterations: 0
                            interval: 1000
                            repeat: true
                            running: undoRedoManager.canUndo
                            onTriggered: {
                                iterations += 1

                                if (iterations % (settingsModel.dismissDuration + 1) === settingsModel.dismissDuration) {
                                    undoRedoManager.discardLastAction()
                                    iterations = 0
                                }
                            }
                        }

                        Connections {
                            target: undoRedoManager
                            onItemRecorded: {
                                autoDismissTimer.iterations = 0
                            }
                        }
                    }

                    Rectangle {
                        id: separator
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.topMargin: visible ? 2 : 4
                        anchors.top: undoRedoRect.bottom
                        height: visible ? 2 : 0
                        color: Colors.defaultDarkColor
                        visible: !undoRedoManager.canUndo && (artworkRepository.artworksSourcesCount > 0)
                    }

                    StyledScrollView {
                        id: mainScrollView
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.top: separator.bottom
                        anchors.bottom: parent.bottom
                        property bool areScrollbarsVisible: flickableItem.contentHeight > flickableItem.height
                        __wheelAreaScrollSpeed: 50 + 10*settingsModel.scrollSpeedScale
                        // does not work for now in Qt 5.4.1 in combination with ListView
                        //verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn

                        GridView {
                            id: imagesListView
                            model: filteredArtItemsModel
                            boundsBehavior: Flickable.StopAtBounds
                            property int cellSpacing: 4
                            cellHeight: applicationWindow.listLayout ? (200 + 80*(settingsModel.keywordSizeScale - 1.0) + cellSpacing) : (200 + cellSpacing)
                            cellWidth: applicationWindow.listLayout ? imagesListView.width : (208 + cellSpacing)

                            function forceUpdateArtworks(needToMoveCurrentItem) {
                                console.debug("UI::forceUpdateArtworks # updating main listview")
                                imagesListView.forceLayout()
                                imagesListView.update()

                                // this piece of code is here in order to beat caching of ListView
                                // when you remove all items but few it fails to draw them
                                if (needToMoveCurrentItem) {
                                    console.debug("UI::forceUpdateArtworks # Moving into current item " + imagesListView.currentIndex)
                                    imagesListView.moveCurrentIndexDown()
                                    imagesListView.positionViewAtIndex(imagesListView.currentIndex, GridView.Visible)
                                }
                            }

                            add: Transition {
                                NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 230 }
                            }

                            remove: Transition {
                                NumberAnimation { property: "opacity"; to: 0; duration: 230 }
                            }

                            displaced: Transition {
                                NumberAnimation { properties: "x,y"; duration: 230 }
                            }

                            addDisplaced: Transition {
                                NumberAnimation { properties: "x,y"; duration: 230 }
                            }

                            removeDisplaced: Transition {
                                NumberAnimation { properties: "x,y"; duration: 230 }
                            }

                            delegate: Rectangle {
                                id: rowWrapper
                                objectName: "artworkDelegate"
                                property bool isHighlighted: (isselected || descriptionTextInput.activeFocus || flv.isFocused || titleTextInput.activeFocus)
                                color: isHighlighted ? Colors.selectedArtworkColor : Colors.artworkImageBackground
                                property var artworkModel: artItemsModel.getArtworkItself(rowWrapper.getIndex())
                                property int delegateIndex: index
                                width: applicationWindow.listLayout ? parent.width : 208
                                height: applicationWindow.listLayout ? (200 + 80*(settingsModel.keywordSizeScale - 1.0)) : 200

                                function getIndex() {
                                    return filteredArtItemsModel.getOriginalIndex(index)
                                }

                                function focusInputs() {
                                    descriptionTextInput.forceActiveFocus()
                                }

                                function isAnyFocused() {
                                    return descriptionTextInput.activeFocus || flv.isFocused || titleTextInput.activeFocus;
                                }

                                function focusIfNeeded() {
                                    if (!isAnyFocused()) {
                                        focusInputs()
                                    }

                                    if (!isselected) {
                                        descriptionTextInput.focus = false
                                        flv.removeFocus()
                                    }
                                }

                                function switchChecked() {
                                    editisselected = !isselected
                                    itemCheckedCheckbox.checked = isselected
                                    GridView.view.currentIndex = rowWrapper.delegateIndex
                                }

                                Connections {
                                    target: rowWrapper.artworkModel
                                    onSpellCheckResultsReady: {
                                        descriptionTextInput.deselect()
                                        if (columnLayout.isWideEnough) {
                                            titleTextInput.deselect()
                                        }
                                    }

                                    onFocusRequested: {
                                        if (directionSign === +1) {
                                            descriptionTextInput.forceActiveFocus()
                                        } else {
                                            flv.activateEdit()
                                        }

                                        imagesListView.positionViewAtIndex(rowWrapper.delegateIndex, ListView.Contain)
                                    }
                                }

                                Item {
                                    anchors.fill: parent

                                    Rectangle {
                                        id: isModifiedRectangle
                                        color: ismodified ? Colors.artworkModifiedColor : Colors.artworkSavedColor
                                        width: applicationWindow.listLayout ? 6 : 3
                                        anchors.left: parent.left
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                    }

                                    Item {
                                        id: checkboxSpacer
                                        width: applicationWindow.listLayout ? 15 : 8
                                        anchors.left: isModifiedRectangle.right
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom

                                        MouseArea {
                                            anchors.fill: parent
                                            enabled: applicationWindow.listLayout
                                            onClicked: {
                                                rowWrapper.switchChecked()
                                                rowWrapper.focusIfNeeded()
                                            }
                                        }
                                    }

                                    Item {
                                        id: checkboxRectangle
                                        anchors.left: checkboxSpacer.right
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom
                                        width: 20

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                rowWrapper.switchChecked()
                                                rowWrapper.focusIfNeeded()
                                            }
                                        }

                                        StyledCheckbox {
                                            id: itemCheckedCheckbox
                                            //checked: isselected
                                            focus: true
                                            anchors.left: parent.left
                                            anchors.verticalCenter: parent.verticalCenter
                                            activeFocusOnPress: true
                                            onClicked: editisselected = checked
                                            Component.onCompleted: itemCheckedCheckbox.checked = isselected
                                            Connections {
                                                target: filteredArtItemsModel
                                                onAllItemsSelectedChanged: {
                                                    itemCheckedCheckbox.checked = isselected
                                                }
                                            }
                                        }
                                    }

                                    Item {
                                        id: imageColumnWrapper
                                        width: applicationWindow.listLayout ? 180 : 160
                                        anchors.left: checkboxRectangle.right
                                        anchors.top: parent.top
                                        anchors.bottom: parent.bottom

                                        MouseArea {
                                            anchors.fill: parent
                                            onClicked: {
                                                rowWrapper.switchChecked()
                                                rowWrapper.focusIfNeeded()
                                            }
                                        }

                                        ColumnLayout {
                                            anchors.fill: parent
                                            anchors.leftMargin: applicationWindow.listLayout ? 10 : 0
                                            anchors.rightMargin: applicationWindow.listLayout ? 15 : 0
                                            spacing: 5

                                            Item {
                                                height: 30
                                            }

                                            Item {
                                                width: 150
                                                height: 132
                                                anchors.horizontalCenter: parent.horizontalCenter

                                                Image {
                                                    id: artworkImage
                                                    anchors.fill: parent
                                                    source: "image://global/" + filename
                                                    sourceSize.width: 150
                                                    sourceSize.height: 150
                                                    fillMode: settingsModel.fitSmallPreview ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                                                    asynchronous: true
                                                    cache: true
                                                }

                                                Image {
                                                    id: imageTypeIcon
                                                    visible: hasvectorattached
                                                    enabled: hasvectorattached
                                                    source: "qrc:/Graphics/vector-icon.svg"
                                                    sourceSize.width: 20
                                                    sourceSize.height: 20
                                                    anchors.left: artworkImage.left
                                                    anchors.bottom: artworkImage.bottom
                                                    cache: true
                                                }

                                                MouseArea {
                                                    anchors.fill: parent
                                                    onClicked: {
                                                        rowWrapper.switchChecked()
                                                        rowWrapper.focusIfNeeded()
                                                    }
                                                    onDoubleClicked: Common.launchItemEditing(rowWrapper.getIndex(), applicationWindow, {
                                                                                                  applyCallback: function() {}
                                                                                              })
                                                }
                                            }

                                            StyledText {
                                                Layout.fillWidth: true
                                                elide: Text.ElideMiddle
                                                color: moreInfoMA.pressed ? Colors.defaultLightColor : Colors.defaultInputBackground
                                                horizontalAlignment: Text.AlignHCenter
                                                text: filename.split(/[\\/]/).pop()

                                                MouseArea {
                                                    id: moreInfoMA
                                                    anchors.fill: parent
                                                    cursorShape: Qt.PointingHandCursor

                                                    onClicked: {
                                                        Common.launchDialog("Dialogs/ArtworkPreview.qml", applicationWindow,
                                                                            {
                                                                                imagePath: filename,
                                                                                artworkIndex: rowWrapper.getIndex()
                                                                            });
                                                    }
                                                }
                                            }

                                            Item {
                                                Layout.fillHeight: true
                                            }
                                        }
                                    }

                                    Rectangle {
                                        id: columnRectangle
                                        visible: applicationWindow.listLayout
                                        anchors.top: applicationWindow.listLayout ? parent.top : undefined
                                        anchors.bottom: applicationWindow.listLayout ? parent.bottom : undefined
                                        anchors.left: applicationWindow.listLayout ? imageColumnWrapper.right : undefined
                                        anchors.leftMargin: applicationWindow.listLayout ? 5 : 0
                                        anchors.right: applicationWindow.listLayout ? parent.right : undefined
                                        color: rowWrapper.isHighlighted  ? Colors.selectedMetadataColor : Colors.artworkBackground

                                        Item {
                                            id: columnLayout
                                            anchors.fill: parent
                                            anchors.margins: { left: 20; right: 20 }
                                            property bool isWideEnough: width > 400

                                            StyledText {
                                                id: descriptionText
                                                anchors.left: parent.left
                                                anchors.top: parent.top
                                                text: qsTr("Description:")
                                            }

                                            StyledText {
                                                id: titleText
                                                anchors.left: titleRect.left
                                                anchors.top: parent.top
                                                visible: columnLayout.isWideEnough
                                                text: qsTr("Title:")
                                            }

                                            Rectangle {
                                                id: descriptionRect
                                                height: 30
                                                anchors.left: parent.left
                                                anchors.right: titleRect.left
                                                anchors.rightMargin: columnLayout.isWideEnough ? 20 : 0
                                                anchors.top: descriptionText.bottom
                                                anchors.topMargin: 3
                                                color: rowWrapper.isHighlighted ? Colors.defaultInputBackground : Colors.defaultControlColor
                                                border.color: Colors.artworkActiveColor
                                                border.width: descriptionTextInput.activeFocus ? 1 : 0
                                                clip: true

                                                Flickable {
                                                    id: descriptionFlick
                                                    contentWidth: descriptionTextInput.paintedWidth
                                                    contentHeight: descriptionTextInput.paintedHeight
                                                    anchors.left: parent.left
                                                    anchors.right: parent.right
                                                    anchors.leftMargin: 5
                                                    anchors.rightMargin: 5
                                                    interactive: false
                                                    flickableDirection: Flickable.HorizontalFlick
                                                    height: 30
                                                    clip: true

                                                    function ensureVisible(r) {
                                                        if (contentX >= r.x)
                                                            contentX = r.x;
                                                        else if (contentX+width <= r.x+r.width)
                                                            contentX = r.x+r.width-width;
                                                    }

                                                    StyledTextEdit {
                                                        id: descriptionTextInput
                                                        width: descriptionFlick.width
                                                        height: descriptionFlick.height
                                                        text: description
                                                        focus: true
                                                        font.pixelSize: 12 * settingsModel.keywordSizeScale
                                                        color: rowWrapper.isHighlighted ? Colors.defaultLightColor : Colors.defaultInputBackground
                                                        onTextChanged: model.editdescription = text

                                                        Keys.onTabPressed: {
                                                            if (columnLayout.isWideEnough) {
                                                                titleTextInput.forceActiveFocus()
                                                            } else {
                                                                flv.activateEdit()
                                                            }
                                                        }

                                                        onActiveFocusChanged: {
                                                            if (descriptionTextInput.length > 0) {
                                                                filteredArtItemsModel.spellCheckDescription(rowWrapper.delegateIndex)
                                                            }
                                                        }

                                                        Keys.onBacktabPressed: {
                                                            filteredArtItemsModel.focusPreviousItem(rowWrapper.delegateIndex)
                                                        }

                                                        Keys.onPressed: {
                                                            if(event.matches(StandardKey.Paste)) {
                                                                var clipboardText = clipboard.getText();
                                                                if (Common.safeInsert(descriptionTextInput, clipboardText)) {
                                                                    event.accepted = true
                                                                }
                                                            } else if ((event.key === Qt.Key_Return) || (event.key === Qt.Key_Enter)) {
                                                                event.accepted = true
                                                            }
                                                        }

                                                        Component.onCompleted: {
                                                            var index = rowWrapper.getIndex()
                                                            artItemsModel.initDescriptionHighlighting(index, descriptionTextInput.textDocument)
                                                        }

                                                        onCursorRectangleChanged: descriptionFlick.ensureVisible(cursorRectangle)
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                id: titleRect
                                                height: 30
                                                width: columnLayout.isWideEnough ? ((columnLayout.width / 2) - 10 ): 0
                                                visible: columnLayout.isWideEnough
                                                anchors.right: parent.right
                                                anchors.top: descriptionText.bottom
                                                anchors.topMargin: 3
                                                color: rowWrapper.isHighlighted ? Colors.defaultInputBackground : Colors.defaultControlColor
                                                border.color: Colors.artworkActiveColor
                                                border.width: titleTextInput.activeFocus ? 1 : 0
                                                clip: true

                                                Flickable {
                                                    id: titleFlick
                                                    contentWidth: titleTextInput.paintedWidth
                                                    contentHeight: titleTextInput.paintedHeight
                                                    height: 30
                                                    anchors.left: parent.left
                                                    anchors.right: parent.right
                                                    anchors.leftMargin: 5
                                                    anchors.rightMargin: 5
                                                    clip: true
                                                    flickableDirection: Flickable.HorizontalFlick
                                                    interactive: false

                                                    function ensureVisible(r) {
                                                        if (contentX >= r.x)
                                                            contentX = r.x;
                                                        else if (contentX+width <= r.x+r.width)
                                                            contentX = r.x+r.width-width;
                                                    }

                                                    StyledTextEdit {
                                                        id: titleTextInput
                                                        font.pixelSize: 12 * settingsModel.keywordSizeScale
                                                        width: titleFlick.width
                                                        height: titleFlick.height
                                                        text: title
                                                        focus: true
                                                        color: rowWrapper.isHighlighted ? Colors.defaultLightColor : Colors.defaultInputBackground
                                                        onTextChanged: model.edittitle = text
                                                        KeyNavigation.backtab: descriptionTextInput

                                                        Keys.onTabPressed: {
                                                            flv.activateEdit()
                                                        }

                                                        onActiveFocusChanged: {
                                                            if (titleTextInput.length > 0) {
                                                                filteredArtItemsModel.spellCheckTitle(rowWrapper.delegateIndex)
                                                            }
                                                        }

                                                        Keys.onPressed: {
                                                            if (event.matches(StandardKey.Paste)) {
                                                                var clipboardText = clipboard.getText();
                                                                if (Common.safeInsert(titleTextInput, clipboardText)) {
                                                                    event.accepted = true
                                                                }
                                                            } else if ((event.key === Qt.Key_Return) || (event.key === Qt.Key_Enter)) {
                                                                event.accepted = true
                                                            }
                                                        }

                                                        Component.onCompleted: {
                                                            var index = rowWrapper.getIndex()
                                                            artItemsModel.initTitleHighlighting(index, titleTextInput.textDocument)
                                                        }

                                                        onCursorRectangleChanged: titleFlick.ensureVisible(cursorRectangle)
                                                    }
                                                }
                                            }

                                            StyledText {
                                                id: keywordsLabel
                                                anchors.left: parent.left
                                                anchors.top: descriptionRect.bottom
                                                anchors.topMargin: 7
                                                text: qsTr("Keywords:")
                                            }

                                            StyledText {
                                                id: plainTextText
                                                text: qsTr("<u>edit in plain text</u>")
                                                color: plainTextMA.containsMouse ? Colors.defaultLightGrayColor : Colors.defaultInputBackground
                                                visible: rowWrapper.isHighlighted
                                                anchors.right: parent.right
                                                anchors.top: descriptionRect.bottom
                                                anchors.topMargin: 7

                                                MouseArea {
                                                    id: plainTextMA
                                                    anchors.fill: parent
                                                    hoverEnabled: true
                                                    enabled: rowWrapper.isHighlighted
                                                    preventStealing: true
                                                    cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
                                                    onClicked: {
                                                        // strange bug with clicking on the keywords field
                                                        if (!containsMouse) { return; }

                                                        var callbackObject = {
                                                            onSuccess: function(text) {
                                                                artItemsModel.plainTextEdit(rowWrapper.getIndex(), text)
                                                            },
                                                            onClose: function() {
                                                                flv.activateEdit()
                                                            }
                                                        }

                                                        Common.launchDialog("Dialogs/PlainTextKeywordsDialog.qml",
                                                                            applicationWindow,
                                                                            {
                                                                                callbackObject: callbackObject,
                                                                                keywordsText: keywordsstring
                                                                            });
                                                    }
                                                }
                                            }

                                            Rectangle {
                                                id: keywordsWrapper
                                                anchors.top: keywordsLabel.bottom
                                                anchors.topMargin: 3
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                height: 80*settingsModel.keywordSizeScale
                                                border.color: Colors.artworkActiveColor
                                                border.width: flv.isFocused ? 1 : 0
                                                color: rowWrapper.isHighlighted ? Colors.defaultInputBackground : Colors.defaultControlColor

                                                function removeKeyword(index) {
                                                    artItemsModel.removeKeywordAt(rowWrapper.getIndex(), index)
                                                }

                                                function removeLastKeyword() {
                                                    artItemsModel.removeLastKeyword(rowWrapper.getIndex())
                                                }

                                                function appendKeyword(keyword) {
                                                    artItemsModel.appendKeyword(rowWrapper.getIndex(), keyword)
                                                }

                                                function pasteKeywords(keywords) {
                                                    artItemsModel.pasteKeywords(rowWrapper.getIndex(), keywords)
                                                }

                                                function saveKeywords() {
                                                    artItemsModel.backupItem(rowWrapper.getIndex())
                                                }

                                                EditableTags {
                                                    id: flv
                                                    model: rowWrapper.artworkModel
                                                    anchors.fill: parent
                                                    property int keywordHeight: 20 * settingsModel.keywordSizeScale + (settingsModel.keywordSizeScale - 1)*10
                                                    scrollStep: keywordHeight
                                                    stealWheel: false
                                                    focus: true

                                                    delegate: KeywordWrapper {
                                                        id: kw
                                                        isHighlighted: rowWrapper.isHighlighted
                                                        delegateIndex: index
                                                        keywordText: keyword
                                                        itemHeight: flv.keywordHeight
                                                        hasSpellCheckError: !iscorrect
                                                        onRemoveClicked: keywordsWrapper.removeKeyword(kw.delegateIndex)

                                                        onActionDoubleClicked: {
                                                            var callbackObject = {
                                                                onSuccess: function(replacement) {
                                                                    artItemsModel.editKeyword(rowWrapper.getIndex(), kw.delegateIndex, replacement)
                                                                },
                                                                onClose: function() {
                                                                    flv.activateEdit()
                                                                }
                                                            }

                                                            Common.launchDialog("Dialogs/EditKeywordDialog.qml",
                                                                                applicationWindow,
                                                                                {
                                                                                    callbackObject: callbackObject,
                                                                                    previousKeyword: keyword
                                                                                })
                                                        }
                                                    }

                                                    onTagAdded: {
                                                        keywordsWrapper.appendKeyword(text)
                                                    }

                                                    onTagsPasted: {
                                                        keywordsWrapper.pasteKeywords(tagsList)
                                                    }

                                                    onRemoveLast: {
                                                        keywordsWrapper.removeLastKeyword()
                                                    }

                                                    onBackTabPressed: {
                                                        if (columnLayout.isWideEnough) {
                                                            titleTextInput.forceActiveFocus()
                                                        } else {
                                                            descriptionTextInput.forceActiveFocus()
                                                        }
                                                    }

                                                    onTabPressed: {
                                                        filteredArtItemsModel.focusNextItem(rowWrapper.delegateIndex)
                                                    }

                                                    onFocusLost: keywordsWrapper.saveKeywords()

                                                    onCopyRequest: clipboard.setText(keywordsstring)
                                                }

                                                CustomScrollbar {
                                                    anchors.topMargin: -5
                                                    anchors.bottomMargin: -5
                                                    anchors.rightMargin: -15
                                                    flickable: flv
                                                }
                                            }

                                            RowLayout {
                                                anchors.left: parent.left
                                                anchors.right: parent.right
                                                anchors.top: keywordsWrapper.bottom
                                                anchors.topMargin: 3
                                                spacing: 5

                                                StyledText {
                                                    text: keywordscount
                                                    color: rowWrapper.isHighlighted ? Colors.defaultControlColor : Colors.selectedArtworkColor
                                                }

                                                Item {
                                                    Layout.fillWidth: true
                                                }

                                                StyledText {
                                                    id: fixSpellingText
                                                    text: qsTr("Fix spelling")
                                                    enabled: rowWrapper.artworkModel ? rowWrapper.artworkModel.hasSpellErrors : false
                                                    color: enabled ? Colors.artworkActiveColor : (rowWrapper.isHighlighted ? Colors.defaultInputBackground : Colors.selectedArtworkColor)

                                                    MouseArea {
                                                        anchors.fill: parent
                                                        cursorShape: enabled ? Qt.PointingHandCursor : Qt.ArrowCursor

                                                        onClicked: {
                                                            artItemsModel.suggestCorrections(rowWrapper.getIndex())
                                                            Common.launchDialog("Dialogs/SpellCheckSuggestionsDialog.qml",
                                                                                applicationWindow,
                                                                                {})
                                                        }
                                                    }
                                                }

                                                StyledText {
                                                    text: "|"
                                                    color: rowWrapper.isHighlighted ? Colors.defaultInputBackground : Colors.selectedMetadataColor
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                StyledText {
                                                    text: qsTr("Suggest")
                                                    color: suggestKeywordsMA.pressed ? Colors.defaultLightColor : Colors.artworkActiveColor

                                                    MouseArea {
                                                        id: suggestKeywordsMA
                                                        anchors.fill: parent
                                                        cursorShape: Qt.PointingHandCursor
                                                        onClicked: {
                                                            var callbackObject = {
                                                                promoteKeywords: function(keywords) {
                                                                    artItemsModel.addSuggestedKeywords(rowWrapper.getIndex(), keywords)
                                                                }
                                                            }

                                                            Common.launchDialog("Dialogs/KeywordsSuggestion.qml",
                                                                                applicationWindow,
                                                                                {callbackObject: callbackObject});
                                                        }
                                                    }
                                                }

                                                StyledText {
                                                    text: "|"
                                                    color: rowWrapper.isHighlighted ? Colors.defaultInputBackground : Colors.selectedMetadataColor
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                StyledText {
                                                    text: qsTr("Copy")
                                                    color: copyKeywordsMA.pressed ? Colors.defaultLightColor : Colors.artworkActiveColor

                                                    MouseArea {
                                                        id: copyKeywordsMA
                                                        anchors.fill: parent
                                                        cursorShape: Qt.PointingHandCursor
                                                        onClicked: clipboard.setText(keywordsstring)
                                                    }
                                                }

                                                StyledText {
                                                    text: "|"
                                                    color: rowWrapper.isHighlighted ? Colors.defaultInputBackground : Colors.selectedMetadataColor
                                                    verticalAlignment: Text.AlignVCenter
                                                }

                                                StyledText {
                                                    text: qsTr("Clear")
                                                    color: clearKeywordsMA.pressed ? Colors.defaultLightColor : Colors.artworkActiveColor

                                                    MouseArea {
                                                        id: clearKeywordsMA
                                                        anchors.fill: parent
                                                        cursorShape: Qt.PointingHandCursor
                                                        onClicked: filteredArtItemsModel.clearKeywords(rowWrapper.delegateIndex)
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }

                            Connections {
                                target: artItemsModel
                                onArtworksChanged: imagesListView.forceUpdateArtworks(needToMoveCurrentItem)
                            }

                            Connections {
                                target: filteredArtItemsModel
                                onAfterInvalidateFilter: imagesListView.forceUpdateArtworks()
                            }
                        }
                    }

                    Item {
                        visible: imagesListView.count == 0
                        anchors.fill: parent

                        RowLayout {
                            anchors.centerIn: parent
                            spacing: 5

                            StyledText {
                                text: qsTr("No items available.")
                                color: Colors.selectedMetadataColor
                            }

                            StyledText {
                                text: qsTr("Add files")
                                color: addFilesMA.pressed ? Colors.defaultLightColor : Colors.artworkActiveColor

                                MouseArea {
                                    id: addFilesMA
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: chooseArtworksDialog.open()
                                }
                            }

                            StyledText {
                                text: qsTr("or")
                                color: Colors.selectedMetadataColor
                            }

                            StyledText {
                                text: qsTr("clear the filter")
                                color: clearFilterMA.pressed ? Colors.defaultLightColor : Colors.artworkActiveColor

                                MouseArea {
                                    id: clearFilterMA
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: clearFilter()
                                }
                            }
                        }
                    }
                }
            }

            ClipboardHelper {
                id: clipboard
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

            StyledText {
                text: qsTr("Show logs")
                color: logsMA.pressed ? Colors.defaultInputBackground : Colors.selectedMetadataColor

                MouseArea {
                    id: logsMA
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        Common.launchDialog("Dialogs/LogsDialog.qml",
                                            applicationWindow,
                                            {
                                                logText: logsModel.getAllLogsText()
                                            });
                    }
                }
            }

            StyledText {
                text: "|"
                color: Colors.selectedMetadataColor
                verticalAlignment: Text.AlignVCenter
            }

            StyledText {
                text: qsTr("Check warnings")
                color: warningsMA.pressed ? Colors.defaultInputBackground : warningsModel.warningsCount > 0 ? Colors.artworkModifiedColor : Colors.selectedMetadataColor

                MouseArea {
                    id: warningsMA
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        warningsModel.update()
                        //filteredArtItemsModel.checkForWarnings()
                        Common.launchDialog("Dialogs/WarningsDialog.qml", applicationWindow, {
                                                componentParent: applicationWindow
                                            });
                    }
                }
            }

            StyledText {
                visible: applicationWindow.showUpdateLink
                text: "|"
                color: Colors.selectedMetadataColor
                verticalAlignment: Text.AlignVCenter
            }

            StyledText {
                visible: applicationWindow.showUpdateLink
                enabled: applicationWindow.showUpdateLink
                text: qsTr("Update available!")
                color: updateMA.pressed ? Colors.defaultInputBackground : Colors.greenColor

                MouseArea {
                    id: updateMA
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        Qt.openUrlExternally("http://ribtoks.github.io/xpiks/downloads/")
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }

            StyledText {
                id: filteredCountText
                text: qsTr("No items available")
                color: Colors.selectedMetadataColor
                verticalAlignment: Text.AlignVCenter

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
            }

            StyledText {
                text: "|"
                color: Colors.selectedMetadataColor
                verticalAlignment: Text.AlignVCenter
            }

            StyledText {
                text: filteredArtItemsModel.selectedArtworksCount > 1 ? qsTr("%1 selected items").arg(filteredArtItemsModel.selectedArtworksCount) : (filteredArtItemsModel.selectedArtworksCount === 1 ? qsTr("1 selected item") : qsTr("No selected items"))
                color: Colors.selectedMetadataColor
                verticalAlignment: Text.AlignVCenter

                MouseArea {
                    id: selectSelectedMA
                    anchors.fill: parent
                    cursorShape: filteredArtItemsModel.selectedArtworksCount > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onClicked: {
                        if (filteredArtItemsModel.selectedArtworksCount > 0) {
                            filteredArtItemsModel.searchTerm = "x:selected"
                        }
                    }
                }
            }

            StyledText {
                text: "|"
                color: Colors.selectedMetadataColor
                verticalAlignment: Text.AlignVCenter
            }

            StyledText {
                text: artItemsModel.modifiedArtworksCount > 1 ? qsTr("%1 modified items").arg(artItemsModel.modifiedArtworksCount) : (artItemsModel.modifiedArtworksCount === 1 ? qsTr("1 modified item") : qsTr("No modified items"))
                verticalAlignment: Text.AlignVCenter
                color: artItemsModel.modifiedArtworksCount > 0 ? Colors.artworkModifiedColor : Colors.selectedMetadataColor

                MouseArea {
                    id: selectModifiedMA
                    anchors.fill: parent
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
                color: Colors.defaultDarkColor
            }
        }
    }
}
