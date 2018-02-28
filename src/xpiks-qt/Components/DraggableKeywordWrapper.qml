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
import "../Constants"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

Rectangle {
    id: itemWrapper
    property int delegateIndex
    property bool isHighlighted
    property bool hasDuplicate : false
    property bool hasSpellCheckError: false
    property string keywordText
    property bool hasPlusSign: false
    property alias itemHeight: tagTextRect.height
    property alias closeIconDisabledColor: closeIcon.disabledColor
    property bool dragDropAllowed: false
    property var dragParent: undefined
    property bool wasDropped: false

    signal removeClicked();
    signal actionDoubleClicked();
    signal actionRightClicked();
    signal spellSuggestionRequested();
    signal moveRequested(int from, int to);

    property bool isDropTarget: dropArea.containsDrag && !keywordMA.drag.active
    property bool isDragSource: keywordMA.drag.active || keywordMA.held

    color: isHighlighted ? uiColors.inputForegroundColor : uiColors.inactiveKeywordBackground
    border.color: uiColors.artworkActiveColor
    border.width: (isDropTarget || wasDropped) ? 1 : 0
    scale: (dragDropAllowed && keywordMA.pressed) ? 0.97 : 1

    width: childrenRect.width
    height: itemHeight

    states: [
        State {
            name: "dragSource"
            when: itemWrapper.isDragSource
            PropertyChanges {
                target: itemWrapper
                opacity: 0.7
                color: uiColors.whiteColor
            }
        }
    ]

    Item {
        id: tagTextRect
        anchors.left: parent.left
        anchors.leftMargin: 5 + (settingsModel.keywordSizeScale - 1)*10
        width: childrenRect.width
        anchors.bottom: parent.bottom
        anchors.top: parent.top

        StyledText {
            id: keywordText
            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter
            opacity: itemWrapper.isDropTarget ? 0.5 : 1
            verticalAlignment: Text.AlignVCenter
            text: itemWrapper.keywordText
            color: itemWrapper.isHighlighted ? uiColors.inactiveControlColor : uiColors.inactiveKeywordForeground
            font.pixelSize: UIConfig.fontPixelSize * settingsModel.keywordSizeScale
        }

        MouseArea {
            id: keywordMA
            hoverEnabled: true
            property bool held: false
            drag.target: itemWrapper.dragDropAllowed ? dragItem : undefined
            onPressed: {
                dragItem.x = 0
                dragItem.y = 0
            }

            onPressAndHold: held = itemWrapper.dragDropAllowed
            drag.smoothed: false
            onReleased: {
                held = false
                if (itemWrapper.dragDropAllowed) {
                    dragItem.Drag.drop()
                }
            }

            anchors.fill: keywordText
            acceptedButtons: Qt.LeftButton | Qt.RightButton
            onDoubleClicked: actionDoubleClicked()
            onClicked: {
                if (mouse.button == Qt.RightButton) {
                    console.log("Click in keyword registered")
                    actionRightClicked()
                }
            }

            preventStealing: true
            propagateComposedEvents: true

            Rectangle {
                id: dragItem
                property int index: itemWrapper.delegateIndex
                color: itemWrapper.color
                border.color: uiColors.defaultControlColor
                border.width: 1
                width: itemWrapper.width*0.85
                height: itemWrapper.height*0.85
                Drag.active: keywordMA.drag.active
                visible: itemWrapper.dragDropAllowed && itemWrapper.isDragSource
                enabled: itemWrapper.dragDropAllowed

                Drag.hotSpot.x: width/2
                Drag.hotSpot.y: height/2

                StyledText {
                    anchors.centerIn: parent
                    text: itemWrapper.keywordText
                    //font.pixelSize: 10
                    color: keywordText.color
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.ClosedHandCursor
                }

                states: [
                    State {
                        name: "dragged"
                        when: itemWrapper.isDragSource
                        ParentChange {
                            target: dragItem
                            parent: itemWrapper.dragParent
                        }
                        PropertyChanges {
                            target: dragItem
                            opacity: 0.9
                        }
                    }
                ]
            }
        }
    }

    Item {
        height: uiManager.keywordHeight
        width: height
        anchors.left: tagTextRect.right
        anchors.leftMargin: 2
        anchors.verticalCenter: parent.verticalCenter
        opacity: itemWrapper.isDropTarget ? 0.5 : 1

        CloseIcon {
            id: closeIcon
            isPlus: itemWrapper.hasPlusSign
            width: 14*settingsModel.keywordSizeScale
            height: 14*settingsModel.keywordSizeScale
            isActive: itemWrapper.isHighlighted
            anchors.centerIn: parent
            onItemClicked: removeClicked()
        }
    }

    Rectangle {
        height: 1.5
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        color: itemWrapper.hasSpellCheckError ? uiColors.destructiveColor : uiColors.artworkActiveColor
        visible: itemWrapper.hasSpellCheckError || itemWrapper.hasDuplicate
    }

    DropArea {
        id: dropArea
        anchors.fill: parent
        enabled: itemWrapper.dragDropAllowed
        onDropped: {
            //console.log(drop.source.index + ' --> ' + itemWrapper.delegateIndex)
            itemWrapper.moveRequested(drag.source.index, itemWrapper.delegateIndex)
        }
    }
}
