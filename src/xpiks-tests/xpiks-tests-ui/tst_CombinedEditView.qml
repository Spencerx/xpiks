import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/StackViews"
import "UiTestsStubPlugin"
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    property string path: ''
    property bool isselected: false

    Component.onCompleted: TestsHost.bump()

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: CombinedEditView {
            componentParent: root
            anchors.fill: parent
        }
    }

    TestCase {
        id: testCase
        name: "CombinedEdit"
        when: windowShown && (loader.status == Loader.Ready)
        property var descriptionInput
        property var titleInput
        property var editableTags
        property var descriptionCheckBox
        property var keywordsEdit
        property var combinedView: loader.item

        function initTestCase() {
            TestsHost.setup()
            titleInput = findChild(combinedView, "titleTextInput")
            descriptionInput = findChild(combinedView, "descriptionTextInput")
            editableTags = findChild(combinedView, "editableTags")
            descriptionCheckBox = findChild(combinedView, "descriptionCheckBox")
            keywordsEdit = findChild(combinedView, "nextTagTextInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function cleanup() {
            combinedArtworks.clearModel()
            wait(200)
        }

        function test_TabTopToBottom() {
            combinedArtworks.changeTitle = true
            descriptionCheckBox.checked = true
            combinedArtworks.changeKeywords = true

            titleInput.forceActiveFocus()
            keyClick(Qt.Key_Tab)
            verify(descriptionInput.activeFocus)
            keyClick(Qt.Key_Tab)
            verify(editableTags.isFocused)
        }

        function test_TabOverTitle() {
            combinedArtworks.changeTitle = true
            descriptionCheckBox.checked = false
            combinedArtworks.changeKeywords = true

            titleInput.forceActiveFocus()
            keyClick(Qt.Key_Tab)
            verify(!(descriptionInput.activeFocus))
            verify(editableTags.isFocused)
        }

        function test_TabFromKeywords() {
            combinedArtworks.changeTitle = true
            descriptionCheckBox.checked = false
            combinedArtworks.changeKeywords = true

            editableTags.activateEdit()
            keyClick(Qt.Key_Backtab)
            verify(!(descriptionInput.activeFocus))
            verify(titleInput.activeFocus)
        }

        function test_addKeywordByTyping() {
            compare(combinedArtworks.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(combinedArtworks.keywordsCount, 1)
            compare(combinedArtworks.getKeywordsString(), testKeyword)
        }
    }
}
