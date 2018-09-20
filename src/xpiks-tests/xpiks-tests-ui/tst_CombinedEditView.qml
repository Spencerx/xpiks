import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/StackViews"
import "../../xpiks-qt/Common.js" as Common
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    property string path: ''
    property bool isselected: false

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

        sourceComponent: CombinedEditView {
            componentParent: root
            anchors.fill: parent
            wasLeftSideCollapsed: false
        }
    }

    Loader {
        asynchronous: true
        focus: false

        sourceComponent: Item {
            UICommandListener {
                commandDispatcher: dispatcher
                commandIDs: [ UICommand.FixSpellingCombined ]
                onDispatched: {
                    Common.launchDialog("Dialogs/SpellCheckSuggestionsDialog.qml",
                                        root,
                                        {})
                }
            }
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
            titleInput = findChild(combinedView, "titleTextInput")
            descriptionInput = findChild(combinedView, "descriptionTextInput")
            editableTags = findChild(combinedView, "editableTags")
            descriptionCheckBox = findChild(combinedView, "descriptionCheckBox")
            keywordsEdit = findChild(combinedView, "nextTagTextInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestUtils.clearEdit(keywordsEdit)
            TestsHost.cleanup()
        }

        function test_TabTopToBottom() {
            combinedView.combinedArtworks.changeTitle = true
            combinedView.combinedArtworks.changeDescription = true
            combinedView.combinedArtworks.changeKeywords = true

            titleInput.forceActiveFocus()
            wait(1000)
            keyClick(Qt.Key_Tab)
            verify(descriptionInput.activeFocus)
            wait(1000)
            keyClick(Qt.Key_Tab)
            verify(editableTags.isFocused)
        }

        function test_TabOverTitle() {
            combinedView.combinedArtworks.changeTitle = true
            combinedView.combinedArtworks.changeDescription = false
            combinedView.combinedArtworks.changeKeywords = true

            titleInput.forceActiveFocus()
            keyClick(Qt.Key_Tab)
            verify(!(descriptionInput.activeFocus))
            verify(editableTags.isFocused)
        }

        function test_TabFromKeywords() {
            combinedView.combinedArtworks.changeTitle = true
            combinedView.combinedArtworks.changeDescription = false
            combinedView.combinedArtworks.changeKeywords = true

            editableTags.activateEdit()
            keyClick(Qt.Key_Backtab)
            verify(!(descriptionInput.activeFocus))
            verify(titleInput.activeFocus)
        }

        function test_addKeywordByTyping() {
            compare(combinedView.combinedArtworks.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(combinedView.combinedArtworks.keywordsCount, 1)
            compare(combinedView.combinedArtworks.getKeywordsString(), testKeyword)
        }

        function test_autoCompleteKeywordBasic() {
            verify(typeof combinedView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_W)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_A)

            wait(TestsHost.smallSleepTime)

            verify(typeof combinedView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_E)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.normalSleepTime)

            compare(combinedView.combinedArtworks.keywordsCount, 1)
            compare(combinedView.combinedArtworks.getKeywordsString(), "weather")
        }

        function test_autoCompleteCancelWhenShortText() {
            verify(typeof combinedView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_U)

            wait(TestsHost.smallSleepTime)

            verify(typeof combinedView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Backspace)

            wait(TestsHost.normalSleepTime)

            verify(typeof combinedView.autoCompleteBox === "undefined")
        }

        function test_autoCompletePreset() {
            verify(typeof combinedView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_F)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)

            wait(TestsHost.smallSleepTime)

            verify(typeof combinedView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return, Qt.ControlModifier)
            wait(TestsHost.normalSleepTime)
            verify(typeof combinedView.autoCompleteBox === "undefined")

            compare(combinedView.combinedArtworks.keywordsCount, 3)
            compare(combinedView.combinedArtworks.getKeywordsString(), "some, other, keywords")
        }

        function test_autoCompleteIntoNonEmptyEdit() {
            verify(typeof combinedView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_Space)

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            compare(keywordsEdit.text, "in space")
        }

        function test_spellingIsCheckedForWrong() {
            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.normalSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasSpellCheckError", true, 2000)
        }

        function test_spellingIsCheckedForCorrect() {
            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_Space)

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_Comma)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            wait(1500)

            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_duplicateIsDetectedWithDescription() {
            descriptionInput.forceActiveFocus()
            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_E)

            keywordsEdit.forceActiveFocus()
            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_E)

            keyClick(Qt.Key_Comma)

            wait(TestsHost.normalSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasDuplicate", true, 2000)
            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_doubleClickEditsKeyword() {
            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.normalSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            compare(combinedView.combinedArtworks.getKeywordsString(), testKeyword1)
            mouseDoubleClick(keywordWrapper)

            wait(TestsHost.smallSleepTime)

            for (var i = 0; i < testKeyword1.length - 1; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestsHost.smallSleepTime)

            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Enter)

            wait(TestsHost.smallSleepTime)

            compare(combinedView.combinedArtworks.getKeywordsString(), testKeyword1[0] + testKeyword2)
        }

        function test_editInPlainText() {
            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.normalSleepTime)

            combinedView.editInPlainText()

            wait(TestsHost.normalSleepTime)

            // hack to detect if plain text edit hasn't started
            keyClick(Qt.Key_Comma)
            keyClick(Qt.Key_Backspace)

            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Enter, Qt.ControlModifier)

            wait(TestsHost.smallSleepTime)

            compare(combinedView.combinedArtworks.getKeywordsString(), testKeyword1 + ", " + testKeyword2)
        }

        function test_copyToQuickBuffer() {
            titleInput.forceActiveFocus()
            TestUtils.keyboardEnterSomething(testCase)

            descriptionInput.forceActiveFocus()
            TestUtils.keyboardEnterSomething(testCase)

            keywordsEdit.forceActiveFocus()
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            var qbButton = findChild(combinedView, "copyToQuickBufferButton")
            mouseClick(qbButton)
            wait(TestsHost.smallSleepTime)

            compare(quickBuffer.title, combinedView.combinedArtworks.title)
            compare(quickBuffer.description, combinedView.combinedArtworks.description)
            compare(quickBuffer.keywordsCount, combinedView.combinedArtworks.keywordsCount)
        }

        function test_fixSpelling() {
            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_P)

            keyClick(Qt.Key_Comma)
            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasSpellCheckError", true, 2000)
            // wait for finding suggestions
            wait(TestsHost.normalSleepTime)

            var fixSpellingLink = findChild(combinedView, "fixSpellingLink")
            tryCompare(fixSpellingLink, "canBeShown", true, 1000)

            mouseClick(fixSpellingLink)
            wait(TestsHost.normalSleepTime)

            var spellSuggestor = dispatcher.getCommandTarget(UICommand.FixSpellingArtwork)
            spellSuggestor.selectSomething()
            spellSuggestor.submitCorrections()

            // hack to detect if the dialog wasn't shown
            keyClick(Qt.Key_Backspace)

            // close the dialog
            keyClick(Qt.Key_Escape)

            compare(combinedView.combinedArtworks.getKeywordsString(), "pet")
        }
    }
}
