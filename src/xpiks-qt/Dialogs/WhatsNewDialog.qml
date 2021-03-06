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
    id: whatsNewComponent
    canMove: false
    canEscapeClose: false
    anchors.fill: parent

    z: 10000

    contentsWidth: 700
    contentsHeight: 500

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
            text: i18.n + qsTr("What's new")
        }

        Rectangle {
            id: slidesHost
            anchors.top: header.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            anchors.topMargin: 10
            anchors.bottom: footer.top
            anchors.bottomMargin: 20
            color: uiColors.defaultControlColor
            clip: true

            property int currentSlideIndex: 0
            property int previousSlideIndex: 0

            // slide pictures should be in 2:1 aspect ratio
            property list<Item> whatsNewSlides: [
                PresentationSlide {
                    parent: slidesHost
                    id: firstSlide
                    title: i18.n + qsTr("Video files support")
                    imageUrl: "qrc:/Graphics/whatsnew/videosupport.png"
                    anchors.fill: parent
                    anchors.margins: 1
                },
                PresentationSlide {
                    parent: slidesHost
                    id: secondSlide
                    title: i18.n + qsTr("Detection of semantical duplicates")
                    imageUrl: "qrc:/Graphics/whatsnew/duplicates.png"
                    anchors.fill: parent
                    anchors.margins: 1
                },
                PresentationSlide {
                    parent: slidesHost
                    id: thirdSlide
                    title: i18.n + qsTr("Presets groups")
                    imageUrl: "qrc:/Graphics/whatsnew/presetsgroups.png"
                    anchors.fill: parent
                    anchors.margins: 1
                },
                PresentationSlide {
                    parent: slidesHost
                    id: fourthSlide
                    title: i18.n + qsTr("Upgraded look and feel")
                    imageUrl: "qrc:/Graphics/whatsnew/lookandfeel.png"
                    anchors.fill: parent
                    anchors.margins: 1
                },
                PresentationSlide {
                    parent: slidesHost
                    id: sixsSlide
                    title: i18.n + qsTr("Bug fixes and improvements")
                    imageUrl: "qrc:/Graphics/whatsnew/gears.png"
                    anchors.fill: parent
                    anchors.margins: 1
                }
            ]

            Component.onCompleted: {
                slidesHost.currentSlideIndex = 0;
                slidesHost.previousSlideIndex = 0;

                for (var i = 0; i < whatsNewSlides.length; ++i) {
                    whatsNewSlides[i].hide();
                }

                whatsNewSlides[0].show(0);
            }

            function showSlide(screenIndex, direction) {
                whatsNewSlides[previousSlideIndex].hide();
                var xVal = direction === -1 ? 400 : -400;
                whatsNewSlides[screenIndex].show(xVal);
            }

            function swipeForward() {
                if (currentSlideIndex < whatsNewSlides.length - 1) {
                    previousSlideIndex = currentSlideIndex;
                    currentSlideIndex = currentSlideIndex + 1;

                    showSlide(currentSlideIndex, -1)
                }
            }

            function swipeBackward() {
                if (currentSlideIndex > 0) {
                    previousSlideIndex = currentSlideIndex;
                    currentSlideIndex = currentSlideIndex - 1;

                    showSlide(currentSlideIndex, 1)
                }
            }

            function isFinished() {
                return currentSlideIndex === whatsNewSlides.length - 1
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

            StyledLink {
                text: i18.n + qsTr("Skip")
                normalLinkColor: uiColors.labelInactiveForeground
                onClicked: {
                    settingsModel.saveCurrentVersion()
                    closePopup()
                }
            }

            Item {
                Layout.fillWidth: true
            }

            StyledLink {
                text: i18.n + qsTr("Previous")
                normalLinkColor: uiColors.labelActiveForeground
                onClicked: {
                    slidesHost.swipeBackward()
                    actionButton.text = qsTr("Next")
                }
            }

            StyledButton {
                id: actionButton
                text: i18.n + qsTr("Next")
                isDefault: true
                width: 100
                onClicked: {
                    if (slidesHost.isFinished()) {
                        settingsModel.saveCurrentVersion()
                        closePopup()
                    } else {
                        slidesHost.swipeForward()
                        if (slidesHost.isFinished()) {
                            actionButton.text = qsTr("Close")
                        }
                    }
                }
            }
        }
    }
}
