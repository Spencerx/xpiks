import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup()

    WhatsNewDialog {
        id: whatsNewDialog
        anchors.fill: parent
    }

    TestCase {
        name: "WhatsNew"
        when: windowShown

        function initTestCase() {
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }
    }
}
