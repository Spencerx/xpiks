/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2017 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.2
import QtQuick.Layouts 1.1
import "../Constants" 1.0
import "../Components"
import "../StyledControls"
import "../Dialogs"
import "../Common.js" as Common
import "../Constants/UIConfig.js" as UIConfig
import xpiks 1.0

ColumnLayout {
    anchors.fill: parent
    anchors.leftMargin: 10
    anchors.rightMargin: 10
    anchors.topMargin: 15
    anchors.bottomMargin: 10
    spacing: 0

    StyledBlackButton {
        anchors.left: parent.left
        anchors.right: parent.right
        text: i18.n + qsTr("Apply")
        enabled: {
            var result = (!quickBuffer.isEmpty) && uiManager.hasCurrentEditable;
            if (result) {
                if (mainStackView.areActionsAllowed) {
                    result = (filteredArtItemsModel.selectedArtworksCount <= 1);
                }
            }

            return result;
        }
        implicitHeight: 30
        height: 30
        onClicked: quickBuffer.copyToCurrentEditable()
    }

    Item {
        height: 20
    }

    Item {
        height: childrenRect.height
        anchors.left: parent.left
        anchors.right: parent.right

        StyledText {
            id: titleText
            anchors.left: parent.left
            text: i18.n + qsTr("Title:")
            isActive: false
        }

        StyledText {
            anchors.left: titleText.right
            anchors.leftMargin: 3
            text: "*"
            color: uiColors.destructiveColor
            visible: quickBuffer.hasTitleSpellErrors
        }

        StyledText {
            anchors.right: parent.right
            text: titleTextInput.length
            isActive: false
        }
    }

    Item {
        height: 5
    }

    Rectangle {
        id: anotherRect
        anchors.left: parent.left
        anchors.right: parent.right
        height: 25
        color: uiColors.inputInactiveBackground
        border.color: uiColors.artworkActiveColor
        border.width: titleTextInput.activeFocus ? 1 : 0
        clip: true

        Flickable {
            id: titleFlick
            contentWidth: titleTextInput.paintedWidth
            contentHeight: titleTextInput.paintedHeight
            anchors.fill: parent
            anchors.margins: 5
            clip: true
            flickableDirection: Flickable.HorizontalFlick
            interactive: false
            focus: false

            function ensureVisible(r) {
                if (contentX >= r.x)
                    contentX = r.x;
                else if (contentX+width <= r.x+r.width)
                    contentX = r.x+r.width-width;
            }

            StyledTextEdit {
                id: titleTextInput
                focus: true
                width: paintedWidth > titleFlick.width ? paintedWidth : titleFlick.width
                height: titleFlick.height
                text: quickBuffer.title
                isActive: false
                property string previousText: text
                onTextChanged: {
                    if (text.length > UIConfig.inputsMaxLength) {
                        var cursor = cursorPosition;
                        text = previousText;
                        if (cursor > text.length) {
                            cursorPosition = text.length;
                        } else {
                            cursorPosition = cursor-1;
                        }
                        console.info("Pasting cancelled: text length exceeded maximum")
                    }

                    previousText = text
                    quickBuffer.title = text
                }

                Keys.onBacktabPressed: {
                    event.accepted = true
                }

                Keys.onTabPressed: {
                    descriptionTextInput.forceActiveFocus()
                    descriptionTextInput.cursorPosition = descriptionTextInput.text.length
                }

                Component.onCompleted: {
                    quickBuffer.initTitleHighlighting(titleTextInput.textDocument)
                }

                onCursorRectangleChanged: titleFlick.ensureVisible(cursorRectangle)

                Keys.onPressed: {
                    if(event.matches(StandardKey.Paste)) {
                        var clipboardText = quickClipboard.getText();
                        if (Common.safeInsert(titleTextInput, clipboardText)) {
                            event.accepted = true
                        }
                    } else if ((event.key === Qt.Key_Return) || (event.key === Qt.Key_Enter)) {
                        event.accepted = true
                    }
                }
            }
        }
    }

    Item {
        height: 15
    }

    Item {
        height: childrenRect.height
        anchors.left: parent.left
        anchors.right: parent.right

        StyledText {
            id: descriptionText
            anchors.left: parent.left
            isActive: false
            text: i18.n + qsTr("Description:")
        }

        StyledText {
            anchors.left: descriptionText.right
            anchors.leftMargin: 3
            text: "*"
            color: uiColors.destructiveColor
            visible: quickBuffer.hasDescriptionSpellErrors
        }

        StyledText {
            anchors.right: parent.right
            text: descriptionTextInput.length
            isActive: false
        }
    }

    Item {
        height: 5
    }

    Rectangle {
        id: rect
        anchors.left: parent.left
        anchors.right: parent.right
        height: 70
        color: uiColors.inputInactiveBackground
        border.color: uiColors.artworkActiveColor
        border.width: descriptionTextInput.activeFocus ? 1 : 0
        clip: true

        Flickable {
            id: descriptionFlick
            contentWidth: descriptionTextInput.paintedWidth
            contentHeight: descriptionTextInput.paintedHeight
            anchors.fill: parent
            anchors.margins: 5
            interactive: false
            flickableDirection: Flickable.HorizontalFlick
            clip: true

            function ensureVisible(r) {
                if (contentX >= r.x)
                    contentX = r.x;
                else if (contentX+width <= r.x+r.width)
                    contentX = r.x+r.width-width;
                if (contentY >= r.y)
                    contentY = r.y;
                else if (contentY+height <= r.y+r.height)
                    contentY = r.y+r.height-height;
            }

            StyledTextEdit {
                id: descriptionTextInput
                width: descriptionFlick.width
                height: paintedHeight > descriptionFlick.height ? paintedHeight : descriptionFlick.height
                text: quickBuffer.description
                focus: true
                isActive: false
                property string previousText: text
                property int maximumLength: 380
                onTextChanged: {
                    if (text.length > UIConfig.inputsMaxLength) {
                        var cursor = cursorPosition;
                        text = previousText;
                        if (cursor > text.length) {
                            cursorPosition = text.length;
                        } else {
                            cursorPosition = cursor-1;
                        }
                        console.info("Pasting cancelled: text length exceeded maximum")
                    }

                    previousText = text
                    quickBuffer.description = text
                }

                wrapMode: TextEdit.Wrap
                horizontalAlignment: TextEdit.AlignLeft
                verticalAlignment: TextEdit.AlignTop
                textFormat: TextEdit.PlainText

                Component.onCompleted: {
                    quickBuffer.initDescriptionHighlighting(descriptionTextInput.textDocument)
                }

                Keys.onBacktabPressed: {
                    titleTextInput.forceActiveFocus()
                    titleTextInput.cursorPosition = titleTextInput.text.length
                }

                Keys.onTabPressed: {
                    flv.activateEdit()
                }

                onCursorRectangleChanged: descriptionFlick.ensureVisible(cursorRectangle)

                Keys.onPressed: {
                    if(event.matches(StandardKey.Paste)) {
                        var clipboardText = quickClipboard.getText();
                        if (Common.safeInsert(descriptionTextInput, clipboardText)) {
                            event.accepted = true
                        }
                    } else if ((event.key === Qt.Key_Return) || (event.key === Qt.Key_Enter)) {
                        event.accepted = true
                    }
                }
            }
        }
    }

    Item {
        height: 15
    }

    Item {
        height: childrenRect.height
        anchors.left: parent.left
        anchors.right: parent.right

        StyledText {
            anchors.left: parent.left
            id: keywordsLabel
            isActive: false
            text: i18.n + qsTr("Keywords:")
        }

        StyledText {
            anchors.left: keywordsLabel.right
            anchors.leftMargin: 3
            anchors.top: keywordsLabel.top
            text: "*"
            color: uiColors.destructiveColor
            visible: quickBuffer.hasKeywordsSpellErrors
        }

        StyledText {
            anchors.right: parent.right
            text: quickBuffer.keywordsCount
            isActive: false
        }
    }

    Item {
        height: 5
    }

    Rectangle {
        id: keywordsWrapper
        border.color: uiColors.artworkActiveColor
        border.width: flv.isFocused ? 1 : 0
        Layout.fillHeight: true
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: quickScrollBar.visible ? 10 : 0
        color: uiColors.inputInactiveBackground
        property var keywordsModel: quickBuffer.getBasicModel()
        state: ""

        function removeKeyword(index) {
            quickBuffer.removeKeywordAt(index)
        }

        function removeLastKeyword() {
            quickBuffer.removeLastKeyword()
        }

        function appendKeyword(keyword) {
            var added = quickBuffer.appendKeyword(keyword)
            if (!added) {
                keywordsWrapper.state = "blinked"
                blinkTimer.start()
            }
        }

        function pasteKeywords(keywords) {
            quickBuffer.pasteKeywords(keywords)
        }

        function expandLastKeywordAsPreset() {
            quickBuffer.expandLastKeywordAsPreset();
        }

        EditableTags {
            id: flv
            anchors.fill: parent
            model: keywordsWrapper.keywordsModel
            property int keywordHeight: uiManager.keywordHeight
            scrollStep: keywordHeight
            populateAnimationEnabled: false

            delegate: KeywordWrapper {
                id: kw
                isHighlighted: false
                keywordText: keyword
                hasSpellCheckError: !iscorrect
                hasDuplicate: hasduplicate
                delegateIndex: index
                itemHeight: flv.keywordHeight
                onRemoveClicked: keywordsWrapper.removeKeyword(delegateIndex)
            }

            onTagAdded: {
                keywordsWrapper.appendKeyword(text)
            }

            onRemoveLast: {
                keywordsWrapper.removeLastKeyword()
            }

            onTagsPasted: {
                keywordsWrapper.pasteKeywords(tagsList)
            }

            onBackTabPressed: {
                descriptionTextInput.forceActiveFocus()
                descriptionTextInput.cursorPosition = descriptionTextInput.text.length
            }

            onExpandLastAsPreset: {
                keywordsWrapper.expandLastKeywordAsPreset()
            }

            onRightClickedInside: {
                filteredPresetsModel.searchTerm = ''
            }
        }

        CustomScrollbar {
            id: quickScrollBar
            anchors.topMargin: -5
            anchors.bottomMargin: -5
            anchors.rightMargin: -15
            flickable: flv
        }

        Timer {
            id: blinkTimer
            repeat: false
            interval: 400
            triggeredOnStart: false
            onTriggered: keywordsWrapper.state = ""
        }

        states: State {
            name: "blinked";
            PropertyChanges {
                target: keywordsWrapper;
                border.width: 0
            }
        }
    }

    Item {
        height: 5
    }

    RowLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        spacing: 5

        Item {
            Layout.fillWidth: true
        }

        StyledLink {
            text: i18.n + qsTr("Copy")
            enabled: quickBuffer.keywordsCount > 0
            onClicked: quickClipboard.setText(quickBuffer.getKeywordsString())
        }

        StyledText {
            text: "|"
            verticalAlignment: Text.AlignVCenter
            isActive: false
        }

        StyledLink {
            text: i18.n + qsTr("Clear")
            enabled: quickBuffer.keywordsCount > 0
            onClicked: quickBuffer.clearKeywords()
        }
    }

    ClipboardHelper {
        id: quickClipboard
    }
}
