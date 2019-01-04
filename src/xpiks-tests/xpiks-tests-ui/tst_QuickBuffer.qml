import QtQuick 2.2
import QtTest 1.1
import XpiksTests 1.0
import xpiks 1.0
import "../../xpiks-qt/CollapserTabs"
import "TestUtils.js" as TestUtils

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

        sourceComponent: QuickBufferTab {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 300
        }
    }

    ClipboardHelper {
        id: clipboard
    }

    TestCase {
        id: testCase
        name: "QuickBuffer"
        when: windowShown && (loader.status == Loader.Ready) && TestsHost.isReady
        property var quickBufferTab: loader.item
        property var titleEdit
        property var descriptionEdit
        property var keywordsEdit
        property var editableTags

        function initTestCase() {
            titleEdit = findChild(quickBufferTab, "titleTextInput")
            descriptionEdit = findChild(quickBufferTab, "descriptionTextInput")

            editableTags = findChild(quickBufferTab, "editableTags")
            keywordsEdit = findChild(editableTags, "nextTagTextInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
            TestUtils.clearEdit(titleEdit)
            TestUtils.clearEdit(descriptionEdit)
            TestUtils.clearEdit(keywordsEdit)
        }

        function test_editTitleSimple() {
            titleEdit.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            compare(quickBuffer.title, testTitle)
        }

        function test_editDescriptionSimple() {
            descriptionEdit.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)
            compare(quickBuffer.description, testDescription)
        }

        function test_addKeywordsSmoke() {
            compare(quickBuffer.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(quickBuffer.keywordsCount, 1)
            compare(quickBuffer.getKeywordsString(), testKeyword)
        }

        function test_copyFromMoreLink() {
            var copyLink = findChild(quickBufferTab, "copyLink")
            compare(copyLink.enabled, false)

            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            // do not press comma - test adding keyword on losing focus

            compare(copyLink.enabled, true)
            copyLink.forceActiveFocus()

            mouseClick(copyLink)

            compare(clipboard.getText(), testKeyword1 + ", " + testKeyword2)
        }

        function test_spellingIsCheckedForWrong() {
            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasSpellCheckError", true, 2000)
        }

        function test_spellingIsCheckedForCorrect() {
            keywordsEdit.forceActiveFocus()

            TestUtils.keyboardEnterText('in')
            keyClick(Qt.Key_Space)

            TestUtils.keyboardEnterText('space')
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            wait(1500)

            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_duplicateIsDetectedWithDescription() {
            descriptionEdit.forceActiveFocus()
            TestUtils.keyboardEnterText('space')

            keywordsEdit.forceActiveFocus()
            TestUtils.keyboardEnterText('space')

            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasDuplicate", true, 2000)
            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_clearKeywords() {
            compare(quickBuffer.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(quickBuffer.keywordsCount, 2)

            var clearLink = findChild(quickBufferTab, "clearLink")
            mouseClick(clearLink)

            compare(quickBuffer.keywordsCount, 0)
            compare(quickBuffer.getKeywordsString(), "")
        }
    }
}
