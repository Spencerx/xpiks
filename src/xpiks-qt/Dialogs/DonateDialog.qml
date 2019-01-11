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
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: donateComponent
    canMinimize: false
    canMove: false
    anchors.fill: parent

    z: 10000

    contentsWidth: 400
    contentsHeight: 400
    contents: ColumnLayout {
        anchors.centerIn: parent
        spacing: 0
        signal donateLinkClicked()

        Image {
            source: uiColors.t + helpersWrapper.getAssetForTheme("Icon_donate.svg", settingsModel.selectedThemeIndex)
            cache: false
            width: 123
            height: 115
            //fillMode: Image.PreserveAspectFit
            asynchronous: true
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Item {
            height: 30
        }

        StyledText {
            text: i18.n + qsTr("We hope you enjoy using Xpiks.")
            anchors.horizontalCenter: parent.horizontalCenter
            isActive: true
            font.bold: true
        }

        Item {
            height: 20
        }

        StyledText {
            text: i18.n + qsTr("Today Xpiks needs Your help to become even better.")
            anchors.horizontalCenter: parent.horizontalCenter
            isActive: true
            font.italic: true
            font.pixelSize: UIConfig.fontPixelSize - 2
        }

        Item {
            height: 50
        }

        StyledBlackButton {
            implicitHeight: 30
            height: 30
            width: 150
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18.n + qsTr("Support Xpiks")
            defaultForeground: uiColors.goldColor
            hoverForeground: uiColors.buttonDefaultForeground
            visible: true
            onClicked: {
                Qt.openUrlExternally(switcher.donateCampaign1Link)
                switcher.setDonateCampaign1LinkClicked()
            }
        }

        Item {
            height: 20
        }

        StyledText {
            text: i18.n + qsTr("Maybe later")
            anchors.horizontalCenter: parent.horizontalCenter
            isActive: dismissMA.pressed

            MouseArea {
                id: dismissMA
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: closePopup()
            }
        }
    }
}
