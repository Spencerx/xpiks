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
import "../StyledControls"

ColumnLayout {
    spacing: 0
    signal donateLinkClicked()

    Image {
        source: uiColors.t + helpersWrapper.getAssetForTheme("Icon_donate.svg", settingsModel.selectedThemeIndex)
        cache: false
        width: 123
        height: 115
        //fillMode: Image.PreserveAspectFit
        asynchronous: true
        Layout.alignment: Qt.AlignHCenter
    }

    Item {
        height: 30
    }

    StyledText {
        text: i18.n + qsTr("Help us to make Xpiks even better.")
        Layout.alignment: Qt.AlignHCenter
        isActive: true
    }

    Item {
        height: 10
    }

    StyledLink {
        text: i18.n + qsTr("Support Xpiks")
        Layout.alignment: Qt.AlignHCenter
        color: isPressed ? uiColors.linkClickedColor : uiColors.goldColor
        font.bold: true
        onClicked: {
            Qt.openUrlExternally(switcher.donateCampaign1Link)
            switcher.setDonateCampaign1LinkClicked()
            donateLinkClicked()
        }
    }
}
