import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/StackViews"
import "UiTestsStubPlugin"
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup()

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
        property bool listLayout: true
        function collapseLeftPane() { }
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: MainGrid {
        }
    }

    TestCase {
        id: testCase
        name: "MainGrid"
        when: windowShown && (loader.status == Loader.Ready)
        property var mainGrid: loader.item
        property var artworksHost

        function initTestCase() {
            artworksHost = findChild(mainGrid, "artworksHost")

            var columnLayout = findChild(mainGrid, "columnLayout")
            columnLayout.isWideEnough = true
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
        }

        function getDelegate(index) {
            return TestUtils.getDelegateInstanceAt(artworksHost.contentItem,
                                                   "artworkDelegate",
                                                   index)
        }

        function test_addKeywordBasic() {
            var artworkDelegate = getDelegate(1)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            compare(artworkDelegate.delegateModel.keywordsstring, testKeyword)
        }

        function test_editDescription() {
            var artworkDelegate = getDelegate(1)
            var descriptionInput = findChild(artworkDelegate, "descriptionTextInput")
            descriptionInput.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)
            wait(TestsHost.smallSleepTime)
            compare(artworkDelegate.delegateModel.description, testDescription)
        }

        function test_editTitle() {
            var artworkDelegate = getDelegate(1)
            var titleInput = findChild(artworkDelegate, "titleTextInput")
            titleInput.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            wait(TestsHost.smallSleepTime)
            compare(artworkDelegate.delegateModel.title, testTitle)
        }
    }
}
