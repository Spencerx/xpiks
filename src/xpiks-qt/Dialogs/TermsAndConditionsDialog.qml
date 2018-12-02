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
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: termsComponent
    canMinimize: false
    canEscapeClose: false
    canMove: false
    property string termsText
    anchors.fill: parent
    z: 20000

    contentsWidth: 750
    contentsHeight: 650

    contents: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10

        StyledText {
            text: i18.n + qsTr("Terms and conditions:")
        }

        Rectangle {
            color: uiColors.popupDarkInputBackground
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillHeight: true

            StyledScrollView {
                anchors.fill: parent
                anchors.margins: 10

                StyledTextEdit {
                    id: textEdit
                    width: 650
                    text: qsTr("The program is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

By accessing and using Xpiks (heretofore called the 'application' or the 'program'), you accept and agree to be bound by the terms and provision of this agreement. In addition, when using this particular application, you shall be subject to any posted guidelines or rules applicable to such application. If you do not agree to abide by the above, please do not use this application.

Xpiks has an option to save credentials of FTP uploads. When you enter such sensitive data, Xpiks always encrypts it. Also you have option to encrypt it with your own password (Master Password). These encrypted credentials are only stored locally.

Xpiks is distributed under Mozilla Public License, v. 2.0. Xpiks source code is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.

This software uses code of FFmpeg (http://ffmpeg.org) licensed under the LGPLv2.1 (http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html) and its source can be downloaded at
https://git.ffmpeg.org/ffmpeg.git.

Application and its original content, features, and functionality are owned by Taras Kushnir and are protected by international copyright.

This terms and conditions can be amended in future and users will be notified on such occasions.")
                    wrapMode: TextEdit.Wrap
                    selectionColor: uiColors.inputBackgroundColor
                    readOnly: true
                    textFormat: TextEdit.PlainText
                }
            }
        }

        Item {
            height: 1
        }

        StyledText {
            text: i18.n + qsTr("Health report:")
        }

        Rectangle {
            color: uiColors.popupDarkInputBackground
            anchors.left: parent.left
            anchors.right: parent.right
            height: 100

            StyledScrollView {
                anchors.fill: parent
                anchors.margins: 10

                StyledTextEdit {
                    width: 650
                    text: qsTr("In order to monitor quality of the application functioning, it can automatically send anonymous usage statistics data. This data is used to monitor issues with Xpiks and to prioritize future work. Such analytics data is never shared with any third-parties and is saved in a secure manner.

Please select 'Health report' checkbox if you agree to send anonymous health report.")
                    wrapMode: TextEdit.Wrap
                    selectionColor: uiColors.inputBackgroundColor
                    readOnly: true
                    textFormat: TextEdit.PlainText
                }
            }
        }

        Item {
            id: checkboxesRect
            height: 40
            anchors.left: parent.left
            anchors.right: parent.right

            StyledCheckbox {
                id: userStatisticCheckBox
                anchors.centerIn: parent
                checked: false
                text: i18.n + qsTr("Health report")
            }
        }

        Item {
            height: 1
        }

        RowLayout {
            height: 24
            spacing: 50

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                id: agreeButton
                text: i18.n + qsTr("Agree")
                isDefault: true
                width: 100
                onClicked: {
                    settingsModel.userStatistics = userStatisticCheckBox.checked
                    settingsModel.userAgreeHandler()
                    helpersWrapper.reportOpen()
                    closePopup()
                }
            }

            StyledButton {
                text: i18.n + qsTr("Disagree")
                width: 100
                tooltip: i18.n + qsTr("Close Xpiks")
                onClicked: {
                    closePopup()
                    if (!debug) {
                        Qt.quit()
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
