import QtQuick 2.0
import XpiksTests 1.0
import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import "../../xpiks-qt/Dialogs"
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup()

    QtObject {
        id: appHost
        property bool leftSideCollapsed: false
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: DeleteKeywordsDialog {
            anchors.fill: parent
            componentParent: root
        }
    }

    ClipboardHelper {
        id: clipboard
    }

    TestCase {
        id: testCase
        name: "DeleteKeywords"
        when: windowShown && (loader.status == Loader.Ready)
        property var deleteKeywordsDialog: loader.item
        property var deleteEditableTags
        property var commonEditableTags
        property var keywordsInput

        function initTestCase() {
            deleteEditableTags = findChild(deleteKeywordsDialog, "deleteEditableTags")
            commonEditableTags = findChild(deleteKeywordsDialog, "commonEditableTags")
            keywordsInput = findChild(deleteEditableTags, "nextTagTextInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
            TestUtils.clearEdit(keywordsInput)
        }

        function test_addKeywordToDeleteSimple() {
            compare(deleteKeywordsDialog.deleteKeywordsModel.keywordsToDeleteCount, 0)
            keywordsInput.forceActiveFocus();
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)
            compare(deleteKeywordsDialog.deleteKeywordsModel.keywordsToDeleteCount, 1)
        }

        function test_pasteKeywords() {
            compare(deleteKeywordsDialog.deleteKeywordsModel.keywordsToDeleteCount, 0)
            clipboard.setText("some, basic, keywords")
            keywordsInput.forceActiveFocus();
            keyClick(Qt.Key_V, Qt.ControlModifier)
            compare(deleteKeywordsDialog.deleteKeywordsModel.keywordsToDeleteCount, 3)
        }

        function test_removeFromCommonAddsToDelete() {
            compare(deleteKeywordsDialog.deleteKeywordsModel.keywordsToDeleteCount, 0)
            var commonKeyword = "common"
            deleteKeywordsDialog.deleteKeywordsModel.appendCommonKeyword(commonKeyword)
            var repeater = findChild(commonEditableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)
            keywordWrapper.removeClicked()
            compare(deleteKeywordsDialog.deleteKeywordsModel.keywordsToDeleteCount, 1)
            verify(deleteKeywordsDialog.deleteKeywordsModel.getKeywordsToDeleteObject().hasKeyword(commonKeyword))
        }
    }
}
