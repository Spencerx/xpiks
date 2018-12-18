/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import QtQuick.Controls.Styles 1.1
import QtGraphicalEffects 1.0
import xpiks 1.0
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: deleteKeywordsComponent
    anchors.fill: parent

    property variant componentParent
    property var deleteKeywordsModel: dispatcher.getCommandTarget(UICommand.SetupDeleteKeywordsInSelected)

    function mustUseConfirmation() {
        return settingsModel.mustUseConfirmations
    }

    Connections {
        target: xpiksApp
        onGlobalBeforeDestruction: {
            console.debug("UI:DeleteKeywordsDialog # global CloseRequested")
            closePopup()
        }
    }

    Connections {
        target: deleteKeywordsModel
        onRequestCloseWindow: closePopup()
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.DeleteKeywordsInSelected]
        onDispatched: closePopup()
    }

    PropertyAnimation { target: deleteKeywordsComponent; property: "opacity";
        duration: 400; from: 0; to: 1;
        easing.type: Easing.InOutQuad ; running: true }

    MessageDialog {
        id: confirmRemoveArtworksDialog
        property int itemsCount
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Are you sure you want to remove %1 item(s)?").arg(itemsCount)
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            doRemoveSelectedArtworks()
        }
    }

    function doRemoveSelectedArtworks() {
        deleteKeywordsModel.removeSelectedArtworks()
    }

    MessageDialog {
        id: clearKeywordsDialog

        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Clear all keywords?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: deleteKeywordsModel.clearKeywordsToDelete()
    }

    Menu {
        id: presetsMenu

        Menu {
            id: subMenu
            property var defaultGroupModel: presetsGroups.getDefaultGroupModel()
            title: i18.n + qsTr("Insert preset")

            Instantiator {
                model: presetsGroups
                onObjectAdded: subMenu.insertItem( index, object )
                onObjectRemoved: subMenu.removeItem( object )
                delegate: Menu {
                    id: groupMenu
                    property int delegateIndex: index
                    property var groupModel: presetsGroups.getGroupModel(index)
                    title: gname

                    Instantiator {
                        model: groupMenu.groupModel
                        onObjectAdded: groupMenu.insertItem( index, object )
                        onObjectRemoved: groupMenu.removeItem( object )

                        delegate: MenuItem {
                            text: name
                            onTriggered: {
                                deleteKeywordsModel.addPreset(groupMenu.groupModel.getOriginalID(index))
                            }
                        }
                    }
                }
            }

            Instantiator {
                model: subMenu.defaultGroupModel
                onObjectAdded: subMenu.insertItem( index, object )
                onObjectRemoved: subMenu.removeItem( object )

                delegate: MenuItem {
                    text: name
                    onTriggered: {
                        deleteKeywordsModel.addPreset(subMenu.defaultGroupModel.getOriginalID(index))
                    }
                }
            }
        }
    }

    contentsWidth: 730
    contentsHeight: 610
    contents: ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: 20
        anchors.rightMargin: 20
        anchors.topMargin: 20
        anchors.bottomMargin: 20
        spacing: 0

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            StyledText {
                text: i18.n + qsTr("Delete keywords")
            }

            Item {
                Layout.fillWidth: true
            }

            StyledText {
                id: textItemsAvailable
                text: i18.n + getOriginalText()

                function getOriginalText() {
                    return deleteKeywordsModel.artworksCount === 1 ? qsTr("1 artwork selected") : qsTr("%1 artworks selected").arg(deleteKeywordsModel.artworksCount)
                }

                Connections {
                    target: deleteKeywordsModel
                    onArtworksCountChanged: {
                        var originalText = textItemsAvailable.getOriginalText();
                        textItemsAvailable.text = i18.n + originalText
                    }
                }
            }
        }

        Item {
            height: 15
        }

        RowLayout {
            spacing: 5
            anchors.left: parent.left
            anchors.right: parent.right
            height: 40

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Remove selected")
                width: 150
                tooltip: i18.n + qsTr("Remove selected artworks from this dialog")
                enabled: deleteKeywordsModel.selectedArtworksCount > 0
                onClicked: {
                    if (mustUseConfirmation()) {
                        confirmRemoveArtworksDialog.itemsCount = deleteKeywordsModel.selectedArtworksCount
                        confirmRemoveArtworksDialog.open()
                    } else {
                        doRemoveSelectedArtworks()
                    }
                }
            }
        }

        Item {
            height: 10
        }

        Item {
            height: 130
            anchors.left: parent.left
            anchors.right: parent.right

            Rectangle {
                anchors.fill: parent
                color: uiColors.defaultControlColor

                StyledScrollView {
                    id: imagesScrollView
                    height: parent.height + 15
                    width: parent.width
                    anchors.margins: 10

                    ListView {
                        boundsBehavior: Flickable.StopAtBounds
                        anchors.fill: parent
                        anchors.margins: 10
                        orientation: Qt.Horizontal
                        spacing: 10
                        model: deleteKeywordsModel

                        displaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        addDisplaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        removeDisplaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        delegate: Item {
                            property int delegateIndex: index
                            id: imageWrapper
                            height: 110
                            width: height

                            Image {
                                id: artworkImage
                                anchors.fill: parent
                                anchors.margins: 1
                                source: "image://cached/" + thumbpath
                                sourceSize.width: 150
                                sourceSize.height: 150
                                fillMode: settingsModel.fitSmallPreview ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                                asynchronous: true
                                cache: false
                            }

                            Image {
                                id: videoTypeIconSmall
                                visible: isvideo
                                enabled: isvideo
                                source: "qrc:/Graphics/video-icon-s.png"
                                fillMode: Image.PreserveAspectFit
                                sourceSize.width: 150
                                sourceSize.height: 150
                                anchors.fill: artworkImage
                                cache: true
                            }

                            Rectangle {
                                anchors.fill: parent
                                color: uiColors.defaultControlColor
                                opacity: isselected ? (mouseArea.containsMouse ? 0.6 : 0.7) : (mouseArea.containsMouse ? 0.4 : 0)
                            }

                            SelectedIcon {
                                opacity: isselected ? (mouseArea.containsMouse ? 0.85 : 1) : (mouseArea.containsMouse ? 0.6 : 0)
                                width: parent.width * 0.33
                                height: parent.height * 0.33
                                anchors.centerIn: parent
                            }

                            MouseArea {
                                id: mouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: {
                                    deleteKeywordsModel.setArtworkSelected(delegateIndex, !isselected)
                                }
                            }
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 120

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 0
                spacing: 0

                RowLayout {
                    spacing: 5

                    StyledText {
                        id: keywordsLabel
                        text: i18.n + qsTr("Keywords to delete:")
                    }

                    StyledText {
                        text: i18.n + qsTr("(comma-separated)")
                        isActive: false
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledText {
                        text: deleteKeywordsModel.keywordsToDeleteCount
                    }
                }

                Item {
                    height: 5
                }

                Rectangle {
                    id: keywordsToDeleteWrapper
                    border.color: uiColors.artworkActiveColor
                    border.width: flv.isFocused ? 1 : 0
                    height: 80
                    anchors.rightMargin: 20
                    Layout.fillWidth: true
                    color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                    property var keywordsModel: deleteKeywordsModel.getKeywordsToDeleteObject()

                    function removeKeyword(index) {
                        deleteKeywordsModel.removeKeywordToDeleteAt(index)
                    }

                    function removeLastKeyword() {
                        deleteKeywordsModel.removeLastKeywordToDelete()
                    }

                    function appendKeyword(keyword) {
                        deleteKeywordsModel.appendKeywordToDelete(keyword)
                    }

                    function pasteKeywords(keywordsList) {
                        deleteKeywordsModel.pasteKeywordsToDelete(keywordsList)
                    }

                    EditableTags {
                        id: flv
                        objectName: "deleteEditableTags"
                        anchors.fill: parent
                        model: keywordsToDeleteWrapper.keywordsModel
                        property int keywordHeight: uiManager.keywordHeight
                        scrollStep: keywordHeight

                        delegate: KeywordWrapper {
                            id: kw
                            keywordText: keyword
                            hasSpellCheckError: !iscorrect
                            hasDuplicate: hasduplicate
                            delegateIndex: index
                            isHighlighted: true
                            itemHeight: flv.keywordHeight
                            onRemoveClicked: keywordsToDeleteWrapper.removeKeyword(delegateIndex)
                            onActionDoubleClicked: {
                                // DO NOTHING
                            }
                        }

                        onTagAdded: {
                            keywordsToDeleteWrapper.appendKeyword(text)
                        }

                        onRemoveLast: {
                            keywordsToDeleteWrapper.removeLastKeyword()
                        }

                        onTagsPasted: {
                            keywordsToDeleteWrapper.pasteKeywords(tagsList)
                        }

                        onBackTabPressed: {
                            // BUMP
                        }

                        onCompletionRequested: {
                            // BUMP
                        }

                        onRightClickedInside: {
                            presetsMenu.popup()
                        }
                    }

                    CustomScrollbar {
                        anchors.topMargin: -5
                        anchors.bottomMargin: -5
                        anchors.rightMargin: -15
                        flickable: flv
                    }
                }

                Item { height: 5 }

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: childrenRect.height

                    StyledLink {
                        anchors.top: parent.top
                        anchors.rightMargin: 3
                        anchors.right: parent.right
                        text: i18.n + qsTr("Clear")
                        enabled: deleteKeywordsModel.keywordsToDeleteCount > 0
                        onClicked: {
                            if (deleteKeywordsModel.keywordsToDeleteCount > 0) {
                                clearKeywordsDialog.open()
                            }
                        }
                    }
                }
            }
        }

        Item {
            height: 10
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 175

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 0
                spacing: 5

                RowLayout {
                    spacing: 5

                    StyledText {
                        text: i18.n + qsTr("Common keywords:")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledText {
                        text: deleteKeywordsModel.commonKeywordsCount
                    }
                }

                Rectangle {
                    id: commonKeywordsWrapper
                    border.width: 0
                    height: 155
                    anchors.rightMargin: 20
                    Layout.fillWidth: true
                    color: uiColors.inputInactiveBackground
                    property var keywordsModel: deleteKeywordsModel.getCommonKeywordsObject()

                    function removeKeyword(index) {
                        var keyword = deleteKeywordsModel.removeCommonKeywordAt(index)
                        if (keyword.length !== 0) {
                            deleteKeywordsModel.appendKeywordToDelete(keyword)
                        }
                    }

                    EditableTags {
                        id: flvCommon
                        objectName: "commonEditableTags"
                        anchors.fill: parent
                        model: commonKeywordsWrapper.keywordsModel
                        property int keywordHeight: uiManager.keywordHeight
                        scrollStep: keywordHeight
                        editEnabled: false

                        delegate: KeywordWrapper {
                            keywordText: keyword
                            hasSpellCheckError: !iscorrect
                            hasDuplicate: hasduplicate
                            delegateIndex: index
                            isHighlighted: false
                            hasPlusSign: true
                            itemHeight: flvCommon.keywordHeight
                            onRemoveClicked: commonKeywordsWrapper.removeKeyword(delegateIndex)
                            onActionDoubleClicked: {
                                // DO NOTHING
                            }
                        }

                        onTagAdded: {
                            // BUMP
                        }

                        onRemoveLast: {
                            // BUMP
                        }

                        onTagsPasted: {
                            // BUMP
                        }

                        onBackTabPressed: {
                            // BUMP
                        }

                        onCompletionRequested: {
                            // BUMP
                        }
                    }

                    CustomScrollbar {
                        anchors.topMargin: -5
                        anchors.bottomMargin: -5
                        anchors.rightMargin: -15
                        flickable: flvCommon
                    }
                }
            }
        }

        Item {
            height: 20
        }

        Item {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 24

            RowLayout {
                anchors.fill: parent
                spacing: 20

                StyledCheckbox {
                    id: caseSensitiveCheckbox
                    text: i18.n + qsTr("Case sensitive")
                    Component.onCompleted: caseSensitiveCheckbox.checked = deleteKeywordsModel.caseSensitive
                    onClicked: {
                        deleteKeywordsModel.caseSensitive = caseSensitiveCheckbox.checked
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                StyledButton {
                    text: i18.n + qsTr("Delete")
                    width: 100
                    onClicked: {
                        flv.onBeforeClose()
                        dispatcher.dispatch(UICommand.DeleteKeywordsInSelected, true)
                    }
                }

                StyledButton {
                    text: i18.n + qsTr("Cancel")
                    width: 100
                    onClicked: {
                        dispatcher.dispatch(UICommand.DeleteKeywordsInSelected, false)
                    }

                    tooltip: qsTr("Exit without changes", "tooltip")
                }
            }
        }
    }

    ClipboardHelper {
        id: clipboard
    }

    Component.onCompleted: {
        focus = true
        flv.activateEdit()
    }
}
