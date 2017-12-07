import QtQuick 2.0
import QtTest 1.1
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600

    FakeColors {
        id: uiColors
    }

    QtObject {
        id: uiManager
        property real keywordHeight: 10
    }

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    QtObject {
        id: settingsModel
        property int keywordSizeScale: 1
    }

    QtObject {
        id: helpersWrapper

        signal globalBeforeDestruction()
    }

    QtObject {
        id: i18
        property string n: ""
    }

    WhatsNewDialog {
        id: whatsNewDialog
        anchors.fill: parent
    }

    TestCase {
        name: "WhatsNew"
        when: windowShown
    }
}
