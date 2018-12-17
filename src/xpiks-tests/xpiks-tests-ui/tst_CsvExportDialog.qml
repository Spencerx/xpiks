import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup(testCase.name)

    QtObject {
        id: appHost
        property bool leftSideCollapsed: false
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: CsvExportDialog {
            anchors.fill: parent
        }
    }

    TestCase {
        id: testCase
        name: "CsvExport"
        when: windowShown && (loader.status == Loader.Ready) && TestsHost.isReady
        property var columnsListView
        property var exportPlanModelsListView
        property var exportDialog: loader.item

        function initTestCase() {
            columnsListView = findChild(exportDialog, "columnsListView")
            exportPlanModelsListView = findChild(exportDialog, "exportPlanModelsListView")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
        }

        function test_addNewRowClickAddsNewRow() {
            var initialCount = exportPlanModelsListView.count
            var button = findChild(exportDialog, "addExportPlanButton")
            mouseClick(button)
            wait(TestsHost.smallSleepTime)
            compare(exportPlanModelsListView.count, initialCount + 1)
        }
    }
}
