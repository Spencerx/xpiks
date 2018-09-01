import QtQuick 2.0
import XpiksTests 1.0
import QtQuick 2.0
import QtTest 1.1
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

        sourceComponent: PresetsEditDialog {
            anchors.fill: parent
            componentParent: root
        }
    }

    TestCase {
        id: testCase
        name: "PresetsTests"
        when: windowShown && (loader.status == Loader.Ready)
        property var presetEditsDialog: loader.item
        property var presetNamesListView
        property var keywordsEdit
        property var addPresetButton
        property var editableTags

        function initTestCase() {
            presetsModel.cleanup()
            presetNamesListView = findChild(presetEditsDialog, "presetNamesListView")
            keywordsEdit = findChild(presetEditsDialog, "nextTagTextInput")
            addPresetButton = findChild(presetEditsDialog, "addPresetButton")
            editableTags = findChild(presetEditsDialog, "editableTags")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function cleanup() {
            keywordsEdit.remove(0, keywordsEdit.length)
            presetsModel.cleanup()
            TestsHost.bump()
        }

        function test_addKeywordBasic() {
            mouseClick(addPresetButton)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(presetsModel.getKeywordsString(0), testKeyword)
            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)
            verify(typeof keywordWrapper !== "undefined")
        }

        function test_addNewPreset() {
            var initialCount = presetNamesListView.count
            mouseClick(addPresetButton)
            compare(presetNamesListView.count, initialCount + 1)
        }

        function test_autoCompleteBasic() {
            mouseClick(addPresetButton)

            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_W)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_A)

            wait(TestUtils.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_E)

            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestUtils.smallSleepTime)

            compare(presetsModel.getKeywordsString(0), "weather")
        }

        function test_autoCompleteCancelWhenShortText() {
            mouseClick(addPresetButton)
            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_U)

            wait(TestUtils.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Backspace)

            wait(TestUtils.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")
        }

        function test_autoCompletePresetShouldNotWork() {
            mouseClick(addPresetButton)
            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_F)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)

            wait(TestUtils.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Down)
            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Return, Qt.ControlModifier)
            wait(TestUtils.smallSleepTime)

            compare(presetsModel.getKeywordsString(0), "interface")
        }

        function test_autoCompleteIntoNonEmptyEdit() {
            mouseClick(addPresetButton)
            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_Space)

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)

            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestUtils.smallSleepTime)

            compare(keywordsEdit.text, "in space")
        }

        function test_makeTitleValid() {
            mouseClick(addPresetButton)
            var titleEdit = findChild(presetEditsDialog, "titleEdit")
            titleEdit.forceActiveFocus()

            var testKeyword = TestUtils.keyboardEnterSomething(testCase)

            for (var i = 0; i < testKeyword.length; i++) {
                keyClick(Qt.Key_Backspace)
            }

            keyClick(Qt.Key_Tab)

            compare(titleEdit.text, "Untitled")
        }

        function test_editInPlainText() {
            mouseClick(addPresetButton)

            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestUtils.smallSleepTime)

            var link = findChild(presetEditsDialog, "plainTextLink")
            mouseClick(link)

            wait(TestUtils.normalSleepTime)

            // hack to detect if plain text edit hasn't started
            keyClick(Qt.Key_Comma)
            keyClick(Qt.Key_Backspace)

            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)

            wait(TestUtils.smallSleepTime)

            keyClick(Qt.Key_Enter, Qt.ControlModifier)

            wait(TestUtils.normalSleepTime)

            compare(presetsModel.getKeywordsString(0), testKeyword1 + ", " + testKeyword2)
        }

        function test_doubleClickEditsKeyword() {
            mouseClick(addPresetButton)

            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestUtils.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            compare(presetsModel.getKeywordsString(0), testKeyword1)
            mouseDoubleClick(keywordWrapper)

            wait(TestUtils.smallSleepTime)

            for (var i = 0; i < testKeyword1.length; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestUtils.smallSleepTime)

            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Enter)

            wait(TestUtils.smallSleepTime)

            compare(presetsModel.getKeywordsString(0), testKeyword2)
        }
    }
}
