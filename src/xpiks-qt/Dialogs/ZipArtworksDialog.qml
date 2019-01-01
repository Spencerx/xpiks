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

StaticDialogBase {
    id: zipArtworksComponent
    canMinimize: false
    canEscapeClose: false
    property bool immediateProcessing: false
    property var callbackObject
    anchors.fill: parent
    property var zipArchiver: dispatcher.getCommandTarget(UICommand.SetupCreatingArchives)

    Connections {
        target: zipArchiver
        onRequestCloseWindow: {
            closePopup()
        }

        onFinishedProcessing: {
            importButton.text = i18.n + qsTr("Start Zipping")

            if (zipArtworksComponent.immediateProcessing) {
                closePopup()
            }

            //if (!zipArchiver.isError) {
            //    closePopup()
            //}
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.CreateArchives]
        onDispatched: {
            if (!value) {
                console.log("Exiting...")
                closePopup()
            }
        }
    }

    onClickedOutside: {
        if (!zipArchiver.inProgress) {
            dispatcher.dispatch(UICommand.CreateArchives, false)
        }
    }

    contentsWidth: 480
    contentsHeight: column.childrenRect.height + 40

    contents: Column {
        id: column
        spacing: 20
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: childrenRect.height
        anchors.margins: 20

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right

            StyledText {
                text: i18.n + qsTr("Zip vectors with previews")
            }

            Item {
                Layout.fillWidth: true
            }

            StyledText {
                id: textItemsAvailable
                text: i18.n + getOriginalText()

                function getOriginalText() {
                    return zipArchiver.itemsCount === 1 ? qsTr("1 artwork with vector") : qsTr("%1 artworks with vectors").arg(zipArchiver.itemsCount)
                }

                Connections {
                    target: zipArchiver
                    onItemsCountChanged: {
                        textItemsAvailable.text = i18.n + textItemsAvailable.getOriginalText()
                    }
                }
            }
        }

        SimpleProgressBar {
            id: progress
            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width
            height: 20
            color: zipArchiver.isError ? uiColors.destructiveColor : uiColors.artworkActiveColor
            value: zipArchiver.percent
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
                id: importButton
                objectName: "zipButton"
                isDefault: true
                width: 130
                text: i18.n + qsTr("Start Zipping")
                enabled: !zipArchiver.inProgress && (zipArchiver.itemsCount > 0)
                onClicked: {
                    text = i18.n + qsTr("Zipping...")
                    dispatcher.dispatch(UICommand.CreateArchives, true)
                }
            }

            StyledButton {
                text: i18.n + qsTr("Close")
                width: 100
                enabled: !zipArchiver.inProgress
                onClicked: {
                    dispatcher.dispatch(UICommand.CreateArchives, false)
                }
            }
        }
    }

    Component.onCompleted: {
        focus = true
        if (immediateProcessing) {
            console.debug("immediate processing for zipper")
            dispatcher.dispatch(UICommand.CreateArchives, true)
        }
    }
}
