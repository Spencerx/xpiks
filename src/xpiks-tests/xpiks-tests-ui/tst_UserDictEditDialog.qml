import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import xpiks 1.0
import "../../xpiks-qt/Dialogs"
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

        sourceComponent: UserDictEditDialog {
            componentParent: root
        }
    }

    TestCase {
        id: testCase
        name: "QuickBuffer"
        when: windowShown && (loader.status == Loader.Ready) && TestsHost.isReady
        property var userDictEditDialog: loader.item
        property var keywordsEdit
        property var editableTags

        function initTestCase() {
            editableTags = findChild(userDictEditDialog, "editableTags")
            keywordsEdit = findChild(editableTags, "nextTagTextInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
            TestUtils.clearEdit(keywordsEdit)
        }

        function test_addKeywordsSmoke() {
            compare(userDictEditDialog.userDictEditModel.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(userDictEditDialog.userDictEditModel.keywordsCount, 1)
            compare(userDictEditDialog.userDictEditModel.getKeywordsString(), testKeyword)
        }

        function test_addRemoveKeyowrd() {
            keywordsEdit.forceActiveFocus()

            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(userDictEditDialog.userDictEditModel.keywordsCount, 2)
            compare(userDictEditDialog.userDictEditModel.getKeywordsString(), testKeyword1 + ', ' + testKeyword2)

            keyClick(Qt.Key_Backspace)
            compare(userDictEditDialog.userDictEditModel.keywordsCount, 1)
            keyClick(Qt.Key_Backspace)
            compare(userDictEditDialog.userDictEditModel.keywordsCount, 0)
        }
    }
}
