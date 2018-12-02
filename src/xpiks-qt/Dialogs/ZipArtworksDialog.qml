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

BaseDialog {
    id: zipArtworksComponent
    canMinimize: false
    canEscapeClose: false
    property bool immediateProcessing: false
    property var callbackObject
    anchors.fill: parent
    property var zipArchiver: dispatcher.getCommandTarget(UICommand.SetupCreatingArchives)

    function closePopup() {
        zipArtworksComponent.destroy()
        zipArchiver.resetModel();
    }

    Connections {
        target: zipArchiver
        onRequestCloseWindow: {
            closePopup();
        }

        onFinishedProcessing: {
            importButton.text = i18.n + qsTr("Start Zipping")

            if (immediateProcessing) {
                if (typeof callbackObject !== "undefined") {
                    callbackObject.afterZipped()
                }

                closePopup()
            }

            //if (!zipArchiver.isError) {
            //    closePopup()
            //}
        }
    }

    FocusScope {
        anchors.fill: parent

        MouseArea {
            anchors.fill: parent
            onWheel: wheel.accepted = true
            onClicked: mouse.accepted = true
            onDoubleClicked: mouse.accepted = true

            property real old_x : 0
            property real old_y : 0

            onPressed:{
                var tmp = mapToItem(zipArtworksComponent, mouse.x, mouse.y);
                old_x = tmp.x;
                old_y = tmp.y;

                var dialogPoint = mapToItem(dialogWindow, mouse.x, mouse.y);
                if (!Common.isInComponent(dialogPoint, dialogWindow)) {
                    if (!zipArchiver.inProgress) {
                        closePopup()
                    }
                }
            }

            onPositionChanged: {
                var old_xy = Common.movePopupInsideComponent(zipArtworksComponent, dialogWindow, mouse, old_x, old_y);
                old_x = old_xy[0]; old_y = old_xy[1];
            }
        }

        RectangularGlow {
            anchors.fill: dialogWindow
            anchors.topMargin: glowRadius/2
            anchors.bottomMargin: -glowRadius/2
            glowRadius: 4
            spread: 0.0
            color: uiColors.popupGlowColor
            cornerRadius: glowRadius
        }

        // This rectangle is the actual popup
        Rectangle {
            id: dialogWindow
            width: 480
            height: childrenRect.height + 40
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            Column {
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
                            return zipArchiver.itemsCount == 1 ? qsTr("1 artwork with vector") : qsTr("%1 artworks with vectors").arg(zipArchiver.itemsCount)
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
                        objectName: "importButton"
                        isDefault: true
                        width: 130
                        text: i18.n + qsTr("Start Zipping")
                        enabled: !zipArchiver.inProgress && (zipArchiver.itemsCount > 0)
                        onClicked: {
                            text = i18.n + qsTr("Zipping...")
                            zipArchiver.archiveArtworks()
                        }
                    }

                    StyledButton {
                        text: i18.n + qsTr("Close")
                        width: 100
                        enabled: !zipArchiver.inProgress
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
        if (immediateProcessing) {
            console.debug("immediate processing for zipper")
            zipArchiver.archiveArtworks()
        }
    }
}
