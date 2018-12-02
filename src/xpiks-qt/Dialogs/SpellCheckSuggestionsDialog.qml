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
import xpiks 1.0
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

BaseDialog {
    id: spellCheckSuggestionsDialog
    anchors.fill: parent
    property var callbackObject
    property bool initialized: false
    property var spellCheckSuggestionModel: dispatcher.getCommandTarget(UICommand.ReviewSpellingArtwork)

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
                var tmp = mapToItem(spellCheckSuggestionsDialog, mouse.x, mouse.y);
                old_x = tmp.x;
                old_y = tmp.y;
            }

            onPositionChanged: {
                var old_xy = Common.movePopupInsideComponent(spellCheckSuggestionsDialog, dialogWindow, mouse, old_x, old_y);
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
            width: 730
            height: 610
            color: uiColors.popupBackgroundColor
            anchors.centerIn: parent
            Component.onCompleted: anchors.centerIn = undefined

            ColumnLayout {
                spacing: 10
                anchors.fill: parent
                anchors.margins: 20

                RowLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    StyledText {
                        text: i18.n + qsTr("Suggestions")
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledText {
                        text: i18.n + getOriginalText()

                        function getOriginalText() {
                            return spellCheckSuggestionModel.artworksCount === 1 ?
                                        qsTr("1 artwork selected") :
                                        qsTr("%1 artworks selected").arg(spellCheckSuggestionModel.artworksCount)
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    color: uiColors.defaultControlColor

                    Item {
                        visible: suggestionsListView.count == 0
                        anchors.fill: parent

                        StyledText {
                            text: i18.n + qsTr("No suggestions available")
                            isActive: false
                            anchors.centerIn: parent
                        }
                    }

                    StyledScrollView {
                        anchors.fill: parent
                        anchors.margins: 10
                        focus: true

                        ListView {
                            id: suggestionsListView
                            model: spellCheckSuggestionModel
                            focus: true
                            spacing: 5

                            delegate: Rectangle {
                                id: suggestionsWrapper
                                property int delegateIndex: index
                                color: uiColors.panelColor
                                width: parent.width - 10
                                height: suggestionsListRect.height

                                Item {
                                    id: wordsColumn
                                    anchors.left: parent.left
                                    anchors.top: parent.top
                                    width: 100
                                    height: suggestionsListRect.height

                                    Column {
                                        anchors.centerIn: parent
                                        spacing: 3

                                        StyledText {
                                            id: originText
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            verticalAlignment: Text.AlignVCenter
                                            text: word
                                            font.pixelSize: UIConfig.fontPixelSize * settingsModel.keywordSizeScale
                                            color: uiColors.artworkModifiedColor
                                            elide: Text.ElideMiddle
                                        }

                                        StyledText {
                                            anchors.horizontalCenter: parent.horizontalCenter
                                            verticalAlignment: Text.AlignVCenter
                                            visible: replacementorigin != word
                                            enabled: replacementorigin != word
                                            text: replacementorigin
                                            font.pixelSize: 11
                                            color: uiColors.selectedArtworkBackground
                                            elide: Text.ElideMiddle
                                        }
                                    }
                                }

                                Item {
                                    id: suggestionsListRect
                                    anchors.left: wordsColumn.right
                                    anchors.leftMargin: 5
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    height: childrenRect.height + 20

                                    Flow {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 10
                                        anchors.right: parent.right
                                        anchors.rightMargin: 10
                                        anchors.top: parent.top
                                        anchors.topMargin: 10
                                        spacing: 10
                                        focus: true

                                        Repeater {
                                            model: spellCheckSuggestionModel.getSuggestionObject(delegateIndex)

                                            delegate: SuggestionWrapper {
                                                property int suggestionIndex: index
                                                itemHeight: uiManager.keywordHeight
                                                suggestionText: suggestion
                                                isSelected: isselected
                                                onActionClicked: {
                                                    editreplacementindex = suggestionIndex
                                                    spellCheckSuggestionModel.updateSelection()
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Item {
                    height: 1
                }

                RowLayout {
                    spacing: 20

                    StyledButton {
                        text: i18.n + qsTr("Reset all")
                        width: 100
                        onClicked: spellCheckSuggestionModel.resetAllSuggestions()
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledButton {
                        objectName: "replaceButton"
                        text: i18.n + qsTr("Replace")
                        enabled: spellCheckSuggestionModel.anythingSelected
                        isDefault: true
                        width: 100
                        onClicked: {
                            dispatcher.dispatch(UICommand.FixSpelling, true)
                            closePopup()
                        }
                    }

                    StyledButton {
                        text: i18.n + qsTr("Cancel")
                        width: 80
                        onClicked: {
                            dispatcher.dispatch(UICommand.FixSpelling, false)
                            closePopup()
                        }
                    }
                }
            }
        }
    }
}
