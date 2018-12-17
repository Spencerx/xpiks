import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/Components"

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup(testCase.name)

    Rectangle {
        anchors.fill: parent
        color: uiColors.defaultControlColor

        EditableTags {
            id: editableTags
            anchors.fill: parent

            delegate: KeywordWrapper {
                itemHeight: 10
            }
        }
    }

    SignalSpy {
        id: tagAddedSpy
        target: editableTags
        signalName: "tagAdded"
    }

    SignalSpy {
        id: removeLastSpy
        target: editableTags
        signalName: "removeLast"
    }

    SignalSpy {
        id: tagsPastedSpy
        target: editableTags
        signalName: "tagsPasted"
    }

    SignalSpy {
        id: copyAllSpy
        target: editableTags
        signalName: "copyRequest"
    }

    SignalSpy {
        id: completionRequestedSpy
        target: editableTags
        signalName: "completionRequested"
    }

    SignalSpy {
        id: expandLastPresetSpy
        target: editableTags
        signalName: "expandLastAsPreset"
    }

    ClipboardHelper {
        id: clipboard
    }

    QtObject {
        id: acSource
        property bool isActive: false
        property bool tryExpandPresetFlag: false
        property bool isCancelled: false
        property bool isAccepted: false
        property bool isAnySelected: false

        function moveSelectionUp() {}
        function cancelCompletion() { isCancelled = true }
        function moveSelectionDown() {}

        function acceptSelected(tryExpandPreset) {
            tryExpandPresetFlag = tryExpandPreset
            isAccepted = true
        }

        function hasSelectedCompletion() { return isAnySelected }

        function clear() {
            isActive = false
            isCancelled = false
            tryExpandPresetFlag = false
            isAccepted = false
            isAnySelected = false
        }
    }

    TestCase {
        id: testCase
        name: "EditableTagsTests"
        when: windowShown
        property var input

        function initTestCase() {
            input = findChild(editableTags, "nextTagTextInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            tagAddedSpy.clear()
            removeLastSpy.clear()
            acSource.clear()
            tagsPastedSpy.clear()
            completionRequestedSpy.clear()
            copyAllSpy.clear()
            input.text = ""
            TestsHost.cleanup()
        }

        function test_PressCommaAddsKeyword() {
            compare(tagAddedSpy.count, 0)

            input.text = "next_keyword"
            input.forceActiveFocus()
            keyClick(Qt.Key_Comma)

            compare(tagAddedSpy.count, 1)
            compare(acSource.isCancelled, true)
        }

        function test_GetTextAPI() {
            input.text = "keyword"
            compare(editableTags.getEditedText(), "keyword")
        }

        function test_RaiseTagAdded() {
            editableTags.raiseAddTag("any text")
            compare(tagAddedSpy.count, 1)
        }

        function test_PressCommaCleansInput() {
            input.text = "next_keyword"
            input.forceActiveFocus()
            keyClick(Qt.Key_Comma)

            compare(input.text.length, 0)
        }

        function test_BackspaceRemovesPrevItem() {
            input.text = "1"
            // need to set focus
            input.forceActiveFocus()
            input.cursorPosition = 1
            keyClick(Qt.Key_Backspace)

            compare(removeLastSpy.count, 0)
            compare(input.length, 0)

            keyClick(Qt.Key_Backspace)
            compare(removeLastSpy.count, 1)
        }

        function test_SimplePasteNoComma() {
            input.text = ""
            clipboard.setText("keyword")

            input.forceActiveFocus()
            keyClick(Qt.Key_V, Qt.ControlModifier)

            compare(input.text, "keyword")
            compare(tagAddedSpy.count, 0)
            compare(tagsPastedSpy.count, 0)
        }

        function test_SimplePasteWithComma() {
            input.text = ""
            clipboard.setText("keyword1,keyword2")

            input.forceActiveFocus()
            keyClick(Qt.Key_V, Qt.ControlModifier)

            compare(input.text, "")
            compare(tagAddedSpy.count, 0)
            compare(tagsPastedSpy.count, 1)
            var list = tagsPastedSpy.signalArguments[0][0]
            compare(list, ["keyword1", "keyword2"])
        }

        function test_PasteOneItemWithComma() {
            input.text = ""
            clipboard.setText("keyword , ")

            input.forceActiveFocus()
            keyClick(Qt.Key_V, Qt.ControlModifier)

            compare(input.text, "keyword , ")
            compare(tagsPastedSpy.count, 0)
            compare(tagAddedSpy.count, 0)
        }

        function test_PasteOnlyCommas() {
            clipboard.setText(" , , ")

            input.forceActiveFocus()
            keyClick(Qt.Key_V, Qt.ControlModifier)

            compare(input.text, " , , ")
            compare(tagsPastedSpy.count, 0)
            compare(tagAddedSpy.count, 0)
        }

        function test_SimplePasteWithSemicolon() {
            clipboard.setText(";;keyword1;keyword2;;;")

            input.forceActiveFocus()
            keyClick(Qt.Key_V, Qt.ControlModifier)

            compare(input.text, "")
            compare(tagAddedSpy.count, 0)
            compare(tagsPastedSpy.count, 1)
            var list = tagsPastedSpy.signalArguments[0][0]
            compare(list, ["keyword1", "keyword2"])
        }

        function test_CopyRequestWhenEmpty() {
            input.forceActiveFocus()
            keyClick(Qt.Key_C, Qt.ControlModifier)

            compare(input.text, "")
            compare(copyAllSpy.count, 1)
        }

        function test_AutoCompleteFromThreeChars() {
            completionRequestedSpy.clear()
            input.text = ""
            input.forceActiveFocus()

            keyClick(Qt.Key_A)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_B)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_C)
            compare(completionRequestedSpy.count, 1)

            var arg = completionRequestedSpy.signalArguments[0][0]
            compare(arg, "abc")
        }

        function test_AutoCompleteFromFourChars() {
            completionRequestedSpy.clear()
            input.text = ""
            input.forceActiveFocus()

            keyClick(Qt.Key_A)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_B)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_C)
            compare(completionRequestedSpy.count, 1)

            var arg0 = completionRequestedSpy.signalArguments[0][0]
            compare(arg0, "abc")

            keyClick(Qt.Key_D)
            compare(completionRequestedSpy.count, 2)

            var arg1 = completionRequestedSpy.signalArguments[1][0]
            compare(arg1, "abcd")
        }

        function test_AutoCompleteBasicCtrlSpace() {
            completionRequestedSpy.clear()
            input.text = ""
            input.forceActiveFocus()

            input.text = "abc"
            input.cursorPosition = 3

            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Space, Qt.ControlModifier)

            compare(completionRequestedSpy.count, 1)
        }

        function test_AutoCompleteCtrlSpaceTooShort() {
            completionRequestedSpy.clear()
            input.text = ""
            input.forceActiveFocus()

            input.text = "ab"
            input.cursorPosition = 2

            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Space, Qt.ControlModifier)

            compare(completionRequestedSpy.count, 0)
        }

        function test_AutoCompletionCancelOnSpace() {
            completionRequestedSpy.clear()
            acSource.clear()

            input.text = ""
            input.forceActiveFocus()

            input.text = "abc"
            input.cursorPosition = 3

            compare(acSource.isCancelled, false)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Space)

            compare(completionRequestedSpy.count, 0)
            compare(acSource.isCancelled, true)
        }

        function test_AcceptWithActiveAutoComplete() {
            completionRequestedSpy.clear()
            acSource.clear()

            input.text = ""
            input.forceActiveFocus()

            keyClick(Qt.Key_A)
            keyClick(Qt.Key_B)
            keyClick(Qt.Key_C)

            compare(acSource.isAccepted, false)
            acSource.isActive = true

            keyClick(Qt.Key_Return)

            compare(acSource.isAccepted, true)
        }

        function test_AcceptNoSelectedWithActiveAC() {
            input.forceActiveFocus()

            keyClick(Qt.Key_A)
            keyClick(Qt.Key_B)
            keyClick(Qt.Key_C)

            compare(acSource.isAccepted, false)
            compare(tagAddedSpy.count, 0)
            acSource.isActive = true
            acSource.isAnySelected = false

            keyClick(Qt.Key_Return)

            compare(acSource.isAccepted, true)
            compare(tagAddedSpy.count, 1)
        }

        function test_AutoCompletePreset() {
            input.forceActiveFocus()

            keyClick(Qt.Key_A)
            keyClick(Qt.Key_B)
            keyClick(Qt.Key_C)

            compare(acSource.isAccepted, false)
            compare(acSource.tryExpandPresetFlag, false)
            acSource.isActive = true

            keyClick(Qt.Key_Return, Qt.ControlModifier)

            compare(acSource.isAccepted, true)
            compare(acSource.tryExpandPresetFlag, true)
        }

        function test_MouseClickActivatesEdit() {
            root.forceActiveFocus()
            compare(input.activeFocus, false)

            mouseClick(editableTags)

            compare(input.activeFocus, true)
        }

        function test_GetWordsCountAPI() {
            input.text = ""
            compare(input.getWordsCount(), 0)

            input.text = " "
            compare(input.getWordsCount(), 0)

            input.text = "a"
            compare(input.getWordsCount(), 1)

            input.text = " a "
            compare(input.getWordsCount(), 1)

            input.text = "abcd"
            compare(input.getWordsCount(), 1)

            input.text = "ab cd"
            compare(input.getWordsCount(), 2)

            input.text = " ab cd "
            compare(input.getWordsCount(), 2)
        }

        function test_AcceptCompletionSingle() {
            input.text = "abc"
            input.cursorPosition = input.length
            compare(tagAddedSpy.count, 0)

            input.acceptCompletion("abcdee")

            compare(input.text, "")
            compare(tagAddedSpy.count, 1)
            compare(tagAddedSpy.signalArguments[0][0], "abcdee")
            compare(input.cursorPosition, 0)
        }

        function test_AcceptCompletionMoreThanOne() {
            input.text = "abc def"
            input.cursorPosition = input.length
            compare(tagAddedSpy.count, 0)

            input.acceptCompletion("llll")

            compare(input.text, "abc llll")
            compare(tagAddedSpy.count, 0)
            compare(input.cursorPosition, input.length)
        }

        function test_AcceptCompletionToEmpty() {
            input.text = "abc"
            input.cursorPosition = input.length
            compare(tagAddedSpy.count, 0)

            input.acceptCompletion("")

            compare(input.text, "")
            compare(tagAddedSpy.count, 0)
            compare(input.cursorPosition, 0)
        }

        function test_AcceptCompletionToEmptyWithMoreThanOne() {
            input.text = "abc def"
            input.cursorPosition = input.length
            compare(tagAddedSpy.count, 0)

            input.acceptCompletion("")

            compare(input.text, "abc ")
            compare(tagAddedSpy.count, 0)
            compare(input.cursorPosition, input.length)
        }

        function test_AcceptCompletionToEmptyWithMoreThanOneInTheMiddle() {
            input.text = "abc def"
            input.cursorPosition = 3
            compare(tagAddedSpy.count, 0)

            input.acceptCompletion("")

            compare(input.text, " def")
            compare(tagAddedSpy.count, 0)
            compare(input.cursorPosition, 0)
        }

        function test_RequestCompletionEmpty() {
            input.text = ""
            input.requestCompletion()
            compare(completionRequestedSpy.count, 0)
        }

        function test_RequestCompletionTooSmall() {
            input.text = "ab"
            input.requestCompletion()
            compare(completionRequestedSpy.count, 0)
        }

        function test_RequestCompletionMinAllowed() {
            input.text = "abc"
            input.requestCompletion()
            compare(completionRequestedSpy.count, 1)
            compare(completionRequestedSpy.signalArguments[0][0], "abc")
        }

        function test_RequestCompletionBigger() {
            input.text = "abcdef"
            input.requestCompletion()
            compare(completionRequestedSpy.count, 1)
            compare(completionRequestedSpy.signalArguments[0][0], "abcdef")
        }

        function test_CurrentWordStartEnd() {
            compare(input.getCurrentWordStart(), 0)
            compare(input.getCurrentWordEnd(), 0)

            input.text = "a"
            compare(input.getCurrentWordStart(), 0)
            compare(input.getCurrentWordEnd(), 1)

            input.text = "abc"
            compare(input.getCurrentWordStart(), 0)
            compare(input.getCurrentWordEnd(), 3)

            input.text = "  abc  "
            input.cursorPosition = 3
            compare(input.getCurrentWordStart(), 2)
            compare(input.getCurrentWordEnd(), 5)

            input.text = "  abc  def"
            input.cursorPosition = input.length
            compare(input.getCurrentWordStart(), input.length - 3)
            compare(input.getCurrentWordEnd(), input.length)
        }

        function test_LosesFocusSubmitsKeyword() {
            compare(tagAddedSpy.count, 0)

            input.text = "next_keyword"
            input.forceActiveFocus()

            root.forceActiveFocus()

            compare(tagAddedSpy.count, 1)
            compare(tagAddedSpy.signalArguments[0][0], "next_keyword")
        }

        function test_LosesFocusCancelsCompletion() {
            input.forceActiveFocus()

            input.text = "abc"
            input.cursorPosition = 3

            compare(acSource.isCancelled, false)
            compare(completionRequestedSpy.count, 0)

            root.forceActiveFocus()

            compare(completionRequestedSpy.count, 0)
            compare(acSource.isCancelled, true)
        }

        function test_EscapeCancelsCompletion() {
            input.forceActiveFocus()

            input.text = "abc"
            input.cursorPosition = 3

            acSource.isActive = true
            compare(acSource.isCancelled, false)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Escape)

            compare(completionRequestedSpy.count, 0)
            compare(acSource.isCancelled, true)
        }

        function test_KeyLeftCancelsCompletion() {
            input.forceActiveFocus()

            input.text = "abc"
            input.cursorPosition = 3

            acSource.isActive = true
            compare(acSource.isCancelled, false)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Left)

            compare(completionRequestedSpy.count, 0)
            compare(acSource.isCancelled, true)
        }

        function test_KeyRightCancelsCompletion() {
            input.forceActiveFocus()

            input.text = "abc"
            input.cursorPosition = 3

            acSource.isActive = true
            compare(acSource.isCancelled, false)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Right)

            compare(completionRequestedSpy.count, 0)
            compare(acSource.isCancelled, true)
        }

        function test_BackspaceCancelsCompletion() {
            input.forceActiveFocus()

            input.text = "abc"
            input.cursorPosition = 3

            acSource.isActive = true
            compare(acSource.isCancelled, false)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Backspace)

            compare(completionRequestedSpy.count, 0)
            compare(acSource.isCancelled, true)
        }

        function test_BackspaceDoesNotCancelCompletionIfLong() {
            input.forceActiveFocus()

            input.text = "abcd"
            input.cursorPosition = 3

            acSource.isActive = true
            compare(acSource.isCancelled, false)
            compare(completionRequestedSpy.count, 0)

            keyClick(Qt.Key_Backspace)

            // also 1 completion request is generated
            compare(completionRequestedSpy.count, 1)
            compare(acSource.isCancelled, false)
        }
    }
}
