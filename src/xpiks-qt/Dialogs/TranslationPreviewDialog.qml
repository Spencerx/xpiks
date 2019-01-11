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

StaticDialogBase {
    id: translationPreviewComponent
    anchors.fill: parent
    canMove: false

    z: 10000

    contentsWidth: 600
    contentsHeight: 700

    contents: Item {
        anchors.fill: parent

        StyledText {
            id: header
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: 20
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            text: i18.n + qsTr("Translation for \"%1\"").arg(translationManager.query)
        }

        Rectangle {
            id: wrapperRect
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
                    text: translationManager.fullTranslation
                    selectionColor: uiColors.inputBackgroundColor
                    readOnly: true
                    width: wrapperRect.width - 40
                    wrapMode: TextEdit.WordWrap
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

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Close")
                width: 100
                onClicked: {
                    closePopup()
                }
            }
        }
    }
}
