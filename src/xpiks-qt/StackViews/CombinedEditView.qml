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
import QtQml 2.2
import xpiks 1.0
import "../Constants"
import "../Common.js" as Common;
import "../Components"
import "../StyledControls"
import "../Constants/UIConfig.js" as UIConfig

Rectangle {
    id: combinedEditComponent
    objectName: "CombinedEditView"
    color: uiColors.selectedArtworkBackground

    property variant componentParent
    property variant combinedArtworks: dispatcher.getCommandTarget(UICommand.SetupEditSelectedArtworks)
    property var autoCompleteBox

    property bool wasLeftSideCollapsed

    function onAutoCompleteClose() {
        autoCompleteBox = undefined
    }

    function closePopup() {
        closeAutoComplete()
        mainStackView.pop({immediate: true})
        restoreLeftPane()
    }

    function restoreLeftPane() {
        if (mainStackView.depth === 1) {
            if (!wasLeftSideCollapsed) {
                expandLeftPane()
            }
        }
    }

    function closeAutoComplete() {
        if (typeof combinedEditComponent.autoCompleteBox !== "undefined") {
            combinedEditComponent.autoCompleteBox.closePopup()
        }
    }

    function doRemoveSelectedArtworks() {
        combinedArtworks.removeSelectedArtworks()
    }

    function openDuplicatesView() {
        dispatcher.dispatch(UICommand.ReviewDuplicatesCombined, {})
    }

    function suggestKeywords() {
        var callbackObject = {
            promoteKeywords: function(keywords) {
                combinedArtworks.pasteKeywords(keywords)
            }
        }

        dispatcher.dispatch(UICommand.InitSuggestionCombined, callbackObject)
    }

    function fixSpelling() {
        dispatcher.dispatch(UICommand.ReviewSpellingCombined, {})
    }

    function editInPlainText() {
        var callbackObject = {
            onSuccess: function(text, spaceIsSeparator) {
                dispatcher.dispatch(UICommand.PlainTextEdit, {
                                        text: text,
                                        spaceIsSeparator: spaceIsSeparator})
            },
            onClose: function() {
                flv.activateEdit()
            }
        }

        Common.launchDialog("Dialogs/PlainTextKeywordsDialog.qml",
                            componentParent,
                            {
                                callbackObject: callbackObject,
                                keywordsText: combinedArtworks.getKeywordsString(),
                                keywordsModel: combinedArtworks.getBasicModelObject()
                            });
    }

    MessageDialog {
            id: confirmRemoveArtworksDialog
            property int itemsCount
            title: i18.n + qsTr("Confirmation")
            text: i18.n + qsTr("Are you sure you want to remove %1 item(s)?").arg(itemsCount)
            standardButtons: StandardButton.Yes | StandardButton.No
            onYes: {
                doRemoveSelectedArtworks()
            }
    }

    MessageDialog {
        id: clearKeywordsDialog

        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Clear all keywords?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: combinedArtworks.clearKeywords()
    }

    Menu {
        id: wordRightClickMenu
        property string word
        property int keywordIndex
        property bool showAddToDict: true
        property bool showExpandPreset: false

        function popupIfNeeded() {
            if (showAddToDict || showExpandPreset) {
                popup()
            }
        }

        MenuItem {
            visible: wordRightClickMenu.showAddToDict
            text: i18.n + qsTr("Add \"%1\" to dictionary").arg(wordRightClickMenu.word)
            onTriggered: dispatcher.dispatch(UICommand.AddToUserDictionary, wordRightClickMenu.word)
        }

        Menu {
            id: presetSubMenu
            visible: wordRightClickMenu.showExpandPreset
            title: i18.n + qsTr("Expand as preset")

            Instantiator {
                id : presetsInstantiator
                model: filteredPresetsModel
                onObjectAdded: presetSubMenu.insertItem( index, object )
                onObjectRemoved: presetSubMenu.removeItem( object )
                delegate: MenuItem {
                    text: name
                    onTriggered: {
                        var presetID = filteredPresetsModel.getOriginalID(index)
                        combinedArtworks.expandPreset(wordRightClickMenu.keywordIndex, presetID);
                    }
                }
            }
        }
    }

    Menu {
        id: presetsMenu

        Menu {
            id: subMenu
            property var defaultGroupModel: presetsGroups.getDefaultGroupModel()
            title: i18.n + qsTr("Insert preset")

            Instantiator {
                model: presetsGroups
                onObjectAdded: subMenu.insertItem( index, object )
                onObjectRemoved: subMenu.removeItem( object )
                delegate: Menu {
                    id: groupMenu
                    property int delegateIndex: index
                    property var groupModel: presetsGroups.getGroupModel(index)
                    title: gname

                    Instantiator {
                        model: groupMenu.groupModel
                        onObjectAdded: groupMenu.insertItem( index, object )
                        onObjectRemoved: groupMenu.removeItem( object )

                        delegate: MenuItem {
                            text: name
                            onTriggered: {
                                combinedArtworks.addPreset(groupMenu.groupModel.getOriginalID(index));
                            }
                        }
                    }
                }
            }

            Instantiator {
                model: subMenu.defaultGroupModel
                onObjectAdded: subMenu.insertItem( index, object )
                onObjectRemoved: subMenu.removeItem( object )

                delegate: MenuItem {
                    text: name
                    onTriggered: {
                        combinedArtworks.addPreset(subMenu.defaultGroupModel.getOriginalID(index));
                    }
                }
            }
        }
    }

    Menu {
        id: keywordsMoreMenu

        MenuItem {
            text: i18.n + qsTr("Suggest keywords")
            onTriggered: {
                suggestKeywords()
            }
        }

        MenuSeparator { }

        MenuItem {
            text: i18.n + qsTr("Fix spelling")
            enabled: {
                return keywordsWrapper.keywordsModel ?
                            (keywordsWrapper.keywordsModel.hasKeywordsSpellErrors ||
                             keywordsWrapper.keywordsModel.hasTitleSpellErrors ||
                             keywordsWrapper.keywordsModel.hasDescriptionSpellErrors)
                          : false
            }
            onTriggered: {
                fixSpelling()
            }
        }

        MenuItem {
            text: i18.n + qsTr("Show duplicates")
            enabled: keywordsWrapper.keywordsModel ? keywordsWrapper.keywordsModel.hasDuplicates : false
            onTriggered: {
                openDuplicatesView()
            }
        }

        MenuSeparator { }

        MenuItem {
            text: i18.n + qsTr("Copy")
            enabled: combinedArtworks.keywordsCount > 0
            onTriggered: {
                clipboard.setText(combinedArtworks.getKeywordsString())
            }
        }

        MenuItem {
            text: i18.n + qsTr("Paste")
            onTriggered: {
                flv.paste()
                flv.activateEdit()
            }
        }

        MenuSeparator { }

        MenuItem {
            text: i18.n + qsTr("Edit in plain text")
            onTriggered: {
                editInPlainText()
            }
        }

        MenuItem {
            text: i18.n + qsTr("Clear")
            enabled: combinedArtworks.keywordsCount > 0
            onTriggered: {
                clearKeywordsDialog.open()
            }
        }
    }

    Menu {
        id: contextMenu
        property string thumbPath

        MenuItem {
            text: i18.n + qsTr("Preview")
            onTriggered: {
                Common.launchDialog("Dialogs/SimplePreview.qml",
                                    componentParent,
                                    {
                                        thumbpath: contextMenu.thumbPath
                                    })
            }
        }
    }

    Menu {
        id: dotsMenu

        MenuItem {
            text: i18.n + qsTr("Assign from selected")
            onTriggered: {
                combinedArtworks.assignFromSelected()
                combinedArtworks.unselectAllItems()
            }
        }
    }

    Stack.onStatusChanged: {
        console.log("CombinedEditView StackView status changed: " + Stack.status)
        if (Stack.status == Stack.Active) {
            focus = true

            titleTextInput.forceActiveFocus()
            titleTextInput.cursorPosition = titleTextInput.text.length
        }
    }

    Connections {
        target: xpiksApp
        onGlobalBeforeDestruction: {
            console.debug("UI:EditArtworkHorizontalDialog # globalBeforeDestruction")
            //closePopup()
        }
    }

    Connections {
        target: combinedArtworks
        onRequestCloseWindow: {
            closePopup();
        }
    }

    Connections {
        target: acSource

        onCompletionsAvailable: {
            acSource.initializeCompletions()

            if (typeof combinedEditComponent.autoCompleteBox !== "undefined") {
                // update completion
                return
            }

            var directParent = combinedEditComponent;
            var currWordStartRect = flv.editControl.getCurrentWordStartRect()

            var tmp = flv.editControl.mapToItem(directParent,
                                                currWordStartRect.x - 17,
                                                flv.editControl.height + 1)

            var visibleItemsCount = Math.min(acSource.getCount(), 5);
            var popupHeight = visibleItemsCount * (25 + 1) + 10

            var isBelow = (tmp.y + popupHeight) < directParent.height;

            var options = {
                model: acSource.getCompletionsModelObject(),
                autoCompleteSource: acSource,
                editableTags: flv,
                isBelowEdit: isBelow,
                "anchors.left": directParent.left,
                "anchors.leftMargin": Math.min(tmp.x, directParent.width - 200)
            }

            if (isBelow) {
                options["anchors.top"] = directParent.top
                options["anchors.topMargin"] = tmp.y
            } else {
                options["anchors.bottom"] = directParent.bottom
                options["anchors.bottomMargin"] = directParent.height - tmp.y + flv.editControl.height
            }

            var component = Qt.createComponent("../Components/CompletionBox.qml");
            if (component.status !== Component.Ready) {
                console.warn("Component Error: " + component.errorString());
            } else {
                var instance = component.createObject(directParent, options);

                instance.boxDestruction.connect(combinedEditComponent.onAutoCompleteClose)
                instance.itemSelected.connect(flv.acceptCompletion)
                combinedEditComponent.autoCompleteBox = instance

                instance.openPopup()
            }
        }
    }

    Rectangle {
        id: leftSpacer
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: appHost.leftSideCollapsed ? 0 : 2
        color: uiColors.defaultDarkerColor
    }

    Rectangle {
        id: topHeader
        anchors.left: leftSpacer.right
        anchors.right: parent.right
        anchors.top: parent.top
        height: 55
        color: uiColors.defaultDarkColor

        RowLayout {
            anchors.leftMargin: 10
            anchors.rightMargin: 20
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            spacing: 20

            StyledButton {
                width: 100
                text: i18.n + qsTr("Cancel")
                onClicked: {
                    flv.onBeforeClose()
                    dispatcher.dispatch(UICommand.EditSelectedArtworks, false)
                    closePopup()
                }
            }

            StyledButton {
                text: i18.n + qsTr("Save")
                width: 100
                isDefault: true
                onClicked: {
                    flv.onBeforeClose()
                    dispatcher.dispatch(UICommand.EditSelectedArtworks, true)
                    closePopup()
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }

    Item {
        id: mainRect
        property real spacing: 5
        anchors.left: leftSpacer.right
        anchors.right: parent.right
        anchors.top: topHeader.bottom
        anchors.bottom: bottomActionsPane.top
        property bool isWideEnough: width > 650
        anchors.leftMargin: 10
        anchors.rightMargin: 15
        anchors.topMargin: 10
        anchors.bottomMargin: 10

        RowLayout {
            id: titleWrapper
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50
            spacing: 0

            Item {
                width: 25
                height: parent.height

                StyledCheckbox {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 5
                    anchors.topMargin: 25
                    text: ''
                    id: titleCheckBox
                    onClicked: {
                        combinedArtworks.changeTitle = checked
                        if (checked) { titleTextInput.forceActiveFocus(); }
                    }

                    Component.onCompleted: titleCheckBox.checked = combinedArtworks.changeTitle
                }
            }

            Item {
                Layout.fillWidth: true
                height: parent.height
                enabled: combinedArtworks.changeTitle

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 2

                    Item {
                        height: 1
                    }

                    RowLayout {
                        spacing: 3

                        StyledText {
                            text: i18.n + qsTr("Title:")
                        }

                        StyledText {
                            text: "*"
                            color: uiColors.destructiveColor
                            visible: combinedArtworks.hasTitleSpellErrors
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        StyledText {
                            text: titleTextInput.length
                        }
                    }

                    Rectangle {
                        id: anotherRect
                        Layout.fillWidth: true
                        height: 25
                        color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
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
                                objectName: "titleTextInput"
                                focus: true
                                width: paintedWidth > titleFlick.width ? paintedWidth : titleFlick.width
                                height: titleFlick.height
                                text: combinedArtworks.title
                                userDictEnabled: true
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
                                    combinedArtworks.title = text
                                }

                                onActionRightClicked: {
                                    var showAddToDict = combinedArtworks.hasTitleWordSpellError(rightClickedWord)
                                    wordRightClickMenu.showAddToDict = showAddToDict
                                    wordRightClickMenu.word = rightClickedWord
                                    wordRightClickMenu.showExpandPreset = false
                                    wordRightClickMenu.popupIfNeeded()
                                }

                                Keys.onBacktabPressed: {
                                    event.accepted = true
                                }

                                Keys.onTabPressed: {
                                    if (combinedArtworks.changeDescription) {
                                        descriptionTextInput.forceActiveFocus()
                                        descriptionTextInput.cursorPosition = descriptionTextInput.text.length
                                        event.accepted = true
                                    } else if (combinedArtworks.changeKeywords) {
                                        flv.activateEdit()
                                        event.accepted = true
                                    }
                                }

                                Component.onCompleted: {
                                    uiManager.initTitleHighlighting(
                                                combinedArtworks.getBasicModelObject(),
                                                titleTextInput.textDocument)
                                }

                                onCursorRectangleChanged: titleFlick.ensureVisible(cursorRectangle)

                                Keys.onPressed: {
                                    if(event.matches(StandardKey.Paste)) {
                                        var clipboardText = clipboard.getText();
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
                }
            }
        }

        RowLayout {
            id: descriptionWrapper
            anchors.top: titleWrapper.bottom
            anchors.topMargin: mainRect.spacing
            anchors.left: parent.left
            anchors.right: parent.right
            height: 85
            spacing: 0

            Item {
                width: 25
                height: parent.height

                StyledCheckbox {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 5
                    anchors.topMargin: 25
                    text: ''
                    id: descriptionCheckBox
                    objectName: "descriptionCheckBox"
                    /*indicatorWidth: 24
                    indicatorHeight: 24*/
                    onClicked: {
                        combinedArtworks.changeDescription = checked
                        if (checked) { descriptionTextInput.forceActiveFocus(); }
                    }

                    Component.onCompleted: descriptionCheckBox.checked = combinedArtworks.changeDescription
                }
            }

            Item {
                Layout.fillWidth: true
                height: parent.height
                enabled: combinedArtworks.changeDescription

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 2

                    Item {
                        height: 1
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 3

                        StyledText {
                            text: i18.n + qsTr("Description:")
                        }

                        StyledText {
                            //anchors.left: titleText.right
                            //anchors.leftMargin: 3
                            text: "*"
                            color: uiColors.destructiveColor
                            visible: combinedArtworks.hasDescriptionSpellErrors
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        StyledText {
                            text: descriptionTextInput.length
                        }
                    }

                    Rectangle {
                        id: rect
                        Layout.fillWidth: true
                        height: 60
                        color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                        border.color: uiColors.artworkActiveColor
                        border.width: descriptionTextInput.activeFocus ? 1 : 0
                        clip: true

                        Flickable {
                            id: descriptionFlick
                            contentWidth: descriptionTextInput.paintedWidth
                            contentHeight: descriptionTextInput.paintedHeight
                            anchors.fill: parent
                            anchors.margins: 5
                            anchors.verticalCenter: parent.verticalCenter
                            interactive: false
                            flickableDirection: Flickable.HorizontalFlick
                            clip: true
                            focus: false

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
                                objectName: "descriptionTextInput"
                                width: descriptionFlick.width
                                height: paintedHeight > descriptionFlick.height ? paintedHeight : descriptionFlick.height
                                text: combinedArtworks.description
                                focus: true
                                userDictEnabled: true
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
                                    combinedArtworks.description = text
                                }

                                onActionRightClicked: {
                                    var showAddToDict = combinedArtworks.hasDescriptionWordSpellError(rightClickedWord)
                                    wordRightClickMenu.showAddToDict = showAddToDict
                                    wordRightClickMenu.word = rightClickedWord
                                    wordRightClickMenu.showExpandPreset = false
                                    wordRightClickMenu.popupIfNeeded()
                                }

                                wrapMode: TextEdit.Wrap
                                horizontalAlignment: TextEdit.AlignLeft
                                verticalAlignment: TextEdit.AlignTop
                                textFormat: TextEdit.PlainText

                                Component.onCompleted: {
                                    uiManager.initDescriptionHighlighting(
                                                combinedArtworks.getBasicModelObject(),
                                                descriptionTextInput.textDocument)
                                }

                                onCursorRectangleChanged: descriptionFlick.ensureVisible(cursorRectangle)

                                Keys.onBacktabPressed: {
                                    if (combinedArtworks.changeTitle) {
                                        titleTextInput.forceActiveFocus()
                                        titleTextInput.cursorPosition = titleTextInput.text.length
                                        event.accepted = true
                                    }
                                }

                                Keys.onTabPressed: {
                                    if (combinedArtworks.changeKeywords) {
                                        flv.activateEdit()
                                        event.accepted = true
                                    }
                                }

                                Keys.onPressed: {
                                    if(event.matches(StandardKey.Paste)) {
                                        var clipboardText = clipboard.getText();
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
                }
            }
        }

        Item {
            anchors.top: descriptionWrapper.bottom
            anchors.topMargin: mainRect.spacing
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Item {
                id: checkboxPane
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 25

                StyledCheckbox {
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.leftMargin: 5
                    anchors.topMargin: 25
                    text: ''
                    id: keywordsCheckBox
                    onClicked: {
                        combinedArtworks.changeKeywords = checked
                        if (checked) { flv.activateEdit(); }
                    }

                    Component.onCompleted: keywordsCheckBox.checked = combinedArtworks.changeKeywords
                }
            }

            Item {
                anchors.left: checkboxPane.right
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                enabled: combinedArtworks.changeKeywords

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 3
                    spacing: 2

                    Item {
                        height: 1
                    }

                    RowLayout {
                        spacing: 3

                        StyledText {
                            id: keywordsLabel
                            text: i18.n + qsTr("Keywords:")
                        }

                        StyledText {
                            text: i18.n + qsTr("(comma-separated)")
                            isActive: false
                        }

                        StyledText {
                            text: "*"
                            color: uiColors.destructiveColor
                            visible: combinedArtworks.hasKeywordsSpellErrors
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        StyledText {
                            text: combinedArtworks.keywordsCount
                        }
                    }

                    Rectangle {
                        id: keywordsWrapper
                        border.color: uiColors.artworkActiveColor
                        border.width: flv.isFocused ? 1 : 0
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                        property var keywordsModel: combinedArtworks.getBasicModelObject()
                        state: ""

                        function removeKeyword(index) {
                            combinedArtworks.removeKeywordAt(index)
                        }

                        function removeLastKeyword() {
                            combinedArtworks.removeLastKeyword()
                        }

                        function appendKeyword(keyword) {
                            var added = combinedArtworks.appendKeyword(keyword)
                            if (!added) {
                                keywordsWrapper.state = "blinked"
                                blinkTimer.start()
                            }
                        }

                        function pasteKeywords(keywordsList) {
                            combinedArtworks.pasteKeywords(keywordsList)
                        }

                        function expandLastKeywordAsPreset() {
                            combinedArtworks.expandLastKeywordAsPreset();
                        }

                        EditableTags {
                            id: flv
                            objectName: "editableTags"
                            anchors.fill: parent
                            model: keywordsWrapper.keywordsModel
                            property int keywordHeight: uiManager.keywordHeight
                            populateAnimationEnabled: false
                            scrollStep: keywordHeight
                            property int droppedIndex: -1
                            onDroppedIndexChanged: dropTimer.start()

                            function acceptCompletion(completionID) {
                                if (acSource.isPreset(completionID)) {
                                    dispatcher.dispatch(UICommand.AcceptPresetCompletionForCombined, completionID)
                                    flv.editControl.acceptCompletion('')
                                } else {
                                    var completion = acSource.getCompletion(completionID)
                                    flv.editControl.acceptCompletion(completion)
                                }
                            }

                            delegate: DraggableKeywordWrapper {
                                id: kw
                                isHighlighted: combinedArtworks.changeKeywords
                                keywordText: keyword
                                hasSpellCheckError: !iscorrect
                                hasDuplicate: hasduplicate
                                delegateIndex: index
                                itemHeight: flv.keywordHeight
                                closeIconDisabledColor: uiColors.closeIconInactiveColor
                                onRemoveClicked: keywordsWrapper.removeKeyword(delegateIndex)
                                dragDropAllowed: switcher.keywordsDragDropEnabled
                                dragParent: flv
                                wasDropped: flv.droppedIndex == delegateIndex
                                onActionDoubleClicked: {
                                    // https://github.com/ribtoks/xpiks/issues/626
                                    if (kw.isDropTarget) { return; }

                                    var callbackObject = {
                                        onSuccess: function(replacement) {
                                            combinedArtworks.editKeyword(kw.delegateIndex, replacement)
                                        },
                                        onClose: function() {
                                            flv.activateEdit()
                                        }
                                    }

                                    Common.launchDialog("Dialogs/EditKeywordDialog.qml",
                                                        componentParent,
                                                        {
                                                            callbackObject: callbackObject,
                                                            previousKeyword: keyword,
                                                            keywordIndex: kw.delegateIndex,
                                                            keywordsModel: keywordsWrapper.keywordsModel
                                                        })
                                }

                                onActionRightClicked: {
                                    wordRightClickMenu.showAddToDict = !iscorrect
                                    var keyword = kw.keywordText
                                    wordRightClickMenu.word = keyword
                                    filteredPresetsModel.searchTerm = keyword
                                    wordRightClickMenu.showExpandPreset = (filteredPresetsModel.getItemsCount() !== 0 )
                                    wordRightClickMenu.keywordIndex = kw.delegateIndex
                                    wordRightClickMenu.popupIfNeeded()
                                }

                                onMoveRequested: {
                                    if (from !== to) {
                                        flv.droppedIndex = to
                                    }

                                    if (combinedArtworks.moveKeyword(from, to)) {
                                        console.debug("Just dropped to " + to)
                                    } else {
                                        flv.droppedIndex = -1
                                    }
                                }
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
                                if (combinedArtworks.changeDescription) {
                                    descriptionTextInput.forceActiveFocus()
                                    descriptionTextInput.cursorPosition = descriptionTextInput.text.length
                                } else if (combinedArtworks.changeTitle) {
                                    titleTextInput.forceActiveFocus()
                                    titleTextInput.cursorPosition = titleTextInput.text.length
                                }
                            }

                            onCompletionRequested: {
                                dispatcher.dispatch(UICommand.GenerateCompletions, prefix)
                            }

                            onExpandLastAsPreset: {
                                keywordsWrapper.expandLastKeywordAsPreset()
                            }

                            onRightClickedInside: {
                                filteredPresetsModel.searchTerm = ''
                                presetsMenu.popup()
                            }
                        }

                        CustomScrollbar {
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

                        Timer {
                            id: dropTimer
                            repeat: false
                            interval: 1000
                            triggeredOnStart: false
                            onTriggered: flv.droppedIndex = -1
                        }

                        states: State {
                            name: "blinked";
                            PropertyChanges {
                                target: keywordsWrapper;
                                border.width: 0
                            }
                        }
                    }

                    Item { height: 3 }

                    RowLayout {
                        Layout.fillWidth: true
                        Layout.leftMargin: 3
                        Layout.rightMargin: 3
                        height: 15
                        spacing: 5

                        StyledCheckbox {
                            id: appendKeywordsCheckbox
                            text: i18.n + qsTr("Only append new keywords")
                            onClicked: combinedArtworks.appendKeywords = checked
                            Component.onCompleted: appendKeywordsCheckbox.checked = combinedArtworks.appendKeywords
                        }

                        Item {
                            Layout.fillWidth: true
                        }

                        StyledLink {
                            id: fixSpellingLink
                            objectName: "fixSpellingLink"
                            text: i18.n + qsTr("Fix spelling")
                            property bool canBeShown: {
                                return keywordsWrapper.keywordsModel ?
                                            (keywordsWrapper.keywordsModel.hasKeywordsSpellErrors ||
                                             keywordsWrapper.keywordsModel.hasTitleSpellErrors ||
                                             keywordsWrapper.keywordsModel.hasDescriptionSpellErrors)
                                          : false
                            }
                            enabled: canBeShown
                            visible: canBeShown
                            onClicked: { fixSpelling() }
                        }

                        StyledText {
                            visible: fixSpellingLink.canBeShown
                            enabled: fixSpellingLink.canBeShown
                            text: "|"
                            verticalAlignment: Text.AlignVCenter
                        }

                        StyledLink {
                            id: removeDuplicatesText
                            text: i18.n + qsTr("Show duplicates")
                            property bool canBeShown: keywordsWrapper.keywordsModel ? keywordsWrapper.keywordsModel.hasDuplicates : false
                            enabled: canBeShown
                            visible: canBeShown
                            onClicked: {
                                openDuplicatesView()
                            }
                        }

                        StyledText {
                            visible: removeDuplicatesText.canBeShown
                            enabled: removeDuplicatesText.canBeShown
                            text: "|"
                            verticalAlignment: Text.AlignVCenter
                        }

                        StyledLink {
                            id: suggestLink
                            text: i18.n + qsTr("Suggest keywords")
                            property bool canBeShown: (combinedArtworks.keywordsCount < warningsModel.minKeywordsCount) || (mainRect.isWideEnough)
                            visible: canBeShown
                            enabled: canBeShown
                            onClicked: {
                                suggestKeywords()
                            }
                        }

                        StyledText {
                            visible: suggestLink.canBeShown
                            enabled: suggestLink.canBeShown
                            text: "|"
                            verticalAlignment: Text.AlignVCenter
                        }

                        StyledLink {
                            id: copyLink
                            property bool canBeShown: combinedArtworks.keywordsCount > 0
                            text: i18.n + qsTr("Copy")
                            enabled: canBeShown
                            visible: canBeShown
                            onClicked: clipboard.setText(combinedArtworks.getKeywordsString())
                        }

                        StyledText {
                            enabled: copyLink.canBeShown
                            visible: copyLink.canBeShown
                            text: "|"
                            verticalAlignment: Text.AlignVCenter
                        }

                        StyledLink {
                            id: clearLink
                            property bool canBeShown: combinedArtworks.keywordsCount > 0
                            text: i18.n + qsTr("Clear")
                            enabled: canBeShown
                            visible: canBeShown
                            onClicked: clearKeywordsDialog.open()
                        }

                        StyledText {
                            enabled: clearLink.canBeShown
                            visible: clearLink.canBeShown
                            text: "|"
                            verticalAlignment: Text.AlignVCenter
                        }

                        Item {
                            width: childrenRect.width
                            height: moreLink.height

                            StyledText {
                                id: moreLink
                                color: enabled ? (moreMA.pressed ?
                                                      uiColors.linkClickedColor :
                                                      uiColors.artworkActiveColor) :
                                                 (isActive ? uiColors.labelActiveForeground :
                                                             uiColors.labelInactiveForeground)
                                text: i18.n + qsTr("More")
                                anchors.verticalCenter: parent.verticalCenter
                                // \u25BE - triangle
                            }

                            TriangleElement {
                                anchors.left: moreLink.right
                                anchors.leftMargin: 4
                                anchors.verticalCenter: parent.verticalCenter
                                // anchors.verticalCenterOffset: isFlipped ? height*0.3 : 0
                                //anchors.verticalCenter: parent.verticalCenter
                                color: moreLink.color
                                isFlipped: true
                                width: 8
                                height: 6
                            }

                            MouseArea {
                                id: moreMA
                                anchors.fill: parent
                                hoverEnabled: true
                                cursorShape: containsMouse ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onClicked: {
                                    // strange bug with clicking on the keywords field
                                    if (!containsMouse) { return; }

                                    keywordsMoreMenu.popup()
                                }
                            }
                        }

                        Item {
                            width: 1
                        }
                    }
                }
            }
        }
    }

    Rectangle {
        id: bottomActionsPane
        anchors.left: leftSpacer.right
        anchors.right: parent.right
        anchors.bottom: bottomPane.top
        height: 55
        color: uiColors.defaultDarkColor

        RowLayout {
            spacing: 10
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 20
            anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter

            StyledButton {
                objectName: "copyToQuickBufferButton"
                text: i18.n + qsTr("Copy to Quick Buffer")
                width: 160
                enabled: (combinedArtworks.title.length > 0) || (combinedArtworks.description.length > 0) || (combinedArtworks.keywordsCount > 0)
                onClicked: {
                    dispatcher.dispatch(UICommand.CopyCombinedToQuickBuffer, {})
                }
            }

            Item {
                Layout.fillWidth: true
            }

            StyledButton {
                text: i18.n + qsTr("Remove selected")
                width: 150
                tooltip: i18.n + qsTr("Remove selected artworks from this dialog")
                enabled: combinedArtworks.selectedArtworksCount > 0
                onClicked: {
                    confirmRemoveArtworksDialog.itemsCount = combinedArtworks.selectedArtworksCount
                    confirmRemoveArtworksDialog.open()
                }
            }

            DotsButton {
                defaultDotsColor: enabled ? uiColors.labelInactiveForeground : uiColors.inactiveControlColor
                enabled: combinedArtworks.selectedArtworksCount > 0
                onDotsClicked: dotsMenu.popup()
            }
        }
    }

    Rectangle {
        id: bottomPane
        anchors.left: leftSpacer.right
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 110
        color: uiColors.panelColor

        Item {
            id: selectPrevButton
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 40
            enabled: (rosterListView.contentWidth > rosterListView.width) && (rosterListView.contentX > 0)

            Rectangle {
                anchors.fill: parent
                color: enabled ? (prevButtonMA.containsMouse ? uiColors.defaultControlColor : uiColors.leftSliderColor) : uiColors.panelColor
            }

            TriangleElement {
                width: 7
                height: 14
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: isFlipped ? -1 : +1
                isVertical: true
                isFlipped: true
                color: {
                    if (enabled) {
                        return prevButtonMA.pressed ? uiColors.inputForegroundColor : uiColors.artworkActiveColor
                    } else {
                        return uiColors.inputBackgroundColor
                    }
                }
            }

            MouseArea {
                id: prevButtonMA
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    rosterListView.scrollHorizontally(-rosterListView.height)
                }
            }
        }

        ListView {
            id: rosterListView
            boundsBehavior: Flickable.StopAtBounds
            orientation: ListView.Horizontal
            anchors.left: selectPrevButton.right
            anchors.right: selectNextButton.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            model: combinedArtworks
            highlightFollowsCurrentItem: false
            highlightMoveDuration: 0
            flickableDirection: Flickable.HorizontalFlick
            interactive: false
            focus: true
            clip: true
            spacing: 0

            function scrollHorizontally(shiftX) {
                var flickable = rosterListView
                var epsilon = 1e-6

                if (shiftX > epsilon) { // up/right
                    var maxScrollPos = flickable.contentWidth - flickable.width
                    flickable.contentX = Math.min(maxScrollPos, flickable.contentX + shiftX)
                } else if (shiftX < -epsilon) { // bottom/left
                    flickable.contentX = Math.max(0, flickable.contentX + shiftX)
                }
            }

            displaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 230 }
            }

            addDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 230 }
            }

            removeDisplaced: Transition {
                NumberAnimation { properties: "x,y"; duration: 230 }
            }

            delegate: Rectangle {
                id: cellItem
                property int delegateIndex: index
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: bottomPane.height
                color: isselected ? uiColors.panelSelectedColor : "transparent"
                property string thumbPath: thumbpath

                Rectangle {
                    anchors.fill: parent
                    visible: !isselected && imageMA.containsMouse
                    color: uiColors.panelSelectedColor
                    opacity: 0.6
                }

                Rectangle {
                    color: imageMA.containsMouse ? uiColors.artworkBackground : uiColors.defaultDarkerColor
                    anchors.fill: parent
                    anchors.margins: 15
                }

                Image {
                    id: artworkImage
                    anchors.fill: parent
                    anchors.margins: 15
                    source: "image://cached/" + thumbpath
                    sourceSize.width: 150
                    sourceSize.height: 150
                    fillMode: settingsModel.fitSmallPreview ? Image.PreserveAspectFit : Image.PreserveAspectCrop
                    asynchronous: true
                    // caching is implemented on different level
                    cache: false
                }

                Rectangle {
                    anchors.right: artworkImage.right
                    anchors.bottom: artworkImage.bottom
                    width: 20
                    height: 20
                    color: uiColors.defaultDarkColor
                    property bool isVideo: isvideo
                    property bool isVector: hasvectorattached
                    visible: isVideo || isVector
                    enabled: isVideo || isVector

                    Image {
                        id: typeIcon
                        anchors.fill: parent
                        source: parent.isVector ? "qrc:/Graphics/vector-icon.svg" : (parent.isVideo ? "qrc:/Graphics/video-icon.svg" : "")
                        sourceSize.width: 20
                        sourceSize.height: 20
                        cache: true
                    }
                }

                Rectangle {
                    anchors.fill: parent
                    color: uiColors.defaultControlColor
                    visible: isselected
                    opacity: imageMA.containsMouse ? 0.6 : 0.7
                }

                SelectedIcon {
                    opacity: imageMA.containsMouse ? 0.85 : 1
                    width: parent.width * 0.33
                    visible: isselected
                    height: parent.height * 0.33
                    anchors.centerIn: parent
                }

                MouseArea {
                    id: imageMA
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: {
                        if (mouse.button == Qt.RightButton) {
                            contextMenu.thumbPath = cellItem.thumbPath
                            contextMenu.popup()
                        } else {
                             combinedArtworks.setArtworkSelected(delegateIndex, !isselected)
                        }
                    }
                }
            }

            // hack until QML will allow scrolling horizontally with mouse wheel
            MouseArea {
                id: horizontalScrollMA
                anchors.fill: parent
                enabled: rosterListView.contentWidth > rosterListView.width
                propagateComposedEvents: true
                preventStealing: true
                property double epsilon: 0.000001

                onWheel: {
                    var shiftX = wheel.angleDelta.y
                    rosterListView.scrollHorizontally(shiftX)
                    wheel.accepted = Math.abs(shiftX) > epsilon
                }
            }
        }

        Item {
            id: selectNextButton
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 40
            enabled: (rosterListView.contentWidth > rosterListView.width) && (rosterListView.contentX < (rosterListView.contentWidth - rosterListView.width))

            Rectangle {
                anchors.fill: parent
                color: enabled ? (nextButtonMA.containsMouse ? uiColors.defaultControlColor : uiColors.leftSliderColor) : uiColors.panelColor
            }

            TriangleElement {
                width: 7
                height: 14
                anchors.centerIn: parent
                anchors.horizontalCenterOffset: isFlipped ? -1 : +1
                isVertical: true
                color: {
                    if (enabled) {
                        return nextButtonMA.pressed ? uiColors.inputForegroundColor : uiColors.artworkActiveColor
                    } else {
                        return uiColors.inputBackgroundColor
                    }
                }
            }

            MouseArea {
                id: nextButtonMA
                anchors.fill: parent
                hoverEnabled: true
                onClicked: {
                    rosterListView.scrollHorizontally(rosterListView.height)
                }
            }
        }
    }

    ClipboardHelper {
        id: clipboard
    }
}
