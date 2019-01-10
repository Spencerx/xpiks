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
import xpiks 1.0
import "../Constants"
import "../Common.js" as Common
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: replaceSetupComponent
    objectName: "findSetupDialog"
    canEscapeClose: false
    property variant componentParent
    property var filteredModel
    property var replaceModel: dispatcher.getCommandTarget(UICommand.FindReplaceCandidates)
    anchors.fill: parent

    function launchReplacePreview() {
        if (!replaceModel.anySearchDestination()) {
            noSearchDestinationMsgDialog.open()
            return
        }

        dispatcher.dispatch(UICommand.FindReplaceCandidates, {})
    }

    MessageDialog {
        id: noSearchDestinationMsgDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please select where to search for")
        standardButtons: StandardButton.Ok
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.FindReplaceCandidates]
        onDispatched: {
            Common.launchDialog("Dialogs/ReplacePreview.qml",
                                replaceSetupComponent.componentParent,
                                {
                                    componentParent: replaceSetupComponent.componentParent
                                })
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.ReplaceInFound]
        onDispatched: {
            if (value) {
                closePopup()
            } else {
                replaceToTextInput.forceActiveFocus()
                replaceToTextInput.cursorPosition = replaceToTextInput.length
            }
        }
    }

    contentsWidth: 480
    contentsHeight: 240

    contents: ColumnLayout {
        id: column
        property int inputsWidth: 200
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        Item {
            Layout.fillWidth: true
            height: childrenRect.height

            ColumnLayout {
                id: inputsRow
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: column.inputsWidth
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
                    width: column.inputsWidth
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
                    width: column.inputsWidth
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
                    objectName: "searchDescriptionCheckbox"
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
            Layout.fillWidth: true
            height: 24
            spacing: 20

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                id: replaceButton
                text: i18.n + qsTr("Find and replace")
                objectName: "findButton"
                enabled: (replaceFromTextInput.length > 0)
                width: 150
                onClicked: {
                    launchReplacePreview()
                }
            }

            StyledButton {
                text: i18.n + qsTr("Cancel")
                objectName: "cancelFindButton"
                width: 100
                onClicked: {
                    closePopup()
                }
            }
        }
    }

    Component.onCompleted: {
        focus = true
        replaceFromTextInput.forceActiveFocus()
    }
}
