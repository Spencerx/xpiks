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
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: masterPasswordComponent
    canMinimize: false
    canEscapeClose: false
    property bool firstTime
    property bool emptyMP: false
    property bool wrongMP: false
    anchors.fill: parent
    property var callbackObject

    function closePopup() {
        secretsManager.purgeMasterPassword()
        masterPasswordComponent.destroy()
    }

    function trySetupMP() {
        if (!firstTime && currentPassword.length == 0) {
            currentPassword.forceActiveFocus()
            emptyMP = true
            return
        }

        if (repeatMasterPassword.text == newMasterPassword.text) {

            if (repeatMasterPassword.length == 0) {
                callbackObject.onCancel(firstTime)
                closePopup()
            }

            if (secretsManager.changeMasterPassword(firstTime,
                                                    currentPassword.text,
                                                    newMasterPassword.text)) {
                callbackObject.onSuccess()
                closePopup()
            } else {
                wrongCurrentMPDialog.open()
            }
        } else {
            notEqualPasswordsDialog.open()
        }
    }

    MessageDialog {
        id: notEqualPasswordsDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Passwords are not equal")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: wrongCurrentMPDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please, enter correct current master password")
        standardButtons: StandardButton.Ok
        onAccepted: {
            currentPassword.forceActiveFocus()
            wrongMP = true
        }
    }

    contentsWidth: 360
    contentsHeight: 220

    contents: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        RowLayout {
            id: currentPasswordRow
            width: parent.width
            height: 20
            enabled: !firstTime
            spacing: 10

            StyledText {
                Layout.fillWidth: true
                Layout.preferredWidth: 130
                horizontalAlignment: Text.AlignRight
                text: i18.n + qsTr("Current Master Password:")
            }

            Rectangle {
                color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                border.width: (currentPassword.activeFocus || emptyMP) ? 1 : 0
                border.color: (emptyMP || wrongMP) ? uiColors.artworkModifiedColor : uiColors.artworkActiveColor
                width: 135
                height: UIConfig.textInputHeight
                clip: true

                StyledTextInput {
                    id: currentPassword
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    echoMode: TextInput.Password
                    anchors.verticalCenter: parent.verticalCenter

                    Keys.onBacktabPressed: {
                        event.accepted = true
                    }

                    KeyNavigation.tab: newMasterPassword
                    onAccepted: {
                        if (repeatMasterPassword.text != newMasterPassword.text) {
                            newMasterPassword.forceActiveFocus()
                        } else {
                            trySetupMP()
                        }
                    }
                }
            }
        }

        RowLayout {
            width: parent.width
            height: 20
            spacing: 10

            StyledText {
                Layout.fillWidth: true
                Layout.preferredWidth: 130
                horizontalAlignment: Text.AlignRight
                text: i18.n + qsTr("New Master Password:")
            }

            Rectangle {
                color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                border.width: newMasterPassword.activeFocus ? 1 : 0
                border.color: uiColors.artworkActiveColor
                width: 135
                height: UIConfig.textInputHeight
                clip: true

                StyledTextInput {
                    id: newMasterPassword
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    echoMode: TextInput.Password
                    KeyNavigation.backtab: currentPassword
                    KeyNavigation.tab: repeatMasterPassword
                    anchors.verticalCenter: parent.verticalCenter

                    onAccepted: {
                        if (repeatMasterPassword.text != newMasterPassword.text) {
                            repeatMasterPassword.forceActiveFocus()
                        } else {
                            trySetupMP()
                        }
                    }
                }
            }
        }

        RowLayout {
            width: parent.width
            height: 20
            spacing: 10

            StyledText {
                Layout.fillWidth: true
                Layout.preferredWidth: 130
                horizontalAlignment: Text.AlignRight
                text: i18.n + qsTr("Repeat Master Password:")
            }

            Rectangle {
                border.width: enabled ? ((repeatMasterPassword.activeFocus || newMasterPassword.length > 0) ? 1 : 0) : 0
                border.color: repeatMasterPassword.text == newMasterPassword.text ? uiColors.artworkActiveColor : uiColors.artworkModifiedColor
                color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                width: 135
                height: UIConfig.textInputHeight
                clip: true

                StyledTextInput {
                    id: repeatMasterPassword
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    echoMode: TextInput.Password
                    KeyNavigation.backtab: newMasterPassword
                    onAccepted: trySetupMP()
                }
            }
        }

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 24
            spacing: 20

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Ok")
                width: 80
                onClicked: trySetupMP()
            }

            StyledButton {
                text: i18.n + qsTr("Cancel")
                width: 80
                onClicked: {
                    callbackObject.onCancel(firstTime)
                    closePopup()
                }
            }
        }
    }

    Component.onCompleted: {
        focus = true
        if (firstTime) {
            newMasterPassword.forceActiveFocus()
        } else {
            currentPassword.forceActiveFocus()
        }
    }
}

