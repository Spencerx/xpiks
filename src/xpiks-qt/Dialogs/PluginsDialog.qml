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
    id: pluginsComponent
    property string logText
    anchors.fill: parent

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

    onClickedOutside: closePopup()

    contentsWidth: 500
    contentsHeight: 400

    contents: Item {
        anchors.fill: parent

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
