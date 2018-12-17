import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup(testCase.name, true)

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: UploadArtworks {
            componentParent: root
            anchors.fill: parent
        }
    }

    TestCase {
        id: testCase
        name: "UploadArtworks"
        when: windowShown && (loader.status == Loader.Ready) && TestsHost.isReady
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

            TestUtils.keyboardEnterText('shttrs')

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
            TestUtils.keyboardEnterText('hutte')

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

            TestUtils.keyboardEnterText('shut')

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
            TestUtils.keyboardEnterText('ftp.abcdef.gh')

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
            // 2 items with vectors, no zipping -> 4 items
            tryCompare(failedLink, "text", "4 failed uploads", 10000)
        }

        function test_zipAndUpload() {
            if (!TestsHost.ftpServerEnabled) { return; }

            filteredArtworksListModel.selectFilteredArtworks()

            mouseClick(addExportPlanButton)

            // leave title as "Untitled"
            keyClick(Qt.Key_Tab)

            // ftp address
            TestUtils.keyboardEnterText('127.0.0.1')

            // username and password
            keyClick(Qt.Key_Tab)
            TestUtils.keyboardEnterText('ftpuser')
            keyClick(Qt.Key_Tab)
            TestUtils.keyboardEnterText('ftppasswd')

            // select host
            var host = TestUtils.getDelegateInstanceAt(uploadHostsListView.contentItem,
                                                       "sourceWrapper",
                                                       0)
            var checkbox = findChild(host, "itemCheckedCheckbox")
            mouseClick(checkbox)
            wait(TestsHost.smallSleepTime)

            var tabsRepeater = findChild(uploadDialog, 'tabsRepeater')
            var advancedTab = tabsRepeater.itemAt(1)
            mouseClick(advancedTab)
            wait(TestsHost.smallSleepTime)

            var zipCheckbox = findChild(uploadDialog, 'zipBeforeUploadCheckBox')
            mouseClick(zipCheckbox)
            wait(TestsHost.smallSleepTime)

            mouseClick(tabsRepeater.itemAt(0))

            verify(!xpiksApp.uploadedFilesExist())

            // do upload now
            var button = findChild(uploadDialog, "uploadButton")
            mouseClick(button)

            var artworkUploader = dispatcher.getCommandTarget(UICommand.SetupUpload)
            var zipArchiver = dispatcher.getCommandTarget(UICommand.SetupCreatingArchives)

            tryCompare(zipArchiver, "inProgress", true, 5000)
            tryCompare(zipArchiver, "inProgress", false, 5000)
            compare(zipArchiver.isError, false)

            tryCompare(artworkUploader, "inProgress", true, 5000)
            tryCompare(artworkUploader, "inProgress", false, 5000)
            compare(artworkUploader.isError, false)

            wait(TestsHost.normalSleepTime)
            verify(xpiksApp.uploadedFilesExist())
        }
    }
}
