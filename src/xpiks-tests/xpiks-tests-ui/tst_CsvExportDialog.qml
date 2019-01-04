import QtQuick 2.2
import QtTest 1.1
import XpiksTests 1.0
import xpiks 1.0
import "../../xpiks-qt/Dialogs"
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup(testCase.name, true)

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

        function configureProperty(columnIndex, propertyIndex) {
            var column0 = TestUtils.getDelegateInstanceAt(columnsListView.contentItem, "columnDelegate", columnIndex)
            var column0Edit = findChild(column0, "columnNameText")
            TestUtils.clearEdit(column0Edit)
            wait(TestsHost.smallSleepTime)

            column0Edit.forceActiveFocus()
            var column0Name = TestUtils.keyboardEnterSomething(testCase)
            var column0ComboBox = findChild(column0, "propertiesCombobox")
            mouseClick(column0ComboBox)
            wait(TestsHost.smallSleepTime)

            var dropDownItems = findChild(exportDialog, "dropDownItems")
            var filenameProperty = TestUtils.getDelegateInstanceAt(dropDownItems.contentItem, "dropDownItem", propertyIndex)
            mouseClick(filenameProperty)
            wait(TestsHost.smallSleepTime)

            return column0Name
        }

        // test should be always the last one because of the popup
        function test_zzzSimpleExport() {
            var initialCount = exportPlanModelsListView.count
            var button = findChild(exportDialog, "addExportPlanButton")
            mouseClick(button)
            wait(TestsHost.smallSleepTime)
            compare(exportPlanModelsListView.count, initialCount + 1)

            var addColumnButton = findChild(exportDialog, "addColumnButton")

            mouseClick(addColumnButton)
            wait(TestsHost.smallSleepTime)
            // select FileName
            var column0Name = configureProperty(0, 1)

            mouseClick(addColumnButton)
            wait(TestsHost.smallSleepTime)
            // select Keywords
            var column1Name = configureProperty(1, 4)

            var exportPlan = TestUtils.getDelegateInstanceAt(exportPlanModelsListView.contentItem,
                                                             "exportPlanDelegate",
                                                             2)
            var checkbox = findChild(exportPlan, "itemCheckedCheckbox")
            mouseClick(checkbox)
            wait(TestsHost.smallSleepTime)

            // simulate "Start Export" button click
            // TODO: implement real "middleware"
            // passing empty parameter should cause to export to "Downloads"
            dispatcher.dispatch(UICommand.StartCSVExport, xpiksApp.sessionDirectory)

            var csvModel = dispatcher.getCommandTarget(UICommand.SetupCSVExportForSelected)
            tryCompare(csvModel, "isExporting", true, 5000)
            tryCompare(csvModel, "isExporting", false, 5000)

            verify(xpiksApp.csvExportIsValid(column0Name, column1Name))
        }
    }
}
