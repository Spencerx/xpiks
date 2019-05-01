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
    id: logsComponent
    canEscapeClose: false
    property string logText
    anchors.fill: parent
    property var logsModel: dispatcher.getCommandTarget(UICommand.UpdateLogs)

    function closePopup() {
        logsModel.clearLogsExtract()
        logsComponent.destroy()
    }

    Keys.onEscapePressed: closePopup()

    function scrollToBottom() {
        var flickable = scrollView.flickableItem
        if (flickable.contentHeight > flickable.height) {
            flickable.contentY = flickable.contentHeight - flickable.height
        } else {
            flickable.contentY = 0
        }
    }

    onClickedOutside: closePopup()

    contentsWidth: logsComponent.width * 0.75
    contentsHeight: logsComponent.height - 60

    contents: Item {
        anchors.fill: parent

        RowLayout {
            id: header
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 20
            anchors.leftMargin: 20
            anchors.rightMargin: 20

            StyledText {
                text: i18.n + qsTr("Logs")
            }

            Item {
                Layout.fillWidth: true
            }

            StyledText {
                property int linesNumber : 100
                id: oneHunderdLinesWarning
                text: i18.n + qsTr("(showing last %1 lines)").arg(linesNumber)
            }
        }

        Rectangle {
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            anchors.topMargin: 10
            anchors.bottom: footer.top
            anchors.bottomMargin: 20
            color: uiColors.popupDarkInputBackground

            StyledScrollView {
                id: scrollView
                anchors.fill: parent
                anchors.margins: 10

                StyledTextEdit {
                    id: textEdit
                    text: logsModel.logsExtract
                    selectionColor: uiColors.inputBackgroundColor
                    readOnly: true

                    Component.onCompleted: {
                        scrollToBottom()
                        uiManager.initLogsHighlighting(textEdit.textDocument)
                    }
                }
            }
        }

        RowLayout {
            id: footer
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.left: parent.left
            anchors.leftMargin: 20
            anchors.right: parent.right
            anchors.rightMargin: 20
            height: 24
            spacing: 20

            StyledButton {
                id: loadMoreButton
                text: i18.n + qsTr("Load more logs")
                enabled: logsModel.withLogs
                width: 130
                onClicked: {
                    dispatcher.dispatch(UICommand.UpdateLogs, true)
                    oneHunderdLinesWarning.linesNumber = 1000
                    loadMoreButton.enabled = false
                    scrollToBottom()
                }
            }

            StyledButton {
                id: revealFileButton
                text: i18.n + qsTr("Reveal logfile")
                width: 130
                onClicked: {
                    helpersWrapper.revealLogFile()
                }
            }

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Close")
                width: 110
                onClicked: {
                    closePopup()
                }
            }
        }
    }
}
