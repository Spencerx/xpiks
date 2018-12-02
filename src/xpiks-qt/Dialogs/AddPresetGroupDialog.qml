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

StaticDialogBase {
    id: addGroupComponent
    canMinimize: false
    canEscapeClose: false
    property var callbackObject
    property bool anyError: false
    anchors.fill: parent

    function submitGroupName(groupName) {
        if (anyError) { return; }

        callbackObject.onSuccess(groupName)
        closePopup()
    }

    Keys.onEscapePressed: closePopup()

    function closePopup() {
        callbackObject.onClose()
        addGroupComponent.destroy()
    }

    contentsWidth: 240
    contentsHeight: column.childrenRect.height + 40

    contents: Column {
        id: column
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: childrenRect.height
        anchors.margins: 20
        spacing: 20

        Rectangle {
            color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
            border.width: groupNameInput.activeFocus ? 1 : 0
            border.color: addGroupComponent.anyError ? uiColors.artworkModifiedColor : uiColors.artworkActiveColor
            width: 200
            height: 30
            clip: true

            StyledTextInput {
                id: groupNameInput
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                onAccepted: submitGroupName(groupNameInput.text)

                Keys.onBacktabPressed: {
                    event.accepted = true
                }

                Keys.onTabPressed: {
                    event.accepted = true
                }

                Keys.onPressed: {
                    if (event.key === Qt.Key_Comma) {
                        event.accepted = true
                    }
                }

                onTextChanged: {
                    addGroupComponent.anyError = !presetsGroups.canAddGroup(text)
                }
            }
        }

        RowLayout {
            width: parent.width
            height: 20
            spacing: 20

            StyledButton {
                text: i18.n + qsTr("Add")
                width: 90
                enabled: !addGroupComponent.anyError && (groupNameInput.text.trim().length > 2)
                onClicked: submitGroupName(groupNameInput.text)
            }

            StyledButton {
                text: i18.n + qsTr("Cancel")
                width: 90
                onClicked: closePopup()
            }
        }
    }

    Component.onCompleted: {
        focus = true
        groupNameInput.forceActiveFocus()
    }
}
