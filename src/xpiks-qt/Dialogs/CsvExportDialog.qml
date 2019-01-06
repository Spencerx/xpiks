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

StaticDialogBase {
    id: csvExportComponent
    canEscapeClose: false
    anchors.fill: parent
    property variant csvExportModel: dispatcher.getCommandTarget(UICommand.SetupCSVExportForSelected)
    property variant columnsModel: csvExportModel.getColumnsModel()
    property variant propertiesModel: columnsModel.getPropertiesList()

    function closePopup() {
        csvExportComponent.destroy()
    }

    Keys.onEscapePressed: closePopup()

    MessageDialog {
        id: confirmRemoveItemDialog
        property int itemIndex
        title: i18.n + qsTr("Confirmation")
        text: i18.n + qsTr("Are you sure you want to remove this item?")
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            csvExportModel.removePlanAt(itemIndex)

            if (exportPlanModelsListView.count == 0) {
                addExportPlanButton.forceActiveFocus()
            }
        }
    }

    Menu {
        id: dotsClickMenu
        property int columnIndex: 0

        MenuItem {
            text: i18.n + qsTr("Remove")
            onTriggered: {
                columnsModel.removeColumn(dotsClickMenu.columnIndex)
            }
        }

        MenuItem {
            text: i18.n + qsTr("Add column above")
            onTriggered: {
                columnsModel.addColumnAbove(dotsClickMenu.columnIndex)
            }
        }

        MenuItem {
            text: i18.n + qsTr("Move up")
            onTriggered: {
                columnsModel.moveColumnUp(dotsClickMenu.columnIndex)
            }
        }

        MenuItem {
            text: i18.n + qsTr("Move down")
            onTriggered: {
                columnsModel.moveColumnDown(dotsClickMenu.columnIndex)
            }
        }
    }

    FileDialog {
        id: exportDirDialog
        title: "Please choose export location"
        selectExisting: true
        selectMultiple: false
        selectFolder: true
        folder: shortcuts.documents
        nameFilters: [ "All files (*)" ]

        onAccepted: {
            console.log("CsvExportDialog # You chose: " + exportDirDialog.folder)
            dispatcher.dispatch(UICommand.StartCSVExport, exportDirDialog.folder)
        }

        onRejected: {
            console.log("CsvExportDialog # File dialog canceled")
        }
    }

    MessageDialog {
        id: selectPlansMessageBox
        title: i18.n + qsTr("Warning")
        text: i18.n + qsTr("Please, select some export plans first")
    }

    MessageDialog {
        id: exportFinishedMessageBox
        title: i18.n + qsTr("Information")
        text: i18.n + qsTr("CSV export finished")
    }

    Connections {
        target: csvExportModel
        onExportFinished: {
            exportFinishedMessageBox.open()
        }
    }

    contentsWidth: 700
    contentsHeight: 520
    contents: Item {
        anchors.fill: parent

        Rectangle {
            id: leftPanel
            color: uiColors.defaultControlColor
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: footer.top
            width: 250
            enabled: !csvExportModel.isExporting

            ListView {
                id: exportPlanModelsListView
                objectName: "exportPlanModelsListView"
                model: csvExportModel
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: leftFooter.top
                anchors.topMargin: 30
                anchors.bottomMargin: 10
                clip: true
                boundsBehavior: Flickable.StopAtBounds

                displaced: Transition {
                    NumberAnimation { properties: "x,y"; duration: 230 }
                }

                addDisplaced: Transition {
                    NumberAnimation { properties: "x,y"; duration: 230 }
                }

                removeDisplaced: Transition {
                    NumberAnimation { properties: "x,y"; duration: 230 }
                }

                onCurrentIndexChanged: {
                    csvExportModel.setCurrentItem(exportPlanModelsListView.currentIndex)
                }

                delegate: Rectangle {
                    id: sourceWrapper
                    objectName: "exportPlanDelegate"
                    property variant myData: model
                    property int delegateIndex: index
                    property bool isCurrent: ListView.isCurrentItem
                    color: ListView.isCurrentItem ? uiColors.popupBackgroundColor : (exportPlanMA.containsMouse ? uiColors.panelColor :  leftPanel.color)
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 50

                    MouseArea {
                        id: exportPlanMA
                        hoverEnabled: true
                        anchors.fill: parent
                        //propagateComposedEvents: true
                        //preventStealing: false

                        onClicked: {
                            if (exportPlanModelsListView.currentIndex != sourceWrapper.delegateIndex) {
                                exportPlanModelsListView.currentIndex = sourceWrapper.delegateIndex
                                //uploadInfos.updateProperties(sourceWrapper.delegateIndex)
                            }
                        }

                        onDoubleClicked: {
                            myData.editisselected = !myData.isselected
                            itemCheckedCheckbox.checked = isselected
                        }
                    }

                    RowLayout {
                        spacing: 10
                        anchors.fill: parent

                        Item {
                            width: 10
                        }

                        StyledCheckbox {
                            id: itemCheckedCheckbox
                            objectName: "itemCheckedCheckbox"
                            isContrast: !sourceWrapper.isCurrent
                            onClicked: editisselected = checked
                            Component.onCompleted: itemCheckedCheckbox.checked = isselected
                        }

                        StyledText {
                            id: infoTitle
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            height: 31
                            text: name
                            elide: Text.ElideMiddle
                            font.bold: sourceWrapper.isCurrent
                        }

                        CloseIcon {
                            width: 14
                            height: 14
                            Layout.alignment: Qt.AlignVCenter
                            // anchors.verticalCenterOffset: 1
                            isActive: false
                            enabled: !issystem
                            visible: !issystem
                            disabledColor: uiColors.closeIconInactiveColor

                            onItemClicked: {
                                confirmRemoveItemDialog.itemIndex = sourceWrapper.delegateIndex
                                confirmRemoveItemDialog.open()
                            }
                        }

                        Item {
                            id: placeholder2
                            width: 15
                        }
                    }
                }

                Component.onCompleted: {
                    if (count > 0) {
                        titleText.forceActiveFocus()
                        titleText.cursorPosition = titleText.text.length
                    }
                }
            }

            CustomScrollbar {
                id: exportPlansScroll
                anchors.topMargin: 0
                anchors.bottomMargin: 0
                anchors.rightMargin: 0
                flickable: exportPlanModelsListView
                canShow: !rightPanelMA.containsMouse
            }

            Item {
                id: leftFooter
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                height: 50

                StyledBlackButton {
                    id: addExportPlanButton
                    objectName: "addExportPlanButton"
                    text: i18.n + qsTr("Add new", "csv export plan")
                    width: 210
                    height: 30
                    anchors.centerIn: parent
                    onClicked: {
                        csvExportModel.addNewPlan()

                        if (exportPlanModelsListView.count > 0) {
                            exportPlanModelsListView.positionViewAtIndex(exportPlanModelsListView.count - 1, ListView.Contain)
                            exportPlanModelsListView.currentIndex = exportPlanModelsListView.count - 1

                            titleText.forceActiveFocus()
                            titleText.cursorPosition = titleText.text.length
                        }
                    }
                }
            }
        }

        Item {
            id: rightHeader
            anchors.left: leftPanel.right
            anchors.right: parent.right
            anchors.top: parent.top
            height: 40

            StyledText {
                anchors.right: parent.right
                anchors.rightMargin: 20
                anchors.verticalCenter: parent.verticalCenter
                text: i18.n + getOriginalText()
                isActive: false

                function getOriginalText() {
                    return (csvExportModel.artworksCount == 1) ? qsTr("1 artwork selected") : qsTr("%1 artworks selected").arg(csvExportModel.artworksCount)
                }
            }
        }

        Item {
            id: rightPanel
            anchors.left: leftPanel.right
            anchors.right: parent.right
            anchors.top: rightHeader.top
            anchors.bottom: footer.top
            property bool isSystemPlan: exportPlanModelsListView.currentItem ? exportPlanModelsListView.currentItem.myData.issystem : false
            enabled: !csvExportModel.isExporting && (exportPlanModelsListView.count > 0)

            MouseArea {
                id: rightPanelMA
                anchors.fill: parent
                hoverEnabled: true
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.leftMargin: 5
                anchors.topMargin: 20
                anchors.bottomMargin: 20
                anchors.rightMargin: 10
                spacing: 0

                Item {
                    height: 10
                }

                StyledText {
                    text: i18.n + qsTr("Title:")
                    isActive: false
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: 33
                }

                Item {
                    height: 4
                }

                Rectangle {
                    id: titleWrapper
                    border.width: titleText.activeFocus ? 1 : 0
                    border.color: uiColors.artworkActiveColor
                    width: 370
                    color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                    height: 24
                    Layout.alignment: Qt.AlignLeft
                    Layout.leftMargin: 33
                    enabled: parent.enabled && !rightPanel.isSystemPlan

                    StyledTextInput {
                        id: titleText
                        height: parent.height
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.rightMargin: 5
                        text: exportPlanModelsListView.currentItem ? exportPlanModelsListView.currentItem.myData.name : ""
                        anchors.leftMargin: 5

                        onTextChanged: {
                            if (exportPlanModelsListView.currentItem) {
                                exportPlanModelsListView.currentItem.myData.editname = text
                            }
                        }

                        onEditingFinished: {
                            if (text.length == 0) {
                                if (exportPlanModelsListView.currentItem) {
                                    exportPlanModelsListView.currentItem.myData.editname = qsTr("Untitled")
                                }
                            }
                        }

                        Keys.onTabPressed: {
                            if (columnsListView.count > 0) {
                                columnsListView.forceActiveFocus()
                                columnsListView.currentIndex = 0
                                columnsListView.currentItem.focusEditing()
                            }
                        }

                        validator: RegExpValidator {
                            // copy paste in keys.onpressed Paste
                            regExp: /[a-zA-Z0-9 _-]*$/
                        }

                        Keys.onPressed: {
                            if (event.matches(StandardKey.Paste)) {
                                var clipboardText = clipboard.getText();
                                clipboardText = clipboardText.replace(/(\r\n|\n|\r)/gm, '');
                                // same regexp as in validator
                                var sanitizedText = clipboardText.replace(/[^a-zA-Z0-9 _-]/g, '');
                                titleText.paste(sanitizedText)
                                event.accepted = true
                            }
                        }
                    }
                }

                Item {
                    height: 30
                }

                Item {
                    Layout.fillWidth: true
                    height: childrenRect.height

                    StyledText {
                        text: i18.n + qsTr("Column name:")
                        anchors.left: parent.left
                        anchors.leftMargin: 35
                        isActive: false
                    }

                    StyledText {
                        text: i18.n + qsTr("Property:")
                        anchors.left: parent.left
                        anchors.leftMargin: 235
                        isActive: false
                    }
                }

                Item {
                    height: 5
                }

                Item {
                    id: columnsHost
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: false

                    ListView {
                        id: columnsListView
                        objectName: "columnsListView"
                        model: columnsModel
                        clip: columnsScrollbar.visible
                        anchors.fill: parent
                        boundsBehavior: Flickable.StopAtBounds
                        spacing: 10
                        focus: true

                        displaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        addDisplaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        removeDisplaced: Transition {
                            NumberAnimation { properties: "x,y"; duration: 230 }
                        }

                        delegate: FocusScope {
                            id: columnWrapper
                            objectName: "columnDelegate"
                            property int delegateIndex: index
                            property variant myData: model
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 30
                            focus: true
                            enabled: columnsListView.enabled && !rightPanel.isSystemPlan

                            function focusEditing() {
                                columnNameText.forceActiveFocus()
                            }

                            onFocusChanged: {
                                if (activeFocus) {
                                    columnsListView.positionViewAtIndex(columnWrapper.delegateIndex, ListView.Contain)
                                }
                            }

                            RowLayout {
                                anchors.fill: parent
                                spacing: 0

                                DotsButton {
                                    id: columnMenu
                                    onDotsClicked: {
                                        dotsClickMenu.columnIndex = columnWrapper.delegateIndex
                                        dotsClickMenu.popup()
                                    }
                                }

                                Item {
                                    width: 5
                                }

                                Rectangle {
                                    id: columnNameWrapper
                                    border.width: columnNameText.activeFocus ? 1 : 0
                                    border.color: uiColors.artworkActiveColor
                                    width: 180
                                    color: enabled ? uiColors.inputBackgroundColor : uiColors.inputInactiveBackground
                                    height: 24
                                    Layout.alignment: Qt.AlignVCenter

                                    StyledTextInput {
                                        id: columnNameText
                                        objectName: "columnNameText"
                                        height: parent.height
                                        anchors.left: parent.left
                                        anchors.right: parent.right
                                        anchors.rightMargin: 5
                                        text: column
                                        anchors.leftMargin: 5
                                        activeFocusOnTab: true
                                        onTextChanged: {
                                            model.editcolumn = text
                                        }

                                        onEditingFinished: {
                                            if (text.length == 0) {
                                                model.editcolumn = qsTr("Untitled")
                                            }
                                        }

                                        validator: RegExpValidator {
                                            // copy paste in keys.onpressed Paste
                                            regExp: /[a-zA-Z0-9 _-]*$/
                                        }

                                        Keys.onPressed: {
                                            if (event.matches(StandardKey.Paste)) {
                                                var clipboardText = clipboard.getText();
                                                clipboardText = clipboardText.replace(/(\r\n|\n|\r)/gm, '');
                                                // same regexp as in validator
                                                var sanitizedText = clipboardText.replace(/[^a-zA-Z0-9 _-]/g, '');
                                                columnNameText.paste(sanitizedText)
                                                event.accepted = true
                                            }
                                        }
                                    }
                                }

                                Item {
                                    width: 20
                                }

                                ComboBoxPopup {
                                    id: propertiesCombobox
                                    objectName: "propertiesCombobox"
                                    model: propertiesModel
                                    width: 170
                                    height: 24
                                    itemHeight: 28
                                    showColorSign: false
                                    maxCount: 5 //columnsScrollbar.visible ? 4 : 5
                                    //z: 100500 - columnWrapper.delegateIndex
                                    isBelow: true
                                    withRelativePosition: true
                                    //relativeParent: columnsHost
                                    dropDownWidth: 170
                                    glowEnabled: true
                                    glowTopMargin: 2
                                    globalParent: csvExportComponent

                                    onComboItemSelected: {
                                        myData.editproperty = selectedIndex
                                    }

                                    onIsOpenedChanged: {
                                        // turn on/off scrolling of main listview
                                        if (isOpened) {
                                            columnsListView.interactive = false
                                        } else {
                                            columnsListView.interactive = true
                                        }
                                    }

                                    Component.onCompleted: {
                                        selectedIndex = myData ? myData.property : 0
                                    }
                                }

                                Item {
                                    Layout.fillWidth: true
                                }
                            }

                        }

                        footer: Item {
                            anchors.left: parent.left
                            anchors.right: parent.right
                            height: 50
                            enabled: columnsListView.enabled && !rightPanel.isSystemPlan

                            GlyphButton {
                                id: addColumnButton
                                objectName: "addColumnButton"
                                text: i18.n + qsTr("Add column")
                                width: 180
                                height: 30
                                anchors.left: parent.left
                                anchors.leftMargin: 33
                                anchors.verticalCenter: parent.verticalCenter
                                opacity: hovered ? 1 : 0.3

                                glyphVisible: true
                                leftShift: -10
                                glyphMargin: 20
                                glyph: Item {
                                    id: plusItem
                                    width: 16
                                    height: 16
                                    property real size: 4

                                    Rectangle {
                                        id: verticalBar
                                        width: plusItem.size
                                        height: parent.height
                                        color: addColumnButton.textColor
                                        anchors.horizontalCenter: parent.horizontalCenter
                                        opacity: 1
                                    }

                                    Rectangle {
                                        anchors.left: parent.left
                                        anchors.right: verticalBar.left
                                        height: plusItem.size
                                        color: addColumnButton.textColor
                                        anchors.verticalCenter: parent.verticalCenter
                                    }

                                    Rectangle {
                                        anchors.right: parent.right
                                        anchors.left: verticalBar.right
                                        height: plusItem.size
                                        color: addColumnButton.textColor
                                        anchors.verticalCenter: parent.verticalCenter
                                    }
                                }

                                onClicked: {
                                    columnsModel.addColumn()
                                    columnsListView.forceActiveFocus()
                                    columnsListView.positionViewAtEnd()
                                    columnsListView.currentIndex = columnsListView.count - 1
                                    columnsListView.currentItem.focusEditing()
                                }
                            }
                        }
                    }

                    CustomScrollbar {
                        id: columnsScrollbar
                        anchors.topMargin: 0
                        anchors.bottomMargin: 0
                        anchors.rightMargin: 5
                        flickable: columnsListView
                        //canShow: leftPanelMA.containsMouse
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                color: uiColors.defaultControlColor
                opacity: 0.5
                visible: csvExportModel.isExporting || (exportPlanModelsListView.count == 0) || rightPanel.isSystemPlan
            }

            LoaderIcon {
                width: 100
                height: 100
                anchors.centerIn: parent
                running: csvExportModel.isExporting
            }
        }

        Rectangle {
            id: footer
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            height: 50
            color: uiColors.defaultDarkColor

            RowLayout {
                id: footerRow
                anchors.fill: parent
                anchors.leftMargin: 20
                anchors.rightMargin: 20
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                spacing: 20

                Item {
                    Layout.fillWidth: true
                }

                StyledButton {
                    text: i18.n + qsTr("Start Export")
                    width: 110
                    isDefault: true
                    Layout.alignment: Qt.AlignVCenter
                    enabled: !csvExportModel.isExporting && (exportPlanModelsListView.count > 0)
                    onClicked: {
                        if (csvExportModel.getSelectedPlansCount() === 0) {
                            selectPlansMessageBox.open()
                        } else {
                            exportDirDialog.open()
                        }
                    }
                }

                StyledButton {
                    text: i18.n + qsTr("Close")
                    Layout.alignment: Qt.AlignVCenter
                    width: 110
                    enabled: !csvExportModel.isExporting
                    onClicked: {
                        closePopup()
                    }
                }
            }
        }
    }
}
