import QtQuick 2.2
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/"
import "../../xpiks-qt/Common.js" as Common
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 1200
    height: 800

    property int openedDialogsCount: 0

    function onDialogCreated(name) {
        console.log("tst_AppHost # dialogCreated " + name)
        openedDialogsCount += 1
    }

    function onDialogClosed() {
        openedDialogsCount -= 1
    }

    Component.onCompleted: TestsHost.setup(testCase.name)

    QtObject {
        id: applicationWindow
        property int openedDialogsCount: 0
    }

    QtObject {
        id: saveAction
        property bool enabled: true
    }

    QtObject {
        id: uploadAction
        property bool enabled: true
    }

    QtObject {
        id: addFilesAction
        property bool enabled: true
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: AppHost {
            anchors.fill: parent
            leftSideCollapsed: true
            componentParent: root
        }
    }

    ClipboardHelper {
        id: clipboard
    }

    TestCase {
        id: testCase
        name: "AppHost"
        when: windowShown && (loader.status == Loader.Ready) && TestsHost.isReady
        property var appHost: loader.item
        property var mainStackView
        property var mainGrid
        property var artworksHost

        function initTestCase() {
            mainStackView = findChild(appHost, "mainStackView")
            mainGrid = findChild(mainStackView, "MainGrid")
            artworksHost = findChild(mainGrid, "artworksHost")

            filteredArtworksListModel.unselectFilteredArtworks()
            // 2nd half of the images
            filteredArtworksListModel.searchTerm = "x:image"

            wait(TestsHost.normalSleepTime)
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            // pop all items before main grid
            mainStackView.pop(null)
            wait(TestsHost.smallSleepTime)
            TestsHost.cleanup()
        }

        function getDelegate(index) {
            var artworkDelegate = TestUtils.getDelegateInstanceAt(artworksHost.contentItem,
                                                                  "artworkDelegate",
                                                                  index)
            artworksHost.positionViewAtIndex(index, GridView.Contain)
            return artworkDelegate
        }

        function selectDelegate(index) {
            var artworkDelegate = getDelegate(index)
            verify(artworkDelegate)

            var imageHost = findChild(artworkDelegate, "imageHost")
            verify(imageHost)
            mouseClick(imageHost)
        }

        function test_doubleClickArtwork() {
            var artworkDelegate = getDelegate(3)
            verify(artworkDelegate)

            var imageHost = findChild(artworkDelegate, "imageHost")
            verify(imageHost)
            mouseClick(imageHost)
            mouseClick(imageHost)

            wait(TestsHost.smallSleepTime)

            compare(mainStackView.currentItem.objectName, "ArtworkEditView")
        }

        function test_reviewDuplicates() {
            var artworkDelegate = getDelegate(1)
            verify(artworkDelegate)

            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            verify(keywordsEdit)
            keywordsEdit.forceActiveFocus()
            var testKeyword = 'duplicate'
            TestUtils.keyboardEnterText(testKeyword)
            keyClick(Qt.Key_Backtab)

            // enter title as well
            TestUtils.keyboardEnterText(testKeyword)

            wait(TestsHost.normalSleepTime)

            tryCompare(filteredArtworksListModel.getBasicModelObject(1), "hasDuplicates", true, 5000)
            wait(TestsHost.normalSleepTime)

            var duplicatesLink = findChild(artworkDelegate, "removeDuplicatesText")
            verify(duplicatesLink.enabled)
            // wait for the spelling link to appear
            wait(TestsHost.normalSleepTime)

            mouseClick(duplicatesLink)
            wait(TestsHost.normalSleepTime)

            compare(mainStackView.currentItem.objectName, "DuplicatesReView")

            var duplicatesListView = findChild(mainStackView.currentItem, "duplicatesListView")
            var duplicatesItem = TestUtils.getDelegateInstanceAt(duplicatesListView.contentItem,
                                                                 "duplicatesDelegate",
                                                                 0)
            verify(duplicatesItem)

            var titleText = findChild(duplicatesItem, "titleText")
            compare(titleText.text, testKeyword)
        }

        function test_selectEditSingleItem() {
            selectDelegate(0)
            wait(TestsHost.smallSleepTime)

            var editToolButton = findChild(mainGrid, "editToolButton")
            verify(editToolButton)
            mouseClick(editToolButton)
            wait(TestsHost.normalSleepTime)

            compare(mainStackView.currentItem.objectName, "ArtworkEditView")
        }

        function test_selectEditMultipleItems() {
            selectDelegate(0)
            wait(TestsHost.smallSleepTime)
            selectDelegate(1)
            wait(TestsHost.smallSleepTime)

            var editToolButton = findChild(mainGrid, "editToolButton")
            verify(editToolButton)
            mouseClick(editToolButton)
            wait(TestsHost.normalSleepTime)

            compare(mainStackView.currentItem.objectName, "CombinedEditView")
        }
    }
}
