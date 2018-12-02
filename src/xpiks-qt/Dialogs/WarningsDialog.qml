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
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.1
import QtGraphicalEffects 1.0
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"

StaticDialogBase {
    id: warningsComponent
    anchors.fill: parent

    property variant componentParent
    property bool isRestricted: false

    onClickedOutside: closePopup()

    contentsWidth: 700
    contentsHeight: 580

    contents: ColumnLayout {
        spacing: 10
        anchors.fill: parent
        anchors.margins: 20

        StyledText {
            text: i18.n + qsTr("Warnings")
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: uiColors.defaultControlColor

            StyledScrollView {
                anchors.fill: parent
                anchors.margins: 10

                ListView {
                    id: warningsListView
                    model: warningsModel
                    spacing: 10

                    delegate: Rectangle {
                        property int delegateIndex: index
                        color: uiColors.defaultDarkColor
                        id: imageWrapper
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: 10
                        height: columnRectangle.height
                        radius: 2

                        Item {
                            id: imageItem
                            anchors.left: parent.left
                            anchors.top: parent.top
                            width: 120
                            height: parent.height

                            ColumnLayout {
                                anchors.centerIn: parent
                                anchors.verticalCenterOffset: 7
                                spacing: 7
                                width: 110

                                Item {
                                    width: 90
                                    height: 60
                                    anchors.horizontalCenter: parent.horizontalCenter

                                    Image {
                                        id: artworkImage
                                        anchors.fill: parent
                                        source: "image://cached/" + thumbpath
                                        sourceSize.width: 150
                                        sourceSize.height: 150
                                        fillMode: settingsModel.fitSmallPreview ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                                        asynchronous: true
                                        cache: false
                                    }

                                    Image {
                                        id: videoTypeIconSmall
                                        visible: isvideo
                                        enabled: isvideo
                                        source: "qrc:/Graphics/video-icon-s.png"
                                        fillMode: Image.PreserveAspectFit
                                        sourceSize.width: 150
                                        sourceSize.height: 150
                                        anchors.fill: artworkImage
                                        cache: true
                                    }
                                }

                                StyledText {
                                    Layout.fillWidth: true
                                    elide: Text.ElideMiddle
                                    horizontalAlignment: Text.AlignHCenter
                                    text: basefilename
                                    font.pixelSize: 11
                                    isActive: false
                                }

                                Item {
                                    Layout.fillHeight: true
                                }
                            }
                        }

                        Item {
                            id: columnRectangle
                            anchors.left: imageItem.right
                            anchors.top: parent.top
                            anchors.right: parent.right
                            height: (childrenRect.height < 80) ? 100 : (childrenRect.height + 20)

                            Column {
                                id: warningsTextList
                                spacing: 10

                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                anchors.leftMargin: 10
                                anchors.rightMargin: 10
                                anchors.topMargin: 10

                                Repeater {
                                    id: warningsDescriptions
                                    model: warningsModel.describeWarnings(imageWrapper.delegateIndex)

                                    delegate: RowLayout {
                                        width: warningsTextList.width
                                        height: 10
                                        spacing: 5

                                        Rectangle {
                                            height: 6
                                            width: height
                                            radius: height/2
                                            anchors.verticalCenter: parent.verticalCenter
                                            color: uiColors.inactiveControlColor
                                        }

                                        StyledText {
                                            Layout.fillWidth: true
                                            text: i18.n + modelData
                                            color: uiColors.artworkModifiedColor
                                            anchors.verticalCenter: parent.verticalCenter
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Item {
                anchors.fill: parent
                visible: warningsListView.count == 0

                StyledText {
                    text: i18.n + qsTr("There are no warnings")
                    anchors.centerIn: parent
                    isActive: false
                }
            }
        }

        Item {
            height: 1
        }

        RowLayout {
            spacing: 20
            height: 24

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
