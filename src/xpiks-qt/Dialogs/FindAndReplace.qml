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
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import "../Constants"
import "../Common.js" as Common
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

Item {
    id: replaceSetupComponent
    property var filteredModel
    anchors.fill: parent

    function closePopup() {
        replaceModel.resetModel();
        replaceSetupComponent.destroy()
    }

    Keys.onEscapePressed: closePopup()


    signal dialogDestruction();
    Component.onDestruction: dialogDestruction();

    Connections {
        target: replaceModel
        onReplaceSucceeded: closePopup()
    }

    function launchReplacePreview() {
        if (!replaceModel.anySearchDestination()) {
            noSearchDestinationMsgDialog.open()
            return
        }

        replaceModel.initArtworksList()
        Common.launchDialog("Dialogs/ReplacePreview.qml",
                            replaceSetupComponent,
                            {
                                componentParent: replaceSetupComponent
                            })
    }

    function onDialogClosed() {
        replaceToTextInput.forceActiveFocus()
        replaceToTextInput.cursorPosition = replaceToTextInput.length
    }

    MessageDialog {
        id: noSearchDestinationMsgDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please select where to search for")
        standardButtons: StandardButton.Ok
    }

    // This rectange is the a overlay to partially show the parent through it
    // and clicking outside of the 'dialog' popup will do 'nothing'
    Rectangle {
        anchors.fill: parent
        id: overlay
        color: "#000000"
        opacity: 0.6
        // add a mouse area so that clicks outside
        // the dialog window will not do anything
        MouseArea {
            anchors.fill: parent
        }
    }

    FocusScope {
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onWheel: wheel.accepted = true
            onClicked: mouse.accepted = true
            onDoubleClicked: mouse.accepted = true

            property real old_x: 0
            property real old_y: 0

            onPressed: {
                var tmp = mapToItem(replaceSetupComponent, mouse.x, mouse.y)
                old_x = tmp.x
                old_y = tmp.y
            }

            onPositionChanged: {
                var old_xy = Common.movePopupInsideComponent(
                            replaceSetupComponent, dialogWindow, mouse,
                            old_x, old_y)
                old_x = old_xy[0]
                old_y = old_xy[1]
            }
        }

        RectangularGlow {
            anchors.fill: dialogWindow
            anchors.topMargin: glowRadius / 2
            anchors.bottomMargin: -glowRadius / 2
            glowRadius: 4
            spread: 0.0
            color: uiColors.popupGlowColor
            cornerRadius: glowRadius
        }

        // This rectangle is the actual popup
        Rectangle {
            id: dialogWindow
            width: 480
            height: 240
            property int inputsWidth: 200
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                Item {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: childrenRect.height

                    ColumnLayout {
                        id: inputsRow
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        width: dialogWindow.inputsWidth
                        spacing: 0

                        StyledText {
                            text: i18.n + qsTr("Search for:")
                        }

                        Item {
                            height: 5
                        }

                        Rectangle {
                            color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                            border.width: (replaceFromTextInput.activeFocus) ? 1 : 0
                            border.color: uiColors.artworkActiveColor
                            width: dialogWindow.inputsWidth
                            height: UIConfig.textInputHeight
                            clip: true

                            StyledTextInput {
                                id: replaceFromTextInput
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.leftMargin: 5
                                anchors.rightMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                                KeyNavigation.tab: replaceToTextInput
                                onTextChanged: replaceModel.replaceFrom = text
                                Component.onCompleted: replaceFromTextInput.text = replaceModel.replaceFrom
                            }
                        }

                        Item {
                            height: 40
                        }

                        StyledText {
                            text: i18.n + qsTr("Replace with:")
                        }

                        Item {
                            height: 5
                        }

                        Rectangle {
                            color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                            border.width: (replaceToTextInput.activeFocus) ? 1 : 0
                            border.color: uiColors.artworkActiveColor
                            width: dialogWindow.inputsWidth
                            height: UIConfig.textInputHeight
                            clip: true

                            StyledTextInput {
                                id: replaceToTextInput
                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.leftMargin: 5
                                anchors.rightMargin: 5
                                anchors.verticalCenter: parent.verticalCenter
                                KeyNavigation.backtab: replaceFromTextInput
                                onTextChanged: replaceModel.replaceTo = text
                                onAccepted: launchReplacePreview()
                                Component.onCompleted: replaceToTextInput.text = replaceModel.replaceTo
                            }
                        }
                    }

                    ColumnLayout {
                        id: checkboxesColumn
                        anchors.left: inputsRow.right
                        anchors.leftMargin: 30
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        spacing: 15

                        StyledCheckbox {
                            id: searchInTitleCheckbox
                            text: i18.n + qsTr("Search in title")
                            Component.onCompleted: searchInTitleCheckbox.checked = replaceModel.searchInTitle
                            onClicked: {
                                replaceModel.searchInTitle = searchInTitleCheckbox.checked
                            }
                        }

                        StyledCheckbox {
                            id: searchInDescriptionCheckbox
                            text: i18.n + qsTr("Search in description")
                            Component.onCompleted: searchInDescriptionCheckbox.checked = replaceModel.searchInDescription
                            onClicked: {
                                replaceModel.searchInDescription = searchInDescriptionCheckbox.checked
                            }
                        }

                        StyledCheckbox {
                            id: searchInKeywordsCheckbox
                            text: i18.n + qsTr("Search in keywords")
                            Component.onCompleted: searchInKeywordsCheckbox.checked = replaceModel.searchInKeywords
                            onClicked: {
                                replaceModel.searchInKeywords = searchInKeywordsCheckbox.checked
                            }
                        }

                        StyledCheckbox {
                            id: caseSensitiveCheckbox
                            text: i18.n + qsTr("Case sensitive")
                            Component.onCompleted: caseSensitiveCheckbox.checked = replaceModel.caseSensitive
                            onClicked: {
                                replaceModel.caseSensitive = caseSensitiveCheckbox.checked
                            }
                        }

                        StyledCheckbox {
                            id: wholeWordsCheckbox
                            text: i18.n + qsTr("Whole words only")
                            Component.onCompleted: wholeWordsCheckbox.checked = replaceModel.searchWholeWords
                            onClicked: {
                                replaceModel.searchWholeWords = wholeWordsCheckbox.checked
                            }
                        }
                    }
                }

                Item {
                    Layout.fillHeight: true
                }

                RowLayout {
                    id: submitRow
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 24
                    spacing: 20

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledButton {
                        id: replaceButton
                        text: i18.n + qsTr("Find and replace")
                        enabled: (replaceFromTextInput.length > 0)
                        width: 150
                        onClicked: {
                            launchReplacePreview()
                        }
                    }

                    StyledButton {
                        text: i18.n + qsTr("Cancel")
                        width: 100
                        onClicked: {
                            closePopup()
                        }
                    }
                }
            }
        }
    }

    Component.onCompleted: {
        focus = true
        replaceFromTextInput.forceActiveFocus()
    }
}
