/*
 * This file is a part of Xpiks - cross platform application for
 * keywording and uploading images for microstocks
 * Copyright (C) 2014-2018 Taras Kushnir <kushnirTV@gmail.com>
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

import QtQuick 2.0

Item {
    id: root
    property bool canMinimize: true
    property bool canEscapeClose: true

    Component.onCompleted: focus = true

    Keys.onEscapePressed: {
        if (root.canEscapeClose) {
            closePopup()
        }
    }

    function closePopup() {
        root.destroy()
    }

    PropertyAnimation { target: root; property: "opacity";
        duration: 400; from: 0; to: 1;
        easing.type: Easing.InOutQuad ; running: true }

    // This rectange is the a overlay to partially show the parent through it
    // and clicking outside of the 'dialog' popup will do 'nothing'
    Rectangle {
        anchors.fill: parent
        id: overlay
        color: "#000000"
        opacity: 0.6
        // add a mouse area so that clicks outside
        // the dialog window will not do anything
        MouseArea {
            anchors.fill: parent
        }
    }

    states: [
        State {
            name: "minimized"
            PropertyChanges {
                target: root
                opacity: 0.2
            }
        }
    ]

    Keys.onPressed: {
        if (event.isAutoRepeat) { return }
        if (root.canMinimize &&
                (event.key === Qt.Key_M) &&
                (event.modifiers === Qt.ControlModifier)) {
            keywordsSuggestionComponent.state = "minimized"
            event.accepted = true
        }
    }

    Keys.onReleased: {
        if (event.isAutoRepeat) { return }
        if (root.canMinimize &&
                (event.key === Qt.Key_M) &&
                (event.modifiers === Qt.ControlModifier)) {
            keywordsSuggestionComponent.state = ""
            event.accepted = true
        }
    }
}
