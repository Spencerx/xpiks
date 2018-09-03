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

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

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
            TestsHost.cleanup()
        }

        function cleanup() {
            uploadDialog.uploadInfos.clear()
            TestUtils.clearEdit(titleTextInput)
            TestUtils.clearEdit(ftpAddressInput)
            TestsHost.bump()
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

            wait(TestUtils.normalSleepTime)

            verify(typeof generalTab.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_S)

            keyClick(Qt.Key_Down)
            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestUtils.smallSleepTime)

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

            wait(TestUtils.normalSleepTime)

            verify(typeof generalTab.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_S, Qt.ShiftModifier)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_U)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_E)

            keyClick(Qt.Key_Down)
            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestUtils.smallSleepTime)

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

            wait(TestUtils.normalSleepTime)

            verify(typeof generalTab.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_U)
            keyClick(Qt.Key_T)

            keyClick(Qt.Key_Tab)
            wait(TestUtils.smallSleepTime)

            verify(typeof generalTab.autoCompleteBox === "undefined")
            verify(ftpAddressInput.activeFocus)
            compare(titleTextInput.text, "shut")
            compare(ftpAddressInput.length, 0)
        }
    }
}
