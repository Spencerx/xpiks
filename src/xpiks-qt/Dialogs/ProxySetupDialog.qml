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
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import "../Constants"
import "../Common.js" as Common
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: proxySetupComponent
    property bool firstTime
    property var callbackObject
    anchors.fill: parent

    function saveProxySettings() {
        settingsModel.saveProxySetting(proxyAddress.text,
                                       proxyUser.text,
                                       proxyPassword.text,
                                       proxyPort.text)
    }

    MessageDialog {
        id: invalidAddress
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please, enter proxy address")
        standardButtons: StandardButton.Ok
    }

    contentsWidth: 280
    contentsHeight: 280

    contents: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        StyledText {
            anchors.left: parent.left
            text: i18.n + qsTr("Proxy settings:")
        }

        RowLayout {
            id: proxyAddressRow
            Layout.fillWidth: true
            spacing: 10

            StyledText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18.n + qsTr("Hostname:")
            }

            Rectangle {
                color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                border.width: (proxyAddress.activeFocus) ? 1 : 0
                border.color: uiColors.artworkActiveColor
                width: 135
                height: UIConfig.textInputHeight
                clip: true

                StyledTextInput {
                    id: proxyAddress
                    text: settingsModel.proxyAddress
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    KeyNavigation.tab: proxyUser
                }
            }
        }

        RowLayout {
            id: proxyUserRow
            Layout.fillWidth: true
            height: 20
            spacing: 10

            StyledText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18.n + qsTr("Username:")
            }

            Rectangle {
                color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                border.width: (proxyUser.activeFocus) ? 1 : 0
                border.color: uiColors.artworkActiveColor
                width: 135
                height: UIConfig.textInputHeight
                clip: true

                StyledTextInput {
                    id: proxyUser
                    text: settingsModel.proxyUser
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    KeyNavigation.backtab: proxyAddress
                    KeyNavigation.tab: proxyPassword
                }
            }
        }

        RowLayout {
            id: proxyPasswordRow
            Layout.fillWidth: true
            height: 20
            spacing: 10

            StyledText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18.n + qsTr("Password:")
            }

            Rectangle {
                color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                border.width: (proxyPassword.activeFocus) ? 1 : 0
                border.color: uiColors.artworkActiveColor
                width: 135
                height: UIConfig.textInputHeight
                clip: true

                StyledTextInput {
                    id: proxyPassword
                    text: settingsModel.proxyPassword
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    echoMode: TextInput.Password
                    KeyNavigation.backtab: proxyUser
                    KeyNavigation.tab: proxyPort
                }
            }
        }

        RowLayout {
            id: proxyPortRow
            Layout.fillWidth: true
            height: 20
            spacing: 10

            StyledText {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignRight
                text: i18.n + qsTr("Port:")
            }

            Rectangle {
                color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                border.width: (proxyPort.activeFocus) ? 1 : 0
                border.color: uiColors.artworkActiveColor
                width: 135
                height: UIConfig.textInputHeight
                clip: true

                StyledTextInput {
                    id: proxyPort
                    text: settingsModel.proxyPort
                    validator: IntValidator {
                        bottom: 0
                        top: 65536
                    }
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 5
                    anchors.rightMargin: 5
                    anchors.verticalCenter: parent.verticalCenter
                    KeyNavigation.backtab: proxyPassword
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            height: 24
            spacing: 20

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Ok")
                width: 80
                onClicked: {
                    if (proxyAddress.text.length == 0){
                        invalidAddress.open()
                    } else {
                        saveProxySettings()
                        callbackObject.onSuccess()
                        closePopup()
                    }
                }
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
        proxyAddress.forceActiveFocus()
        proxyAddress.cursorPosition = proxyAddress.text.length
    }
}
