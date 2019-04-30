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
    id: artworkEditComponent
    objectName: "ArtworkEditView"
    color: uiColors.defaultDarkerColor

    property variant componentParent
    property var autoCompleteBox

    property var artworkProxy: dispatcher.getCommandTarget(UICommand.SetupProxyArtworkEdit)
    property var keywordsModel: artworkProxy.getBasicModelObject()
    property bool wasLeftSideCollapsed
    property bool listViewEnabled: true
    property bool canShowChangesSaved: false
    property bool showInfo: false

    function onAutoCompleteClose() {
        autoCompleteBox = undefined
    }

    function reloadItemEditing(itemIndex) {
        console.debug("reloadItemEditing # " + itemIndex)
        if (itemIndex === artworkProxy.proxyIndex) { return }
        if ((itemIndex < 0) || (itemIndex >= rosterListView.count)) { return }

        canShowChangesSaved = false
        changesText.visible = false
        closeAutoComplete()
        flv.submitCurrentKeyword()

        dispatcher.dispatch(UICommand.SetupProxyArtworkEdit, itemIndex)
    }

    function closePopup() {
        closeAutoComplete()
        artworkProxy.resetModel()
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
        if (typeof artworkEditComponent.autoCompleteBox !== "undefined") {
            artworkEditComponent.autoCompleteBox.closePopup()
        }
    }

    function updateChangesText() {
        if (canShowChangesSaved) {
            changesText.visible = true
        }
    }

    function openDuplicatesView() {
        dispatcher.dispatch(UICommand.ReviewDuplicatesSingle, {})
    }

    function suggestKeywords() {
        var callbackObject = {
            promoteKeywords: function(keywords) {
                artworkProxy.pasteKeywords(keywords)
                updateChangesText()
            }
        }

        dispatcher.dispatch(UICommand.InitSuggestionSingle, callbackObject)
    }

    function fixSpelling() {
        dispatcher.dispatch(UICommand.ReviewSpellingSingle, {})
        updateChangesText()
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
                                keywordsText: artworkProxy.getKeywordsString(),
                                keywordsModel: artworkProxy.getBasicModelObject()
                            });
    }

    function clearKeywords() {
        filteredArtworksListModel.clearKeywords(artworkProxy.proxyIndex)
        artworkProxy.updateKeywords()
        updateChangesText()
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
                        artworkProxy.expandPreset(wordRightClickMenu.keywordIndex, presetID);
                        updateChangesText()
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
                                artworkProxy.addPreset(groupMenu.groupModel.getOriginalID(index));
                                updateChangesText()
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
                        artworkProxy.addPreset(subMenu.defaultGroupModel.getOriginalID(index));
                        updateChangesText()
                    }
                }
            }
        }
    }

    Menu {
        id: itemPreviewMenu
        property int index

        MenuItem {
            text: i18.n + qsTr("Copy to Quick Buffer")
            onTriggered: dispatcher.dispatch(UICommand.CopyArtworkToQuickBuffer, itemPreviewMenu.index)
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
                return artworkEditComponent.keywordsModel ?
                            (artworkEditComponent.keywordsModel.hasKeywordsSpellErrors ||
                             artworkEditComponent.keywordsModel.hasTitleSpellErrors ||
                             artworkEditComponent.keywordsModel.hasDescriptionSpellErrors)
                          : false
            }
            onTriggered: {
                fixSpelling()
            }
        }

        MenuItem {
            text: i18.n + qsTr("Show duplicates")
            enabled: artworkEditComponent.keywordsModel ? artworkEditComponent.keywordsModel.hasDuplicates : false
            onTriggered: openDuplicatesView()
        }

        MenuSeparator { }

        MenuItem {
            text: i18.n + qsTr("Copy")
            enabled: artworkProxy.keywordsCount > 0
            objectName: "copyMenuItem"
            onTriggered: {
                clipboard.setText(artworkProxy.getKeywordsString())
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
            enabled: artworkProxy.keywordsCount > 0
            onTriggered: {
                clearKeywordsDialog.open()
            }
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupProxyArtworkEdit]
        onDispatched: {
            console.log("# [EditArtwork] dispatched")
            artworkEditComponent.keywordsModel = artworkProxy.getBasicModelObject()

            if (listViewEnabled) {
                rosterListView.currentIndex = value
                rosterListView.positionViewAtIndex(value, ListView.Contain)
            }

            titleTextInput.forceActiveFocus()
            titleTextInput.cursorPosition = titleTextInput.text.length

            uiManager.initTitleHighlighting(artworkProxy.getBasicModelObject(), titleTextInput.textDocument)
            uiManager.initDescriptionHighlighting(artworkProxy.getBasicModelObject(), descriptionTextInput.textDocument)

            savedTimer.start()
        }
    }

    Timer {
        id: savedTimer
        interval: 1000
        running: true
        repeat: false
        onTriggered: {
            canShowChangesSaved = true
        }
    }

    Keys.onPressed: {
        var withMeta = (event.modifiers & Qt.ControlModifier) ||
                (event.modifiers & Qt.MetaModifier);
        var withAlt = (event.modifiers & Qt.AltModifier);

        if (((event.key === Qt.Key_Left) && (withMeta && withAlt)) ||
                event.matches(StandardKey.MoveToPreviousPage)) {
            reloadItemEditing(rosterListView.currentIndex - 1)
            event.accepted = true;
        }

        if (((event.key === Qt.Key_Right) && (withMeta && withAlt)) ||
                event.matches(StandardKey.MoveToNextPage)) {
            reloadItemEditing(rosterListView.currentIndex + 1)
            event.accepted = true;
        }
    }

    Keys.onEscapePressed: closePopup()

    Component.onCompleted: {
        if (showInfo) {
            editTabView.setCurrentIndex(1)
        }
    }

    Stack.onStatusChanged: {
        console.log("ArtworkEditView StackView status changed: " + Stack.status)
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
            artworkProxy.resetModel()
        }
    }

    Connections {
        target: artworkProxy
        onItemBecomeUnavailable: {
            closePopup()
        }
    }

    MessageDialog {
        id: clearKeywordsDialog
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Clear all keywords?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: clearKeywords()
    }

    // Keys.onEscapePressed: closePopup()

    Connections {
        target: acSource

        onCompletionsAvailable: {
            acSource.initializeCompletions()

            if (typeof artworkEditComponent.autoCompleteBox !== "undefined") {
                // update completion
                return
            }

            var directParent = artworkEditComponent;
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

                instance.boxDestruction.connect(artworkEditComponent.onAutoCompleteClose)
                instance.itemSelected.connect(flv.acceptCompletion)
                artworkEditComponent.autoCompleteBox = instance

                instance.openPopup()
            }
        }
    }

    Rectangle {
        id: leftSpacer
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: 2
        color: appHost.leftSideCollapsed ? bottomPane.color : artworkEditComponent.color
    }

    SplitView {
        orientation: Qt.Horizontal
        anchors.left: leftSpacer.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: bottomPane.top

        handleDelegate: Rectangle {
            width: 0
            color: uiColors.selectedArtworkBackground
        }

        onResizingChanged: {
            uiManager.artworkEditRightPaneWidth = rightPane.width
        }

        Item {
            id: boundsRect
            Layout.fillWidth: true
            anchors.top: parent.top
            anchors.bottom: parent.bottom

            Rectangle {
                id: topHeader
                anchors.left: parent.left
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
                    height: childrenRect.height
                    spacing: 0

                    BackGlyphButton {
                        objectName: "backButton"
                        text: i18.n + qsTr("Back")
                        onClicked: {
                            flv.onBeforeClose()
                            closePopup()
                        }
                    }

                    Item {
                        width: 10
                    }

                    StyledText {
                        id: changesText
                        text: i18.n + qsTr("All changes are saved.")
                        isActive: false
                        visible: false
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    StyledText {
                        text: artworkProxy.basename
                        isActive: false
                    }
                }
            }

            Item {
                id: imageWrapper
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: topHeader.bottom
                anchors.bottom: parent.bottom
                property int imageMargin: 10

                LoaderIcon {
                    width: 100
                    height: 100
                    anchors.centerIn: parent
                    running: previewImage.status == Image.Loading
                }

                StyledScrollView {
                    id: scrollview
                    anchors.fill: parent
                    anchors.leftMargin: imageWrapper.imageMargin
                    anchors.topMargin: imageWrapper.imageMargin
                    flickableItem.interactive: previewImage.isFullSize

                    Image {
                        id: previewImage
                        source: "image://global/" + artworkProxy.thumbPath
                        cache: false
                        property bool isFullSize: false
                        width: {
                            var fullWidth = sourceSize.width
                            var fitWidth = (imageWrapper.width - 2*imageWrapper.imageMargin)

                            if (isFullSize) {
                                return Math.max(fullWidth, fitWidth)
                            } else {
                                return fitWidth
                            }
                        }
                        height: {
                            var fullHeight = sourceSize.height
                            var fitHeight = (imageWrapper.height - 2*imageWrapper.imageMargin)

                            if (isFullSize) {
                                return Math.max(fullHeight, fitHeight)
                            } else {
                                return fitHeight
                            }
                        }
                        fillMode: Image.PreserveAspectFit
                        anchors.centerIn: parent
                        asynchronous: true
                    }
                }

                Rectangle {
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
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
                            previewImage.isFullSize = !previewImage.isFullSize
                            zoomIcon.isPlus = !zoomIcon.isPlus
                        }
                    }
                }
            }
        }

        Item {
            id: rightPane
            Layout.maximumWidth: 550
            Layout.minimumWidth: 250
            //width: 300
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            property bool isWideEnough: width > 350

            Component.onCompleted: {
                rightPane.width = uiManager.artworkEditRightPaneWidth
            }

            Row {
                id: tabsHeader
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: 55
                spacing: 0

                Repeater {
                    model: [i18.n + qsTr("Edit"), i18.n + qsTr("Info")]
                    delegate: CustomTab {
                        width: rightPane.width/2
                        property int delegateIndex: index
                        tabIndex: delegateIndex
                        isSelected: tabIndex === editTabView.currentIndex
                        color: isSelected ? uiColors.selectedArtworkBackground : uiColors.inputInactiveBackground
                        hovered: tabMA.containsMouse

                        StyledText {
                            color: parent.isSelected ? uiColors.artworkActiveColor : (parent.hovered ? uiColors.labelActiveForeground : uiColors.labelInactiveForeground)
                            text: modelData
                            anchors.centerIn: parent
                        }

                        MouseArea {
                            id: tabMA
                            anchors.fill: parent
                            hoverEnabled: true
                            onClicked: editTabView.setCurrentIndex(parent.tabIndex)
                        }
                    }
                }
            }

            Item {
                id: editTabView
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: tabsHeader.bottom
                anchors.bottom: parent.bottom
                property int currentIndex: 0

                function setCurrentIndex(index) {
                    if (index === 0) {
                        infoTab.visible = false
                        editTab.visible = true
                    } else if (index === 1) {
                        infoTab.visible = true
                        editTab.visible = false
                    }

                    editTabView.currentIndex = index
                }

                Rectangle {
                    id: editTab
                    color: uiColors.selectedArtworkBackground
                    anchors.fill: parent
                    enabled: artworkProxy.isValid

                    ColumnLayout {
                        id: fields
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 20
                        anchors.topMargin: 5
                        anchors.bottomMargin: 10
                        spacing: 0

                        Item {
                            height: childrenRect.height
                            Layout.fillWidth: true

                            StyledText {
                                id: titleText
                                anchors.left: parent.left
                                text: i18.n + qsTr("Title:")
                            }

                            StyledText {
                                anchors.left: titleText.right
                                anchors.leftMargin: 3
                                text: "*"
                                color: uiColors.destructiveColor
                                visible: artworkProxy.hasTitleSpellErrors
                            }

                            StyledText {
                                anchors.right: parent.right
                                text: titleTextInput.length
                            }
                        }

                        Item {
                            height: 5
                        }

                        Rectangle {
                            id: anotherRect
                            Layout.fillWidth: true
                            height: 25
                            color: uiColors.inputBackgroundColor
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
                                    text: artworkProxy.title
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
                                        artworkProxy.title = text
                                        updateChangesText()
                                    }

                                    onActionRightClicked: {
                                        var showAddToDict = artworkProxy.hasTitleWordSpellError(rightClickedWord)
                                        wordRightClickMenu.showAddToDict = showAddToDict
                                        wordRightClickMenu.word = rightClickedWord
                                        wordRightClickMenu.showExpandPreset = false
                                        wordRightClickMenu.popupIfNeeded()
                                    }

                                    Keys.onBacktabPressed: {
                                        event.accepted = true
                                    }

                                    Keys.onTabPressed: {
                                        descriptionTextInput.forceActiveFocus()
                                        descriptionTextInput.cursorPosition = descriptionTextInput.text.length
                                    }

                                    Component.onCompleted: {
                                        uiManager.initTitleHighlighting(
                                                    artworkProxy.getBasicModelObject(),
                                                    titleTextInput.textDocument)
                                    }

                                    onCursorRectangleChanged: titleFlick.ensureVisible(cursorRectangle)

                                    Keys.onPressed: {
                                        if (event.matches(StandardKey.Paste)) {
                                            var clipboardText = clipboard.getText();
                                            if (Common.safeInsert(titleTextInput, clipboardText)) {
                                                event.accepted = true
                                            }
                                        } else if ((event.key === Qt.Key_Return) || (event.key === Qt.Key_Enter)) {
                                            event.accepted = true
                                        } else {
                                            event.accepted = false
                                        }
                                    }
                                }
                            }
                        }

                        Item {
                            height: 20
                        }

                        Item {
                            height: childrenRect.height
                            Layout.fillWidth: true

                            StyledText {
                                id: descriptionText
                                anchors.left: parent.left
                                text: i18.n + qsTr("Description:")
                            }

                            StyledText {
                                anchors.left: descriptionText.right
                                anchors.leftMargin: 3
                                text: "*"
                                color: uiColors.destructiveColor
                                visible: artworkProxy.hasDescriptionSpellErrors
                            }

                            StyledText {
                                anchors.right: parent.right
                                text: descriptionTextInput.length
                            }
                        }

                        Item {
                            height: 5
                        }

                        Rectangle {
                            id: rect
                            Layout.fillWidth: true
                            height: 70
                            color: uiColors.inputBackgroundColor
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
                                    objectName: "descriptionTextInput"
                                    width: descriptionFlick.width
                                    height: paintedHeight > descriptionFlick.height ? paintedHeight : descriptionFlick.height
                                    text: artworkProxy.description
                                    focus: true
                                    userDictEnabled: true
                                    property string previousText: text
                                    property int maximumLength: 280
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
                                        artworkProxy.description = text
                                        updateChangesText()
                                    }

                                    onActionRightClicked: {
                                        var showAddToDict = artworkProxy.hasDescriptionWordSpellError(rightClickedWord)
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
                                                    artworkProxy.getBasicModelObject(),
                                                    descriptionTextInput.textDocument)
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

                        Item {
                            height: 20
                        }

                        Item {
                            height: childrenRect.height
                            Layout.fillWidth: true

                            StyledText {
                                anchors.left: parent.left
                                id: keywordsLabel
                                text: i18.n + qsTr("Keywords:")
                            }

                            StyledText {
                                anchors.left: keywordsLabel.right
                                anchors.leftMargin: 3
                                anchors.top: keywordsLabel.top
                                text: "*"
                                color: uiColors.destructiveColor
                                visible: artworkProxy.hasKeywordsSpellErrors
                            }

                            StyledText {
                                anchors.right: parent.right
                                text: artworkProxy.keywordsCount
                            }
                        }

                        Item {
                            height: 4
                        }

                        Rectangle {
                            id: keywordsWrapper
                            border.color: uiColors.artworkActiveColor
                            border.width: flv.isFocused ? 1 : 0
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            color: uiColors.inputBackgroundColor
                            state: ""

                            function removeKeyword(index) {
                                artworkProxy.removeKeywordAt(index)
                                updateChangesText()
                            }

                            function removeLastKeyword() {
                                artworkProxy.removeLastKeyword()
                                updateChangesText()
                            }

                            function appendKeyword(keyword) {
                                if (artworkProxy.appendKeyword(keyword)) {
                                    updateChangesText()
                                } else {
                                    keywordsWrapper.state = "blinked"
                                    blinkTimer.start()
                                }
                            }

                            function pasteKeywords(keywords) {
                                artworkProxy.pasteKeywords(keywords)
                                updateChangesText()
                            }

                            function expandLastKeywordAsPreset() {
                                artworkProxy.expandLastKeywordAsPreset();
                                updateChangesText()
                            }

                            EditableTags {
                                id: flv
                                objectName: "editableTags"
                                anchors.fill: parent
                                model: artworkEditComponent.keywordsModel
                                property int keywordHeight: uiManager.keywordHeight
                                scrollStep: keywordHeight
                                populateAnimationEnabled: false
                                property int droppedIndex: -1
                                onDroppedIndexChanged: dropTimer.start()

                                function acceptCompletion(completionID) {
                                    if (acSource.isPreset(completionID)) {
                                        dispatcher.dispatch(UICommand.AcceptPresetCompletionForSingle, completionID)
                                        flv.editControl.acceptCompletion('')
                                    } else {
                                        var completion = acSource.getCompletion(completionID)
                                        flv.editControl.acceptCompletion(completion)
                                    }
                                }

                                delegate: DraggableKeywordWrapper {
                                    id: kw
                                    isHighlighted: true
                                    keywordText: keyword
                                    hasSpellCheckError: !iscorrect
                                    hasDuplicate: hasduplicate
                                    delegateIndex: index
                                    itemHeight: flv.keywordHeight
                                    onRemoveClicked: keywordsWrapper.removeKeyword(delegateIndex)
                                    dragDropAllowed: switcher.keywordsDragDropEnabled
                                    dragParent: flv
                                    wasDropped: flv.droppedIndex == delegateIndex
                                    onActionDoubleClicked: {
                                        // https://github.com/ribtoks/xpiks/issues/626
                                        if (kw.isDropTarget) { return; }

                                        var callbackObject = {
                                            onSuccess: function(replacement) {
                                                artworkProxy.editKeyword(kw.delegateIndex, replacement)
                                                updateChangesText()
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
                                                                keywordsModel: artworkProxy.getBasicModelObject()
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

                                        if (artworkProxy.moveKeyword(from, to)) {
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
                                    descriptionTextInput.forceActiveFocus()
                                    descriptionTextInput.cursorPosition = descriptionTextInput.text.length
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

                        Item {
                            height: 10
                        }

                        Flow {
                            Layout.fillWidth: true
                            spacing: 5

                            StyledLink {
                                id: fixSpellingLink
                                objectName: "fixSpellingLink"
                                text: i18.n + qsTr("Fix spelling")
                                property bool canBeShown: {
                                    return artworkEditComponent.keywordsModel ?
                                                (artworkEditComponent.keywordsModel.hasKeywordsSpellErrors ||
                                                 artworkEditComponent.keywordsModel.hasTitleSpellErrors ||
                                                 artworkEditComponent.keywordsModel.hasDescriptionSpellErrors)
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
                                property bool canBeShown: artworkEditComponent.keywordsModel ? artworkEditComponent.keywordsModel.hasDuplicates : false
                                enabled: canBeShown
                                visible: canBeShown
                                onClicked: { openDuplicatesView() }
                            }

                            StyledText {
                                visible: removeDuplicatesText.canBeShown
                                enabled: removeDuplicatesText.canBeShown
                                text: "|"
                                verticalAlignment: Text.AlignVCenter
                            }

                            StyledLink {
                                id: suggestLink
                                property bool canBeShown: (artworkProxy.keywordsCount < warningsModel.minKeywordsCount) || rightPane.isWideEnough
                                visible: canBeShown
                                enabled: canBeShown
                                text: i18.n + qsTr("Suggest keywords")
                                onClicked: { suggestKeywords() }
                            }

                            StyledText {
                                visible: suggestLink.canBeShown
                                enabled: suggestLink.canBeShown
                                text: "|"
                                verticalAlignment: Text.AlignVCenter
                            }

                            StyledLink {
                                id: copyLink
                                objectName: "copyLink"
                                property bool canBeShown: (artworkProxy.keywordsCount > 0)
                                text: i18.n + qsTr("Copy")
                                enabled: canBeShown
                                visible: canBeShown
                                onClicked: clipboard.setText(artworkProxy.getKeywordsString())
                            }

                            StyledText {
                                enabled: copyLink.canBeShown
                                visible: copyLink.canBeShown
                                text: "|"
                                verticalAlignment: Text.AlignVCenter
                            }

                            StyledLink {
                                id: clearLink
                                property bool canBeShown: (artworkProxy.keywordsCount > 0)
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
                                objectName: "moreLinkHost"

                                StyledText {
                                    id: moreLink
                                    color: enabled ? (moreMA.pressed ? uiColors.linkClickedColor :
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
                        }
                    }
                }

                Rectangle {
                    id: infoTab
                    enabled: artworkProxy.isValid
                    visible: false
                    color: uiColors.selectedArtworkBackground
                    anchors.fill: parent

                    ListView {
                        model: artworkProxy.getPropertiesMap()
                        anchors.fill: parent
                        anchors.margins: 20
                        spacing: 10

                        delegate: Item {
                            id: rowWrapper
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: childrenRect.height

                            Item {
                                id: keyItem
                                anchors.left: parent.left
                                width: 70
                                height: childrenRect.height

                                StyledText {
                                    text: i18.n + key + ":"
                                    isActive: false
                                    anchors.right: parent.right
                                }
                            }

                            Item {
                                id: valueItem
                                anchors.left: keyItem.right
                                anchors.right: parent.right
                                anchors.leftMargin: 10
                                height: childrenRect.height

                                StyledText {
                                    wrapMode: TextEdit.Wrap
                                    text: value
                                    width: parent.width
                                }
                            }
                        }
                    }
                }
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
            objectName: "selectPrevButton"
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 40
            enabled: rosterListView.currentIndex > 0

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
                    reloadItemEditing(rosterListView.currentIndex - 1)
                }
            }
        }

        ListView {
            id: rosterListView
            objectName: "rosterListView"
            enabled: listViewEnabled
            boundsBehavior: Flickable.StopAtBounds
            orientation: ListView.Horizontal
            anchors.left: selectPrevButton.right
            anchors.right: selectNextButton.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            model: listViewEnabled ? filteredArtworksListModel : undefined
            highlightFollowsCurrentItem: false
            highlightMoveDuration: 0
            flickableDirection: Flickable.HorizontalFlick
            interactive: false
            focus: true
            clip: true
            spacing: 0

            Component.onCompleted: {
                if (listViewEnabled) {
                    rosterListView.currentIndex = artworkProxy.proxyIndex
                    rosterListView.positionViewAtIndex(artworkProxy.proxyIndex, ListView.Contain)
                    // TODO: fix bug with ListView.Center
                }
            }

            delegate: Rectangle {
                id: cellItem
                objectName: "rosterDelegateItem"
                property int delegateIndex: index
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: bottomPane.height
                color: ListView.isCurrentItem ? uiColors.panelSelectedColor : uiColors.panelColor

                Rectangle {
                    anchors.fill: parent
                    visible: imageMA.containsMouse
                    color: Qt.lighter(uiColors.panelColor, 1.7)
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

                MouseArea {
                    id: imageMA
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: {
                        if (mouse.button == Qt.RightButton) {
                            itemPreviewMenu.index = cellItem.delegateIndex
                            itemPreviewMenu.popup()
                        } else {
                            reloadItemEditing(cellItem.delegateIndex)
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
                    var flickable = rosterListView

                    if (shiftX > epsilon) { // up/right
                        var maxScrollPos = flickable.contentWidth - flickable.width
                        flickable.contentX = Math.min(maxScrollPos, flickable.contentX + shiftX)
                        wheel.accepted = true
                    } else if (shiftX < -epsilon) { // bottom/left
                        flickable.contentX = Math.max(0, flickable.contentX + shiftX)
                        wheel.accepted = true
                    }
                }
            }
        }

        Item {
            id: selectNextButton
            objectName: "selectNextButton"
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 40
            enabled: rosterListView.currentIndex < (rosterListView.count - 1)

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
                    reloadItemEditing(rosterListView.currentIndex + 1)
                }
            }
        }
    }

    ClipboardHelper {
        id: clipboard
    }
}
