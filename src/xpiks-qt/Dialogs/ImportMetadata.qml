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

BaseDialog {
    id: metadataImportComponent
    anchors.fill: parent
    canEscapeClose: false
    property int importID: 0
    property bool backupsEnabled: true
    property bool reimport: false

    signal dialogDestruction()
    Component.onDestruction: dialogDestruction()

    Keys.onEscapePressed: {
        if (!metadataIOCoordinator.isInProgress) {
            closePopup()
        }

        event.accepted = true
    }

    function closePopup() {
        metadataImportComponent.destroy()
    }

    Connections {
        target: metadataIOCoordinator

        onImportFinished: {
            console.log("UI::ImportMetadata # Import finished handler")

            if (importID == metadataImportComponent.importID) {
                warningsModel.update()

                if (metadataIOCoordinator.hasErrors) {
                    errorsNotification.open()
                } else {
                    closePopup()
                }
            } else {
                console.warn("Import ID " + importID + " doesn't match to " + metadataImportComponent.importID)
            }
        }
    }

    Component.onCompleted: {
        focus = true

        if (metadataIOCoordinator.hasImportFinished(metadataImportComponent.importID)) {
            console.debug("UI::ImportMetadata # Import seems to be finished")
            closePopup()
        }
    }

    MessageDialog {
        id: errorsNotification
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Action finished with errors. See logs for details.")

        onAccepted: {
            closePopup()
        }
    }

    MessageDialog {
        id: installExiftoolDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please install Exiftool to import metadata")
        informativeText: i18.n + qsTr('<a href="http://www.sno.phy.queensu.ca/~phil/exiftool/">Official Exiftool website</a>')
        standardButtons: StandardButton.Abort | StandardButton.Ignore
        onRejected: {
            closePopup()
        }
        onAccepted: {
            continueImport()
        }
    }

    function continueImport() {
        importButton.text = i18.n + qsTr("Importing...")
        metadataIOCoordinator.continueReading(ignoreAutosavesCheckbox.checked)
    }

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
                var tmp = mapToItem(metadataImportComponent, mouse.x, mouse.y);
                old_x = tmp.x;
                old_y = tmp.y;
            }

            onPositionChanged: {
                var old_xy = Common.movePopupInsideComponent(metadataImportComponent, dialogWindow, mouse, old_x, old_y);
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
            enabled: !settingsModel.useSimplifiedUI
            visible: !settingsModel.useSimplifiedUI
        }

        // This rectangle is the actual popup
        Rectangle {
            id: dialogWindow
            width: 380
            height: childrenRect.height + 40
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            Behavior on height {
                NumberAnimation {
                    duration: 200
                    easing.type: Easing.InQuad
                }
            }

            Column {
                id: column
                spacing: 20
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                height: childrenRect.height
                anchors.margins: 20

                add: Transition {
                    NumberAnimation { properties: "x,y"; easing.type: Easing.InQuad; duration: 200 }
                }

                move: Transition {
                    NumberAnimation { properties: "x,y"; easing.type: Easing.InQuad; duration: 200 }
                }

                Item {
                    height: childrenRect.height
                    anchors.left: parent.left
                    anchors.right: parent.right

                    StyledText {
                        anchors.left: parent.left
                        text: i18.n + qsTr("Import existing metadata")
                    }

                    StyledText {
                        anchors.right: parent.right
                        text: i18.n + qsTr("from %1 file(s)").arg(metadataIOCoordinator.processingItemsCount)
                    }
                }

                LoaderIcon {
                    id: spinner
                    width: 150
                    height: 0
                    anchors.horizontalCenter: parent.horizontalCenter
                    running: metadataIOCoordinator.isInProgress

                    states: [
                        State {
                            name: "importing"
                            when: metadataIOCoordinator.isInProgress
                            PropertyChanges {
                                target: spinner
                                height: spinner.width
                            }
                        }
                    ]
                }

                StyledCheckbox {
                    id: ignoreAutosavesCheckbox
                    text: i18.n + qsTr("Ignore autosaves")
                    enabled: settingsModel.saveBackups && !metadataIOCoordinator.isInProgress && metadataImportComponent.backupsEnabled
                    checked: !metadataImportComponent.backupsEnabled
                }

                RowLayout {
                    height: 24
                    anchors.left: parent.left
                    anchors.right: parent.right

                    StyledButton {
                        id: importButton
                        isDefault: true
                        width: 130
                        text: i18.n + qsTr("Start Import")
                        enabled: !metadataIOCoordinator.isInProgress
                        onClicked: {
                            console.debug("Start Import pressed")
                            if (metadataIOCoordinator.exiftoolNotFound) {
                                installExiftoolDialog.open()
                            } else {
                                continueImport()
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledButton {
                        text: i18.n + qsTr("Close")
                        width: 100
                        enabled: !metadataIOCoordinator.isInProgress
                        onClicked: {
                            console.debug("Close without Import pressed")
                            metadataIOCoordinator.continueWithoutReading(ignoreAutosavesCheckbox.checked, metadataImportComponent.reimport)
                            closePopup()
                        }
                    }
                }
            }

        }
    }
}
