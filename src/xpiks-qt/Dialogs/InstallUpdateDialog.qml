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
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"

BaseDialog {
    id: installUpdateComponent
    anchors.fill: parent

    property bool isInProgress: false

    FocusScope {
        anchors.fill: parent
        id: focusScope

        MouseArea {
            anchors.fill: parent
            onWheel: wheel.accepted = true
            onClicked: mouse.accepted = true;
            onDoubleClicked: mouse.accepted = true

            property real old_x : 0
            property real old_y : 0

            onPressed:{
                var tmp = mapToItem(installUpdateComponent, mouse.x, mouse.y);
                old_x = tmp.x;
                old_y = tmp.y;
            }

            onPositionChanged: {
                var old_xy = Common.movePopupInsideComponent(installUpdateComponent, dialogWindow, mouse, old_x, old_y);
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
            width: 300
            height: 100
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 20
                spacing: 20

                StyledText {
                    text: i18.n + qsTr("Updates are ready to install")
                    anchors.left: parent.left
                }

                RowLayout {
                    spacing: 20
                    anchors.left: parent.left
                    anchors.right: parent.right

                    StyledButton {
                        width: 100
                        isDefault: true
                        text: i18.n + qsTr("Upgrade")
                        onClicked: {
                            xpiksApp.upgradeNow()
                            closePopup()
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledButton {
                        width: 100
                        text: i18.n + qsTr("Later")
                        onClicked: {
                            closePopup()
                        }
                    }
                }
            }
        }
    }
}
