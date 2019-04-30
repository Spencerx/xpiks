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
import QtGraphicalEffects 1.0
import xpiks 1.0
import "../Common.js" as Common;
import "../Components"

BaseDialog {
    id: root
    property bool canMove: true
    property real contentsWidth
    property real contentsHeight
    property alias contents: placeholder.children

    signal clickedOutside()

    signal dialogDestruction()
    Component.onDestruction: dialogDestruction()

    FocusScope {
        anchors.fill: parent

        MouseArea {
            id: backgroundMA
            anchors.fill: parent
            onWheel: wheel.accepted = true
            onClicked: mouse.accepted = true
            onDoubleClicked: mouse.accepted = true
            hoverEnabled: true

            property real old_x : 0
            property real old_y : 0

            onPressed: {
                if (root.canMove) {
                    var tmp = mapToItem(root, mouse.x, mouse.y);
                    old_x = tmp.x;
                    old_y = tmp.y;

                    var dialogPoint = mapToItem(dialogWindow, mouse.x, mouse.y);
                    if (!Common.isInComponent(dialogPoint, dialogWindow)) {
                        clickedOutside()
                    }
                }
            }

            onPositionChanged: {
                if (root.canMove) {
                    if (!backgroundMA.pressed) { return }

                    var old_xy = Common.movePopupInsideComponent(root, dialogWindow, mouse, old_x, old_y);
                    old_x = old_xy[0]; old_y = old_xy[1];
                }
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
            visible: !settingsModel.useSimplifiedUI
            enabled: !settingsModel.useSimplifiedUI
        }

        // This rectangle is the actual popup
        Rectangle {
            id: dialogWindow
            width: root.contentsWidth
            height: root.contentsHeight
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            Item {
                id: placeholder
                anchors.fill: parent
            }
        }
    }
}
