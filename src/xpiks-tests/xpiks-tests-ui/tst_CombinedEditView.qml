import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/StackViews"
import "UiTestsStubPlugin"

Item {
    id: root
    width: 800
    height: 600

    property string path: ''
    property bool isselected: false
    property string testsHostMention: TestsHost.scoreme

    QtObject {
        id: keywordsWrapper
        property bool keywordsModel: false
    }

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    CombinedEditView {
        id: combinedView
        anchors.fill: parent
    }

    TestCase {
        name: "CombinedEdit"
        when: windowShown
        property var descriptionInput
        property var titleInput
        property var keywordsInput
        property var descriptionCheckBox

        function initTestCase() {
            TestsHost.setup()
            titleInput = findChild(combinedView, "titleTextInput")
            descriptionInput = findChild(combinedView, "descriptionTextInput")
            keywordsInput = findChild(combinedView, "keywordsInput")
            descriptionCheckBox = findChild(combinedView, "descriptionCheckBox")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function test_TabTopToBottom() {
            combinedArtworks.changeTitle = true
            descriptionCheckBox.checked = true
            combinedArtworks.changeKeywords = true

            titleInput.forceActiveFocus()
            keyClick(Qt.Key_Tab)
            verify(descriptionInput.activeFocus)
            keyClick(Qt.Key_Tab)
            verify(keywordsInput.isFocused)
        }

        function test_TabOverTitle() {
            combinedArtworks.changeTitle = true
            descriptionCheckBox.checked = false
            combinedArtworks.changeKeywords = true

            titleInput.forceActiveFocus()
            keyClick(Qt.Key_Tab)
            verify(!(descriptionInput.activeFocus))
            verify(keywordsInput.isFocused)
        }

        function test_TabFromKeywords() {
            combinedArtworks.changeTitle = true
            descriptionCheckBox.checked = false
            combinedArtworks.changeKeywords = true

            keywordsInput.activateEdit()
            keyClick(Qt.Key_Backtab)
            verify(!(descriptionInput.activeFocus))
            verify(titleInput.activeFocus)
        }
    }
}
