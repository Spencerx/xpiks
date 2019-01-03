import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/"
import "../../xpiks-qt/Common.js" as Common
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    property string path: ''
    property bool isselected: false
    property int openedDialogsCount: 0

    function onDialogClosed() {
        openedDialogsCount -= 1
    }

    Component.onCompleted: TestsHost.setup(testCase.name)

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

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Backtab)

            TestUtils.keyboardEnterText(testKeyword)

            wait(TestsHost.normalSleepTime)

            var duplicatesLink = findChild(artworkDelegate, "removeDuplicatesText")
            tryCompare(filteredArtworksListModel.getBasicModelObject(1), "hasDuplicates", true, 5000)
            verify(duplicatesLink.enabled)

            mouseClick(duplicatesLink)
            wait(TestsHost.smallSleepTime)

            compare(mainStackView.currentItem.objectName, "DuplicatesReView")
        }
    }
}
