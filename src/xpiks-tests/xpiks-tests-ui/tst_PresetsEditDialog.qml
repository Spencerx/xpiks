import QtQuick 2.2
import XpiksTests 1.0
import QtQuick 2.2
import QtTest 1.1
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

        sourceComponent: PresetsEditDialog {
            anchors.fill: parent
            componentParent: root
        }
    }

    TestCase {
        id: testCase
        name: "PresetsTests"
        when: windowShown && (loader.status == Loader.Ready) && TestsHost.isReady
        property var presetEditsDialog: loader.item
        property var presetNamesListView
        property var keywordsEdit
        property var addPresetButton
        property var editableTags

        function initTestCase() {
            presetsModel.clearModel()
            presetNamesListView = findChild(presetEditsDialog, "presetNamesListView")
            keywordsEdit = findChild(presetEditsDialog, "nextTagTextInput")
            addPresetButton = findChild(presetEditsDialog, "addPresetButton")
            editableTags = findChild(presetEditsDialog, "editableTags")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestUtils.clearEdit(keywordsEdit)
            presetsModel.clearModel()
            TestsHost.cleanup()
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

            TestUtils.keyboardEnterText('wea')

            wait(TestsHost.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            TestUtils.keyboardEnterText('the')

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            compare(presetsModel.getKeywordsString(0), "weather")
        }

        function test_autoCompleteCancelWhenShortText() {
            mouseClick(addPresetButton)
            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            TestUtils.keyboardEnterText('tru')

            wait(TestsHost.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Backspace)

            wait(TestsHost.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")
        }

        function test_autoCompletePresetShouldNotWork() {
            mouseClick(addPresetButton)
            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            TestUtils.keyboardEnterText('interfac')

            wait(TestsHost.smallSleepTime)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return, Qt.ControlModifier)
            wait(TestsHost.smallSleepTime)

            compare(presetsModel.getKeywordsString(0), "interface")
        }

        function test_autoCompleteIntoNonEmptyEdit() {
            mouseClick(addPresetButton)
            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            TestUtils.keyboardEnterText('in')
            keyClick(Qt.Key_Space)

            TestUtils.keyboardEnterText('spac')

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

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

            wait(TestsHost.smallSleepTime)

            var link = findChild(presetEditsDialog, "plainTextLink")
            mouseClick(link)

            wait(TestsHost.normalSleepTime)

            // hack to detect if plain text edit hasn't started
            keyClick(Qt.Key_Comma)
            keyClick(Qt.Key_Backspace)

            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Enter, Qt.ControlModifier)

            wait(TestsHost.normalSleepTime)

            compare(presetsModel.getKeywordsString(0), testKeyword1 + ", " + testKeyword2)
        }

        function test_doubleClickEditsKeyword() {
            mouseClick(addPresetButton)

            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            compare(presetsModel.getKeywordsString(0), testKeyword1)
            mouseDoubleClick(keywordWrapper)

            wait(TestsHost.smallSleepTime)

            for (var i = 0; i < testKeyword1.length; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestsHost.smallSleepTime)

            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Enter)

            wait(TestsHost.smallSleepTime)

            compare(presetsModel.getKeywordsString(0), testKeyword2)
        }

        function test_addPresetGroup() {
            var groupCombobox = findChild(presetEditsDialog, "groupsCombobox")
            var selectedText = findChild(groupCombobox, "selectedText");

            mouseClick(addPresetButton)

            compare(selectedText.text, "Default")

            var titleEdit = findChild(presetEditsDialog, "titleEdit")
            titleEdit.forceActiveFocus()

            var size = titleEdit.text.length
            for (var i = 0; i < size; i++) {
                keyClick(Qt.Key_Backspace)
            }

            var testKeyword = TestUtils.keyboardEnterSomething(testCase)

            mouseClick(groupCombobox)
            wait(TestsHost.smallSleepTime)

            var lastItem = findChild(root, "lastItem")
            // should call group dialog
            mouseClick(lastItem)
            wait(TestsHost.smallSleepTime)

            var groupName = "my group 99"
            TestUtils.keyboardEnterText(groupName)
            keyClick(Qt.Key_Return)

            compare(selectedText.text, groupName)

            // now switch back and forth

            mouseClick(addPresetButton)
            wait(TestsHost.smallSleepTime)
            compare(selectedText.text, "Default")

            mouseClick(TestUtils.getDelegateInstanceAt(presetNamesListView.contentItem, "presetDelegate", 0))
            wait(TestsHost.smallSleepTime)
            compare(selectedText.text, groupName)
        }
    }
}
