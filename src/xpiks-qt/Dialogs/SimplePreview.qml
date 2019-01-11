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
    id: artworkSimplePreview
    anchors.fill: parent
    property string thumbpath

    onClickedOutside: closePopup()

    contentsWidth: 700
    contentsHeight: 500

    contents: Item {
        anchors.fill: parent

        Rectangle {
            id: imageWrapper
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: bottomRow.top
            color: uiColors.defaultDarkColor
            property int imageMargin: 10

            LoaderIcon {
                width: 100
                height: 100
                anchors.centerIn: parent
                running: image.status == Image.Loading
            }

            StyledScrollView {
                id: scrollview
                anchors.fill: parent
                anchors.leftMargin: imageWrapper.imageMargin
                anchors.topMargin: imageWrapper.imageMargin

                Image {
                    id: image
                    source: "image://global/" + artworkSimplePreview.thumbpath
                    property bool isFullSize: false
                    width: isFullSize ? sourceSize.width : (imageWrapper.width - 2*imageWrapper.imageMargin)
                    height: isFullSize ? sourceSize.height : (imageWrapper.height - 2*imageWrapper.imageMargin)
                    fillMode: Image.PreserveAspectFit
                    anchors.centerIn: parent
                    asynchronous: true
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                width: 50
                height: 50
                color: uiColors.defaultDarkColor

                ZoomAmplifier {
                    id: zoomIcon
                    anchors.fill: parent
                    anchors.margins: 10
                    scale: zoomMA.pressed ? 0.9 : 1
                }

                MouseArea {
                    id: zoomMA
                    anchors.fill: parent
                    onClicked: {
                        image.isFullSize = !image.isFullSize
                        zoomIcon.isPlus = !zoomIcon.isPlus
                    }
                }
            }
        }

        Item {
            id: bottomRow
            anchors.bottom: parent.bottom
            height: 50
            anchors.left: parent.left
            anchors.right: parent.right

            StyledButton {
                anchors.right: parent.right
                anchors.verticalCenter: parent.verticalCenter
                anchors.rightMargin: 20
                width: 100
                text: i18.n + qsTr("Close")
                onClicked: {
                    closePopup()
                }
            }
        }
    }
}
