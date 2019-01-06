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
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import xpiks 1.0
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: replacePreviewComponent
    anchors.fill: parent
    property variant componentParent
    property variant replaceModel: dispatcher.getCommandTarget(UICommand.FindReplaceCandidates)
    property bool isRestricted: false

    Keys.onEscapePressed: closePopup()
    Keys.onReturnPressed: replaceModel.replace()
    Keys.onEnterPressed: replaceModel.replace()

    onClickedOutside: closePopup()

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.ReplaceInFound]
        onDispatched: closePopup()
    }

    contentsWidth: 700
    contentsHeight: 580

    contents: ColumnLayout {
        spacing: 10
        anchors.fill: parent
        anchors.margins: 20

        RowLayout {
            spacing: 5

            StyledText {
                text: i18.n + qsTr("Replace", "caption")
            }

            StyledText {
                text: '"' + replaceModel.replaceFrom + '"'
            }

            StyledText {
                text: i18.n + qsTr("with")
            }

            StyledText {
                text: '"' + replaceModel.replaceTo + '"'
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: uiColors.defaultControlColor

            StyledScrollView {
                anchors.fill: parent
                anchors.margins: 10

                ListView {
                    id: replacePreviewList
                    objectName: "replacePreviewList"
                    model: replaceModel
                    spacing: 5

                    delegate: Rectangle {
                        id: imageWrapper
                        objectName: "imageDelegate"
                        property int delegateIndex: index
                        color: isselected ? uiColors.selectedArtworkBackground : uiColors.artworkBackground
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        height: columnRectangle.height

                        StyledCheckbox {
                            id: applyReplaceCheckBox
                            objectName: "applyReplaceCheckBox"
                            anchors.leftMargin: 14
                            anchors.left: parent.left
                            width: 20
                            anchors.verticalCenter: parent.verticalCenter
                            onClicked: editisselected = checked
                            Component.onCompleted: applyReplaceCheckBox.checked = isselected

                            Connections {
                                target: replaceModel
                                onAllSelectedChanged: {
                                    applyReplaceCheckBox.checked = isselected
                                }
                            }
                        }

                        Item {
                            id: imageItem
                            anchors.leftMargin: 0
                            anchors.left: applyReplaceCheckBox.right
                            anchors.top: parent.top
                            width: 120
                            height: parent.height

                            ColumnLayout {
                                anchors.centerIn: parent
                                anchors.verticalCenterOffset: 7
                                spacing: 7
                                width: 110

                                Item {
                                    id: imageContainer
                                    width: 100
                                    height: 100
                                    Layout.alignment: Qt.AlignHCenter

                                    Image {
                                        id: artworkImage
                                        anchors.fill: parent
                                        source: "image://cached/" + path
                                        sourceSize.width: 150
                                        sourceSize.height: 150
                                        fillMode: settingsModel.fitSmallPreview ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                                        asynchronous: true
                                        cache: false
                                    }

                                    Rectangle {
                                        anchors.right: artworkImage.right
                                        anchors.bottom: artworkImage.bottom
                                        width: 20
                                        height: 20
                                        color: uiColors.defaultDarkColor
                                        property bool isVideo: isvideo
                                        property bool isVector: hasvectorattached
                                        visible: isVideo || isVector
                                        enabled: isVideo || isVector

                                        Image {
                                            id: typeIcon
                                            anchors.fill: parent
                                            source: parent.isVector ? "qrc:/Graphics/vector-icon.svg" : (parent.isVideo ? "qrc:/Graphics/video-icon.svg" : "")
                                            sourceSize.width: 20
                                            sourceSize.height: 20
                                            cache: true
                                        }
                                    }

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            editisselected = !isselected
                                            applyReplaceCheckBox.checked = isselected
                                        }
                                    }
                                }

                                StyledText {
                                    Layout.fillWidth: true
                                    elide: Text.ElideMiddle
                                    horizontalAlignment: Text.AlignHCenter
                                    text: path.split(/[\\/]/).pop()
                                    font.pixelSize: UIConfig.fontPixelSize
                                }

                                Item {
                                    Layout.fillHeight: true
                                }
                            }
                        }

                        Rectangle {
                            id: columnRectangle
                            anchors.left: imageItem.right
                            anchors.top: parent.top
                            anchors.right: parent.right
                            height: (childrenRect.height < 130) ? 150 : (childrenRect.height + 20)
                            color: isselected ? uiColors.selectedArtworkBackground : uiColors.artworkBackground

                            Item {
                                clip: true
                                anchors.left: parent.left
                                anchors.top: parent.top
                                anchors.right: parent.right
                                anchors.leftMargin: 5
                                anchors.rightMargin: 10
                                anchors.topMargin: 10
                                height: childrenRect.height

                                StyledText {
                                    id: titleHit
                                    text: i18.n + qsTr("Title:")
                                    isActive: isselected
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                }

                                Rectangle {
                                    id: titleRectangle
                                    height: childrenRect.height + 10
                                    color: (isselected && enabled) ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                                    anchors.right: parent.right
                                    anchors.left: parent.left
                                    anchors.top: titleHit.bottom
                                    anchors.topMargin: 3
                                    enabled: hastitle
                                    opacity: (isselected && enabled) ? 1 : 0.5

                                    StyledTextEdit {
                                        id: titleText
                                        anchors.top: parent.top
                                        anchors.topMargin: 5
                                        anchors.right: parent.right
                                        anchors.left: parent.left
                                        anchors.leftMargin: 5
                                        anchors.rightMargin: 5
                                        wrapMode: TextEdit.Wrap
                                        text: i18.n + replaceModel.getSearchTitle(imageWrapper.delegateIndex)
                                        readOnly: true
                                        selectByKeyboard: false
                                        selectByMouse: false
                                        isActive: isselected
                                        Component.onCompleted: {
                                            replaceModel.initHighlighting(titleText.textDocument)
                                        }
                                    }
                                }

                                StyledText {
                                    id: descriptionHit
                                    text: i18.n + qsTr("Description:")
                                    isActive: isselected
                                    anchors.left: parent.left
                                    anchors.top: titleRectangle.bottom
                                    anchors.topMargin: 10
                                }

                                Rectangle {
                                    id: descriptionRectangle
                                    color: (isselected && enabled) ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                                    height: childrenRect.height + 10
                                    anchors.top: descriptionHit.bottom
                                    anchors.topMargin: 3
                                    anchors.right: parent.right
                                    anchors.left: parent.left
                                    enabled: hasdescription
                                    opacity: (isselected && enabled) ? 1 : 0.5

                                    StyledTextEdit {
                                        id: descriptionText
                                        wrapMode: TextEdit.Wrap
                                        anchors.top: parent.top
                                        anchors.topMargin: 5
                                        anchors.right: parent.right
                                        anchors.left: parent.left
                                        anchors.leftMargin: 5
                                        anchors.rightMargin: 5
                                        text: i18.n + replaceModel.getSearchDescription(imageWrapper.delegateIndex).trim()
                                        readOnly: true
                                        selectByKeyboard: false
                                        selectByMouse: false
                                        isActive: isselected
                                        Component.onCompleted: {
                                            replaceModel.initHighlighting(descriptionText.textDocument)
                                        }
                                    }
                                }

                                StyledText {
                                    id: keywordsHit
                                    text: i18.n + qsTr("Keywords:")
                                    isActive: isselected
                                    anchors.left: parent.left
                                    anchors.top: descriptionRectangle.bottom
                                    anchors.topMargin: 10
                                }

                                Rectangle {
                                    id: keywordsRectangle
                                    color: (isselected && enabled) ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                                    height: childrenRect.height + 10
                                    anchors.right: parent.right
                                    anchors.left: parent.left
                                    enabled: haskeywords
                                    anchors.top: keywordsHit.bottom
                                    anchors.topMargin: 3
                                    opacity: (isselected && enabled) ? 1 : 0.5

                                    StyledTextEdit {
                                        id: keywordsText
                                        anchors.top: parent.top
                                        anchors.topMargin: 5
                                        anchors.right: parent.right
                                        anchors.left: parent.left
                                        anchors.leftMargin: 5
                                        anchors.rightMargin: 5
                                        wrapMode: TextEdit.Wrap
                                        text: i18.n + replaceModel.getSearchKeywords(imageWrapper.delegateIndex)
                                        readOnly: true
                                        selectByKeyboard: false
                                        selectByMouse: false
                                        isActive: isselected

                                        Component.onCompleted: {
                                            replaceModel.initHighlighting(keywordsText.textDocument)
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                anchors.fill: parent
                visible: replaceModel.count == 0

                StyledText {
                    text: i18.n + qsTr("Nothing found")
                    anchors.centerIn: parent
                    color: uiColors.selectedArtworkBackground
                }
            }
        }

        Item {
            height: 1
        }

        RowLayout {
            spacing: 20
            height: 24

            StyledButton {
                text: i18.n + qsTr("Select all")
                width: 100
                enabled: replaceModel.count > 0
                onClicked: {
                    replaceModel.selectAll();
                }
            }

            StyledButton {
                text: i18.n + qsTr("Unselect all")
                width: 100
                enabled: replaceModel.count > 0
                onClicked: {
                    replaceModel.unselectAll();
                }
            }

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Replace", "button")
                objectName: "replaceButton"
                enabled: replaceModel.count > 0
                width: 100
                onClicked: {
                    dispatcher.dispatch(UICommand.ReplaceInFound, true)
                }
            }

            StyledButton {
                text: i18.n + qsTr("Close")
                objectName: "cancelReplaceButton"
                width: 100
                onClicked: {
                    dispatcher.dispatch(UICommand.ReplaceInFound, false)
                }
            }
        }
    }
}
