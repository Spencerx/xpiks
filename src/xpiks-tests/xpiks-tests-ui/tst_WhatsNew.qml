import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600
    property string testsHostMention: TestsHost.scoreme

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    WhatsNewDialog {
        id: whatsNewDialog
        anchors.fill: parent
    }

    TestCase {
        name: "WhatsNew"
        when: windowShown

        function initTestCase() {
            TestsHost.setup()
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }
    }
}
