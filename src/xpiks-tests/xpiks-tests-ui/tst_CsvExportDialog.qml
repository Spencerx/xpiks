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

        sourceComponent: CsvExportDialog {
            anchors.fill: parent
        }
    }

    TestCase {
        name: "CsvExport"
        when: windowShown && (loader.status == Loader.Ready)
        property var columnsListView
        property var exportPlanModelsListView
        property var exportDialog: loader.item

        function initTestCase() {
            columnsListView = findChild(exportDialog, "columnsListView")
            exportPlanModelsListView = findChild(exportDialog, "exportPlanModelsListView")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function cleanup() {
            exportDialog.csvExportModel.clearModel()
            TestsHost.bump()
        }

        function test_addNewRowClickAddsNewRow() {
            var initialCount = exportPlanModelsListView.count
            var button = findChild(exportDialog, "addExportPlanButton")
            mouseClick(button)
            compare(exportPlanModelsListView.count, initialCount + 1)
        }
    }
}
