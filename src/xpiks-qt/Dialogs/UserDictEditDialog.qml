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
import xpiks 1.0
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

StaticDialogBase {
    id: userDictComponent
    anchors.fill: parent
    property variant componentParent
    property var userDictEditModel: dispatcher.getCommandTarget(UICommand.InitUserDictionary)

    Component.onCompleted: {
        flv.activateEdit()
    }

    function scrollToBottom() {
        var flickable = scrollView.flickableItem
        if (flickable.contentHeight > flickable.height) {
            flickable.contentY = flickable.contentHeight - flickable.height
        } else {
            flickable.contentY = 0
        }
    }

    contentsWidth: 500
    contentsHeight: 300

    contents: ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 0

        StyledText {
            text: i18.n + qsTr("Words added to the user dictionary:")
        }

        Item {
            height: 5
        }

        Rectangle {
            id: keywordsWrapper
            border.color: uiColors.artworkActiveColor
            border.width: flv.isFocused ? 1 : 0
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
            property var keywordsModel: userDictEditModel.getBasicModelObject()

            function removeKeyword(index) {
                userDictEditModel.removeKeywordAt(index)
            }

            function removeLastKeyword() {
                userDictEditModel.removeLastKeyword()
            }

            function appendKeyword(keyword) {
                userDictEditModel.appendKeyword(keyword)
            }

            EditableTags {
                id: flv
                objectName: "editableTags"
                anchors.fill: parent
                model: keywordsWrapper.keywordsModel
                property int keywordHeight: uiManager.keywordHeight
                populateAnimationEnabled: false
                scrollStep: keywordHeight

                delegate: KeywordWrapper {
                    id: kw
                    keywordText: keyword
                    delegateIndex: index
                    hasSpellCheckError: false
                    hasDuplicate: hasduplicate
                    isHighlighted: true
                    itemHeight: flv.keywordHeight
                    closeIconDisabledColor: uiColors.closeIconInactiveColor
                    onRemoveClicked: keywordsWrapper.removeKeyword(delegateIndex)
                }

                onTagAdded: {
                    keywordsWrapper.appendKeyword(text)
                }

                onRemoveLast: {
                    keywordsWrapper.removeLastKeyword()
                }
            }

            CustomScrollbar {
                anchors.topMargin: -5
                anchors.bottomMargin: -5
                anchors.rightMargin: -15
                flickable: flv
            }
        }

        Item {
            height: 20
        }

        RowLayout {
            id: footer
            Layout.fillWidth: true
            height: 24
            spacing: 20

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Save")
                width: 100
                onClicked: {
                    userDictEditModel.saveUserDict()
                    closePopup()
                }
            }

            StyledButton {
                text: i18.n + qsTr("Cancel")
                width: 100
                onClicked: {
                    closePopup()
                }
            }
        }
    }
}
