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
    id: editKeywordComponent
    canMinimize: false
    canEscapeClose: false
    property var callbackObject
    property string previousKeyword: ''
    property var keywordsModel
    property int keywordIndex
    property bool anyError: false
    anchors.fill: parent

    function submitKeyword() {
        if (anyError) { return; }

        var keyword = keywordInput.text
        if (helpersWrapper.isKeywordValid(keyword)) {
            callbackObject.onSuccess(keyword)
            closePopup()
        }
    }

    Connections {
        target: keywordsModel
        onAboutToBeRemoved: {
            console.info("About to be removed")
            closePopup()
        }
    }

    Keys.onEscapePressed: closePopup()

    function closePopup() {
        callbackObject.onClose()
        editKeywordComponent.destroy()
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
            border.width: keywordInput.activeFocus ? 1 : 0
            border.color: editKeywordComponent.anyError ? uiColors.artworkModifiedColor : uiColors.artworkActiveColor
            width: 200
            height: 30
            clip: true

            StyledTextInput {
                id: keywordInput
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 5
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                onAccepted: submitKeyword()
                Component.onCompleted: keywordInput.text = previousKeyword

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
                    editKeywordComponent.anyError = !keywordsModel.canEditKeyword(keywordIndex, text) &&
                            (previousKeyword !== text.trim());
                }
            }
        }

        RowLayout {
            width: parent.width
            height: 20
            spacing: 20

            StyledButton {
                text: i18.n + qsTr("Save")
                width: 90
                enabled: !editKeywordComponent.anyError
                onClicked: submitKeyword()
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
        keywordInput.forceActiveFocus()
    }
}
