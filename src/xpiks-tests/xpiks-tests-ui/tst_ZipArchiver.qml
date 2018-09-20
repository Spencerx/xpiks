import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup()

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
        when: windowShown && (loader.status == Loader.Ready)
        property var zipDialog: loader.item

        function initTestCase() {
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
        }

        function test_zipFakeItems() {
            compare(zipDialog.zipArchiver.itemsCount, 5)

            var zipButton = findChild(zipDialog, "importButton")
            mouseClick(zipButton)

            verify(zipDialog.zipArchiver.inProgress)
            wait(TestsHost.normalSleepTime)
            verify(!zipDialog.zipArchiver.inProgress)
            verify(!zipDialog.zipArchiver.isError)
        }
    }
}
