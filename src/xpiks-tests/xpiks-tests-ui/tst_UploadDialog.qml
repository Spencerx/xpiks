import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"
import "TestUtils.js" as TestUtils

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

        sourceComponent: UploadArtworks {
            anchors.fill: parent
        }
    }

    TestCase {
        id: testCase
        name: "UploadArtworks"
        when: windowShown && (loader.status == Loader.Ready)
        property var uploadDialog: loader.item
        property var uploadHostsListView
        property var addExportPlanButton
        property var titleTextInput
        property var generalTab
        property var ftpAddressInput

        function initTestCase() {
            uploadHostsListView = findChild(uploadDialog, "uploadHostsListView")
            addExportPlanButton = findChild(uploadDialog, "addExportPlanButton")
            titleTextInput = findChild(uploadDialog, "titleTextInput")
            generalTab = findChild(uploadDialog, "generalTab")
            ftpAddressInput = findChild(uploadDialog, "ftpAddressInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestUtils.clearEdit(titleTextInput)
            TestUtils.clearEdit(ftpAddressInput)
            TestsHost.cleanup()
        }

        function test_addButtonClickAddsHost() {
            var initialCount = uploadHostsListView.count
            mouseClick(addExportPlanButton)
            compare(uploadHostsListView.count, initialCount+1)
        }

        function test_stockAutoCompleteFuzzy() {
            mouseClick(addExportPlanButton)

            var initialText = "Untitled"
            compare(titleTextInput.text, initialText)
            verify(titleTextInput.activeFocus)
            verify(typeof generalTab.autoCompleteBox === "undefined")

            for (var i = 0; i < initialText.length; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestsHost.normalSleepTime)

            verify(typeof generalTab.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_S)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            verify(typeof generalTab.autoCompleteBox === "undefined")
            compare(titleTextInput.text, "Shutterstock")
            compare(ftpAddressInput.text, "ftp.shutterstock.com")
        }

        function test_stockAutoCompleteFull() {
            mouseClick(addExportPlanButton)

            var initialText = "Untitled"
            compare(titleTextInput.text, initialText)
            verify(titleTextInput.activeFocus)
            verify(typeof generalTab.autoCompleteBox === "undefined")

            for (var i = 0; i < initialText.length; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestsHost.normalSleepTime)

            verify(typeof generalTab.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_S, Qt.ShiftModifier)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_U)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_E)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            verify(typeof generalTab.autoCompleteBox === "undefined")
            compare(titleTextInput.text, "Shutterstock")
            compare(ftpAddressInput.text, "ftp.shutterstock.com")
        }

        function test_tabFromInputClosesAutoComplete() {
            mouseClick(addExportPlanButton)

            var initialText = "Untitled"
            compare(titleTextInput.text, initialText)
            verify(titleTextInput.activeFocus)
            verify(typeof generalTab.autoCompleteBox === "undefined")

            for (var i = 0; i < initialText.length; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestsHost.normalSleepTime)

            verify(typeof generalTab.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_U)
            keyClick(Qt.Key_T)

            keyClick(Qt.Key_Tab)
            wait(TestsHost.smallSleepTime)

            verify(typeof generalTab.autoCompleteBox === "undefined")
            verify(ftpAddressInput.activeFocus)
            compare(titleTextInput.text, "shut")
            compare(ftpAddressInput.length, 0)
        }

        function test_uploadToFakeHost() {
            mouseClick(addExportPlanButton)
            // leave title as "Untitled"
            keyClick(Qt.Key_Tab)

            // ftp address
            keyClick(Qt.Key_F)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_Period)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_B)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_D)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_F)
            keyClick(Qt.Key_Period)
            keyClick(Qt.Key_G)
            keyClick(Qt.Key_H)

            // username and password
            keyClick(Qt.Key_Tab)
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Tab)
            TestUtils.keyboardEnterSomething(testCase)

            // select host
            var host = TestUtils.getDelegateInstanceAt(uploadHostsListView.contentItem,
                                                       "sourceWrapper",
                                                       0)
            var checkbox = findChild(host, "itemCheckedCheckbox")
            mouseClick(checkbox)

            wait(TestsHost.smallSleepTime)

            var button = findChild(uploadDialog, "uploadButton")
            mouseClick(button)

            var failedLink = findChild(uploadDialog, "failedArtworksStatus")
            // 5 items with vectors, no zipping -> 10 items
            tryCompare(failedLink, "text", "10 failed uploads", 10000)
        }
    }
}
