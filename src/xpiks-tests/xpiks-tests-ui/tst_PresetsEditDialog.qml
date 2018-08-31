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

        function initTestCase() {
            TestsHost.setup()
            presetsModel.cleanup()
            presetNamesListView = findChild(presetEditsDialog, "presetNamesListView")
            keywordsEdit = findChild(presetEditsDialog, "nextTagTextInput")
            addPresetButton = findChild(presetEditsDialog, "addPresetButton")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function cleanup() {
            presetsModel.cleanup()
            keywordsEdit.remove(0, keywordsEdit.length)
            wait(500)
        }

        function test_addNewPreset() {
            var initialCount = presetNamesListView.count
            mouseClick(addPresetButton)
            compare(presetNamesListView.count, initialCount + 1)
        }

        function test_addKeywordBasic() {
            mouseClick(addPresetButton)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(presetsModel.getKeywordsString(0), testKeyword)
        }

        function test_autoCompleteBasic() {
            mouseClick(addPresetButton)

            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_W)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_A)

            wait(200)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_E)

            wait(200)

            keyClick(Qt.Key_Down)
            wait(200)

            keyClick(Qt.Key_Return)
            wait(200)

            compare(presetsModel.getKeywordsString(0), "weather")
        }

        function test_autoCompleteCancelWhenShortText() {
            mouseClick(addPresetButton)
            verify(typeof presetEditsDialog.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_U)

            wait(200)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Backspace)

            wait(200)

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

            wait(200)

            verify(typeof presetEditsDialog.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Down)
            wait(200)

            keyClick(Qt.Key_Return, Qt.ControlModifier)
            wait(200)

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

            wait(200)

            keyClick(Qt.Key_Down)
            wait(200)

            keyClick(Qt.Key_Return)
            wait(200)

            compare(keywordsEdit.text, "in space")
        }
    }
}
