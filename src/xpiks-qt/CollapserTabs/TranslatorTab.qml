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
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1
import "../Constants" 1.0
import "../Components"
import "../StyledControls"
import "../Dialogs"
import "../Common.js" as Common
import "../Constants/UIConfig.js" as UIConfig

ColumnLayout {
    anchors.fill: parent
    anchors.leftMargin: 10
    anchors.rightMargin: 10
    anchors.topMargin: 20
    anchors.bottomMargin: 20
    spacing: 0

    function initializeTab() {
        dictionariesComboBox.selectedIndex = translationManager.selectedDictionaryIndex
        trTextEdit.forceActiveFocus()
    }

    StyledText {
        text: i18.n + qsTr("Dictionary:")
    }

    Item {
        height: 10
    }

    ComboBoxPopup {
        id: dictionariesComboBox
        model: translationManager.dictionaries
        Layout.fillWidth: true
        height: 28
        itemHeight: 28
        showColorSign: true
        hasLastItemAction: true
        dropDownWidth: width
        glowEnabled: true
        glowTopMargin: 2
        globalParent: appHost
        lastActionText: i18.n + qsTr("Add dictionary...")
        comboboxBackgroundColor: uiColors.popupBackgroundColor

        onLastItemActionInvoked: {
            openDictionaryDialog.open()
        }

        onComboItemSelected: {
            translationManager.selectedDictionaryIndex = dictionariesComboBox.selectedIndex
            trTextEdit.forceActiveFocus()
            trTextEdit.cursorPosition = trTextEdit.text.length
        }
    }

    Item {
        height: 20
    }

    Rectangle {
        Layout.fillWidth: true
        height: 30
        color: uiColors.popupDarkInputBackground

        StyledTextEdit {
            id: trTextEdit
            width: parent.width - 10
            height: parent.height
            clip: true
            anchors.left: parent.left
            anchors.leftMargin: 5
            focus: true
            text: translationManager.query
            onTextChanged: translationManager.query = text
            isActive: false
            selectionColor: uiColors.inputBackgroundColor

            Keys.onBacktabPressed: {
                event.accepted = true
            }

            Keys.onTabPressed: {
                event.accepted = true
            }

            Keys.onPressed: {
                if(event.matches(StandardKey.Paste)) {
                    var clipboardText = clipboard.getText();
                    if (Common.safeInsert(trTextEdit, clipboardText)) {
                        event.accepted = true
                    }
                } else if ((event.key === Qt.Key_Return) || (event.key === Qt.Key_Enter)) {
                    event.accepted = true
                }
            }
        }

        CloseIcon {
            width: 14
            height: 14
            anchors.right: parent.right
            anchors.rightMargin: 5
            enabled: trTextEdit.length > 0
            anchors.verticalCenter: parent.verticalCenter
            onItemClicked: {
                translationManager.clear()
                trTextEdit.forceActiveFocus()
            }
        }
    }

    Item {
        height: 20
    }

    Rectangle {
        Layout.fillWidth: true
        Layout.fillHeight: true
        color: uiColors.popupDarkInputBackground

        Flickable {
            id: trFlickOther
            anchors.fill: parent
            anchors.margins: 10
            clip: true
            contentWidth: trTextEdit.paintedWidth
            contentHeight: trTextEdit.paintedHeight

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
                id: trTextEditOther
                anchors.top: parent.top
                width: trFlickOther.width - 10
                height: trFlickOther.height
                focus: true
                readOnly: true
                text: translationManager.shortTranslation
                selectionColor: uiColors.inputBackgroundColor
                wrapMode: TextEdit.Wrap
                horizontalAlignment: TextEdit.AlignLeft
                verticalAlignment: TextEdit.AlignTop
                textFormat: TextEdit.RichText
                isActive: false

                Component.onCompleted: {
                    // scrollToBottom()
                }

                Keys.onBacktabPressed: {
                    event.accepted = true
                }

                Keys.onTabPressed: {
                    event.accepted = true
                }
            }
        }

        CustomScrollbar {
            anchors.topMargin: -5
            anchors.bottomMargin: -5
            anchors.rightMargin: -5
            flickable: trFlickOther
        }

        Rectangle {
            anchors.fill: parent
            color: uiColors.selectedArtworkBackground
            opacity: 0.2
            visible: translationManager.isBusy
        }
    }

    Item {
        height: 10
    }

    Item {
        Layout.fillWidth: true
        height: childrenRect.height

        StyledLink {
            anchors.right: parent.right
            text: i18.n + qsTr("Show more")
            enabled: translationManager.hasMore
            onClicked: {
                Common.launchDialog("Dialogs/TranslationPreviewDialog.qml",
                                    applicationWindow, {})
            }
        }
    }
}
