import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup()

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: ZipArtworksDialog {
            anchors.fill: parent
        }
    }

    TestCase {
        name: "ZipArchiver"
        when: windowShown
        property var zipDialog: loader.item

        function initTestCase() {
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }
    }
}
