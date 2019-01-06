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

StaticDialogBase {
    id: installUpdateComponent
    anchors.fill: parent

    property bool isInProgress: false

    contentsWidth: 300
    contentsHeight: 100

    contents: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        StyledText {
            text: i18.n + qsTr("Updates are ready to install")
            anchors.left: parent.left
        }

        RowLayout {
            spacing: 20
            Layout.fillWidth: true

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
