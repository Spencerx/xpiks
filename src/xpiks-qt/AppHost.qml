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
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.1
import QtQuick.Layouts 1.3
import QtQuick.Window 2.0
import xpiks 1.0
import "Constants" 1.0
import "Components"
import "StyledControls"
import "Dialogs"
import "StackViews"
import "Common.js" as Common
import "Constants/UIConfig.js" as UIConfig

Rectangle {
    id: appHost
    color: uiColors.defaultDarkColor
    property var componentParent
    property alias areActionsAllowed: mainStackView.areActionsAllowed
    property bool leftSideCollapsed: false
    property bool listLayout: true

    function collapseLeftPane() {
        leftDockingGroup.state = "collapsed"
        appHost.leftSideCollapsed = true
    }

    function expandLeftPane() {
        leftDockingGroup.state = ""
        appHost.leftSideCollapsed = false
    }

    function toggleLeftPane() {
        leftDockingGroup.state = (leftDockingGroup.state == "collapsed") ? "" : "collapsed"
        appHost.leftSideCollapsed = !leftSideCollapsed
    }

    function startOneItemEditing(showInfoFirst) {
        var wasCollapsed = appHost.leftSideCollapsed
        appHost.collapseLeftPane()
        var component = Qt.createComponent("StackViews/ArtworkEditView.qml")
        var instance = component.createObject(appHost,
                                              {
                                                  componentParent: appHost.componentParent,
                                                  wasLeftSideCollapsed: wasCollapsed,
                                                  showInfo: showInfoFirst
                                              })
        mainStackView.push({
                               item: instance,
                               destroyOnPop: true
                           })
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.ReviewDuplicatesSingle,
            UICommand.ReviewDuplicatesCombined,
            UICommand.ReviewDuplicatesInSelected,
            UICommand.ReviewDuplicatesArtwork]
        onDispatched: {
            console.log("appHost # onDuplicatesReview")
            var wasCollapsed = appHost.leftSideCollapsed
            if (value) {
                appHost.collapseLeftPane()
            }

            var component = Qt.createComponent("StackViews/DuplicatesReView.qml")
            var instance = component.createObject(appHost,
                                                  {
                                                      componentParent: appHost.componentParent,
                                                      wasLeftSideCollapsed: wasCollapsed
                                                  })

            mainStackView.push({
                                   item: instance,
                                   destroyOnPop: true
                               })
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupProxyArtworkEdit, UICommand.ReviewArtworkInfo]
        onDispatched: {
            if (mainStackView.currentItem.objectName !== "ArtworkEditView") {
                startOneItemEditing((commandID === UICommand.ReviewArtworkInfo))
            }
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.CheckWarnings]
        onDispatched: {
            var wasCollapsed = appHost.leftSideCollapsed
            appHost.collapseLeftPane()

            var component = Qt.createComponent("StackViews/WarningsView.qml")
            var instance = component.createObject(appHost,
                                                  {
                                                      componentParent: appHost.componentParent,
                                                      wasLeftSideCollapsed: wasCollapsed
                                                  })
            mainStackView.push({
                                   item: instance,
                                   destroyOnPop: true
                               })
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupEditSelectedArtworks]
        onDispatched: {
            var component = Qt.createComponent("StackViews/CombinedEditView.qml")
            var instance = component.createObject(appHost,
                                                  {
                                                      componentParent: appHost.componentParent
                                                  })
            mainStackView.push({
                                   item: instance,
                                   destroyOnPop: true
                               })
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.CopyArtworkToQuickBuffer, UICommand.CopyCombinedToQuickBuffer]
        onDispatched: {
            uiManager.activateQuickBufferTab()
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.InitSuggestionArtwork,
            UICommand.InitSuggestionCombined,
            UICommand.InitSuggestionSingle]
        onDispatched: {
            var callback = value
            if (commandID == UICommand.InitSuggestionArtwork) {
                callback = value.callbackObject
            }

            Common.launchDialog("Dialogs/KeywordsSuggestion.qml",
                                appHost.componentParent,
                                {callbackObject: callback});
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.ReviewSpellingArtwork,
            UICommand.ReviewSpellingCombined,
            UICommand.ReviewSpellingInSelected,
            UICommand.ReviewSpellingSingle]
        onDispatched: {
            Common.launchDialog("Dialogs/SpellCheckSuggestionsDialog.qml",
                                appHost.componentParent,
                                {})
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupDeleteKeywordsInSelected]
        onDispatched: {
            Common.launchDialog("Dialogs/DeleteKeywordsDialog.qml",
                                appHost.componentParent,
                                { componentParent: appHost.componentParent })
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupCSVExportForSelected]
        onDispatched: {
            Common.launchDialog("Dialogs/CsvExportDialog.qml", appHost.componentParent, {})
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupCreatingArchives]
        onDispatched: {
            Common.launchDialog("Dialogs/ZipArtworksDialog.qml", appHost.componentParent, {})
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupUpload]
        onDispatched: {
            Common.launchDialog("Dialogs/UploadArtworks.qml",
                                appHost.componentParent,
                                {
                                    componentParent: appHost.componentParent,
                                    skipUploadItems: value
                                })
        }
    }

    UICommandListener {
        commandDispatcher: dispatcher
        commandIDs: [UICommand.SetupFindInArtworks]
        onDispatched: {
            Common.launchDialog("Dialogs/FindAndReplace.qml",
                                appHost.componentParent,
                                { componentParent: appHost.componentParent })
        }
    }

    Action {
        id: editAction
        shortcut: "Ctrl+E"
        enabled: (artworksRepository.artworksSourcesCount > 0) && (applicationWindow.openedDialogsCount == 0)
        onTriggered: {
            if (filteredArtworksListModel.selectedArtworksCount === 0) {
                mustSelectDialog.open()
            } else {
                var launched = false
                var index = filteredArtworksListModel.findSelectedItemIndex()

                if (index !== -1) {
                    dispatcher.dispatch(UICommand.SetupProxyArtworkEdit, index)
                    launched = true
                }

                if (!launched) {
                    // also as fallback in case of errors in findSelectedIndex
                    dispatcher.dispatch(UICommand.SetupEditSelectedArtworks, {})
                }
            }
        }
    }

    DropArea {
        enabled: appHost.areActionsAllowed
        anchors.fill: parent
        onDropped: {
            if (drop.hasUrls) {
                var filesCount = xpiksApp.dropItems(drop.urls)
                console.debug(filesCount + ' files added via drag&drop')
            }
        }
    }

    Item {
        id: leftDockingGroup
        width: 250
        anchors.left: parent.left
        anchors.leftMargin: leftCollapser.width
        anchors.top: parent.top
        anchors.bottom: parent.bottom

        states: [
            State {
                name: "collapsed"

                PropertyChanges {
                    target: leftDockingGroup
                    anchors.leftMargin: -leftDockingGroup.width + leftCollapser.width
                }
            }
        ]

        Behavior on anchors.leftMargin {
            NumberAnimation {
                duration: 100
                easing.type: Easing.InQuad
            }
        }

        Row {
            id: tabsHolder
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            property int currentIndex: 0
            property bool moreTabsAvailable: tabsModel.tabsCount > 3
            property real expanderWidth: tabsHolder.moreTabsAvailable ? plusTab.width : 0
            height: 55
            spacing: 0

            Repeater {
                id: tabsRepeater
                model: activeTabs

                delegate: CustomTab {
                    id: customTab
                    tabIndex: index
                    width: (tabsHolder.width - tabsHolder.expanderWidth) / tabsRepeater.count
                    isSelected: tabsHolder.currentIndex == tabIndex
                    hovered: (!isSelected) && tabMA.containsMouse

                    function activateThisTab() {
                        var tabIndex = customTab.tabIndex;
                        tabsHolder.currentIndex = tabIndex
                    }

                    Loader {
                        property bool isHighlighted: customTab.isSelected || customTab.hovered
                        property color parentBackground: customTab.color
                        anchors.centerIn: parent
                        source: tabicon
                    }

                    Connections {
                        target: activeTabs
                        onTabActivateRequested: {
                            console.log("On tab opened " + originalTabIndex)
                            if (activeTabs.getIndex(customTab.tabIndex) === originalTabIndex) {
                                customTab.activateThisTab()
                            }
                        }
                    }

                    StyledText {
                        enabled: debugTabs
                        visible: debugTabs
                        anchors.top: parent.top
                        anchors.right: parent.right
                        text: cachetag
                        isActive: false
                    }

                    MouseArea {
                        id: tabMA
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            customTab.activateThisTab()
                            activeTabs.openTab(tabIndex)
                        }
                    }
                }
            }

            CustomTab {
                id: plusTab
                enabled: false
                visible: false
                tabIndex: tabsRepeater.count
                isSelected: tabsHolder.currentIndex == tabIndex
                hovered: (!isSelected) && plusMA.containsMouse
                width: 20
                property bool isHighlighted: isSelected || hovered

                Connections {
                    target: tabsModel
                    onTabsCountChanged: {
                        if (tabsModel.tabsCount <= 3) {
                            if (plusTab.isSelected) {
                                activeTabs.reactivateMostRecentTab()
                            }
                        }

                        plusTab.enabled = tabsModel.tabsCount > 3
                        plusTab.visible = tabsModel.tabsCount > 3
                    }
                }

                TriangleElement {
                    anchors.centerIn: parent
                    anchors.verticalCenterOffset: isFlipped ? height*0.3 : 0
                    color: (enabled && (plusMA.containsMouse || plusTab.isHighlighted)) ? uiColors.labelActiveForeground : uiColors.labelInactiveForeground
                    isFlipped: !plusTab.isSelected
                    width: parent.width * 0.6
                    height: width * 0.5
                }

                MouseArea {
                    id: plusMA
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        var tabIndex = tabsRepeater.count;
                        tabsHolder.currentIndex = tabIndex
                    }
                }
            }
        }

        Rectangle {
            color: uiColors.defaultControlColor
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: tabsHolder.bottom
            anchors.bottom: parent.bottom

            StackLayout {
                id: mainTabView
                anchors.fill: parent
                currentIndex: tabsHolder.currentIndex

                Repeater {
                    model: activeTabs

                    delegate: Loader {
                        id: tabLoader
                        source: tabcomponent
                        asynchronous: true
                        property int myIndex: index
                        property var tabModel: uiManager.retrieveTabsModel(tabid)
                        property int selectedArtworksCount: filteredArtworksListModel.selectedArtworksCount
                        property bool areActionsAllowed: appHost.areActionsAllowed

                        Connections {
                            target: mainTabView
                            onCurrentIndexChanged: {
                                if (tabLoader.myIndex == mainTabView.currentIndex) {
                                    if (tabLoader.status == Loader.Ready) {
                                        if (typeof tabLoader.item.initializeTab !== "undefined") {
                                            tabLoader.item.initializeTab()
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    Flow {
                        anchors.fill: parent
                        anchors.leftMargin: 5
                        anchors.topMargin: 5
                        spacing: 5

                        Repeater {
                            id: inactiveTabsRepeater
                            model: inactiveTabs

                            delegate: CustomTab {
                                anchors.top: undefined
                                anchors.bottom: undefined
                                id: customInactiveTab
                                tabIndex: index
                                width: (tabsHolder.width - plusTab.width) / tabsRepeater.count
                                height: 45
                                isSelected: false
                                hovered: (!isSelected) && inactiveTabMA.containsMouse

                                Loader {
                                    property bool isHighlighted: customInactiveTab.isSelected || customInactiveTab.hovered
                                    property color parentBackground: customInactiveTab.color
                                    anchors.centerIn: parent
                                    source: tabicon
                                }

                                StyledText {
                                    enabled: debugTabs
                                    visible: debugTabs
                                    anchors.top: parent.top
                                    anchors.right: parent.right
                                    //text: cachetag
                                    isActive: false
                                }

                                MouseArea {
                                    id: inactiveTabMA
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onClicked: {
                                        inactiveTabs.openTab(tabIndex)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // hack for visual order of components (slider will be created after left panel)
    // in order not to deal with Z animation/settings
    Rectangle {
        id: leftCollapser
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        color: uiColors.leftSliderColor
        width: 20

        TriangleElement {
            width: leftCollapseMA.pressed ? 6 : 7
            height: leftCollapseMA.pressed ? 12 : 14
            isVertical: true
            isFlipped: !appHost.leftSideCollapsed
            anchors.centerIn: parent
            anchors.horizontalCenterOffset: isFlipped ? -1 : 0
            color: {
                if (leftCollapseMA.pressed) {
                    return uiColors.whiteColor
                } else {
                    return leftCollapseMA.containsMouse ? uiColors.labelInactiveForeground : uiColors.inputBackgroundColor
                }
            }
        }

        MouseArea {
            id: leftCollapseMA
            hoverEnabled: true
            anchors.fill: parent
            onClicked: toggleLeftPane()
        }
    }

    StackView {
        id: mainStackView
        objectName: "mainStackView"
        anchors.left: leftDockingGroup.right
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        focus: true
        property bool areActionsAllowed: depth <= 1

        initialItem: MainGrid {
            componentParent: appHost.componentParent
        }

        Component.onCompleted: {
            console.log("StackView initialized")
        }

        delegate: StackViewDelegate {
            function transitionFinished(properties)
            {
                properties.exitItem.opacity = 1
            }

            pushTransition: StackViewTransition {
                PropertyAnimation {
                    target: enterItem
                    property: "opacity"
                    from: 0
                    to: 1
                    duration: 100
                }

                PropertyAnimation {
                    target: exitItem
                    property: "opacity"
                    from: 1
                    to: 0
                    duration: 100
                }
            }
        }
    }
}
