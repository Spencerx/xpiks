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

Item {
    id: pluginsComponent
    property string logText
    anchors.fill: parent

    signal dialogDestruction();
    Component.onDestruction: dialogDestruction();

    function closePopup() {
        pluginsComponent.destroy()
    }

    Component.onCompleted: focus = true
    Keys.onEscapePressed: closePopup()

    PropertyAnimation { target: pluginsComponent; property: "opacity";
        duration: 400; from: 0; to: 1;
        easing.type: Easing.InOutQuad ; running: true }

    // This rectange is the a overlay to partially show the parent through it
    // and clicking outside of the 'dialog' popup will do 'nothing'
    Rectangle {
        anchors.fill: parent
        id: overlay
        color: "#000000"
        opacity: 0.6
        // add a mouse area so that clicks outside
        // the dialog window will not do anything
        MouseArea {
            anchors.fill: parent
        }
    }

    MessageDialog {
        id: pluginExistsDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Selected plugin is already installed.")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: confirmRemovePluginDialog
        property int pluginIndex
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Are you sure you want to remove this plugin?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            if (pluginManager.removePlugin(pluginIndex)) {
                removedSuccessfullyDialog.open()
            } else {
                failedToRemoveDialog.open()
            }
        }
    }

    MessageDialog {
        id: installedSuccessfullyDialog
        title: i18.n + qsTr("Info")
        text: i18.n + qsTr("Plugin has been successfully installed.")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: removedSuccessfullyDialog
        title: i18.n + qsTr("Info")
        text: i18.n + qsTr("Plugin has been successfully uninstalled.")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: failedToInstallDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Failed to install selected plugin.")
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: failedToRemoveDialog
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Failed to uninstall selected plugin.")
        standardButtons: StandardButton.Ok
    }

    FileDialog {
        id: choosePluginDialog
        title: "Select plugin"
        selectExisting: true
        selectMultiple: false
        nameFilters: [ "All files (*)" ]

        onAccepted: {
            var pluginUrl = choosePluginDialog.fileUrl
            console.debug("You chose: " + pluginUrl)
            if (!pluginManager.installPlugin(pluginUrl)) {
                if (pluginManager.pluginExists(pluginUrl)) {
                    pluginExistsDialog.open()
                } else {
                    failedToInstallDialog.open()
                }
            } else {
                installedSuccessfullyDialog.open()
            }
        }

        onRejected: {
            console.debug("Open files dialog canceled")
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
                var tmp = mapToItem(pluginsComponent, mouse.x, mouse.y);
                old_x = tmp.x;
                old_y = tmp.y;

                var dialogPoint = mapToItem(dialogWindow, mouse.x, mouse.y);
                if (!Common.isInComponent(dialogPoint, dialogWindow)) {
                    closePopup()
                }
            }

            onPositionChanged: {
                var old_xy = Common.movePopupInsideComponent(pluginsComponent, dialogWindow, mouse, old_x, old_y);
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
            width: 500
            height: 400
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            StyledText {
                id: header
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.topMargin: 20
                anchors.leftMargin: 20
                text: i18.n + qsTr("Plugins")
            }

            Rectangle {
                anchors.top: header.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.topMargin: 10
                anchors.bottom: footer.top
                anchors.bottomMargin: 20
                color: uiColors.defaultControlColor

                StyledScrollView {
                    id: scrollView
                    anchors.fill: parent
                    anchors.margins: 10

                    ListView {
                        id: pluginsListView
                        model: pluginManager
                        spacing: 5

                        delegate: Rectangle {
                            id: wrapper
                            color: uiColors.panelColor                            
                            property int delegateIndex: index
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 30

                            StyledText {
                                id: pluginNameText
                                text: prettyname
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: parent.left
                                anchors.leftMargin: 10
                                isActive: !removed
                                font.strikeout: removed
                            }

                            StyledText {
                                text: i18.n + qsTr("(restart required)")
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.left: pluginNameText.right
                                anchors.leftMargin: 10
                                isActive: false
                                visible: removed
                                enabled: removed
                            }

                            StyledText {
                                text: version
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: deleteIcon.left
                                anchors.rightMargin: 10
                                isActive: !removed
                                font.strikeout: removed
                            }

                            CloseIcon {
                                id: deleteIcon
                                width: 14
                                height: 14
                                anchors.verticalCenter: parent.verticalCenter
                                anchors.right: parent.right
                                anchors.rightMargin: 10
                                isActive: false
                                enabled: !removed
                                crossOpacity: 1

                                onItemClicked: {
                                    confirmRemovePluginDialog.pluginIndex = wrapper.delegateIndex
                                    confirmRemovePluginDialog.open()
                                }
                            }
                        }
                    }
                }

                Item {
                    visible: pluginsListView.count == 0
                    anchors.fill: parent

                    StyledText {
                        text: i18.n + qsTr("No plugins available")
                        anchors.centerIn: parent
                        isActive: false
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

                StyledButton {
                    text: i18.n + qsTr("Add plugin")
                    width: 150
                    onClicked: {
                        choosePluginDialog.open()
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                StyledButton {
                    text: i18.n + qsTr("Close")
                    width: 150
                    onClicked: {
                        closePopup()
                    }
                }
            }
        }
    }
}
