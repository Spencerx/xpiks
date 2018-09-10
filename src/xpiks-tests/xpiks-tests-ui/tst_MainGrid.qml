import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import xpiks 1.0
import "../../xpiks-qt/StackViews"
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: TestsHost.setup()

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
        property bool listLayout: true
        property bool actionsEnabled: true
        function collapseLeftPane() { }
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: MainGrid {
            componentParent: root
        }
    }

    TestCase {
        id: testCase
        name: "MainGrid"
        when: windowShown && (loader.status == Loader.Ready)
        property var mainGrid: loader.item
        property var artworksHost
        property var workflowHost

        function initTestCase() {
            artworksHost = findChild(mainGrid, "artworksHost")
            workflowHost = findChild(mainGrid, "workflowHost")

            var columnLayout = findChild(mainGrid, "columnLayout")
            columnLayout.isWideEnough = true

            filteredArtworksListModel.unselectFilteredArtworks()
            // 2nd half of the images
            filteredArtworksListModel.searchTerm = "x:image"
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
        }

        function getDelegate(index) {
            return TestUtils.getDelegateInstanceAt(artworksHost.contentItem,
                                                   "artworkDelegate",
                                                   index)
        }

        function test_addKeywordBasic() {
            var artworkDelegate = getDelegate(1)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            compare(artworkDelegate.delegateModel.keywordsstring, testKeyword)
        }

        function test_editDescription() {
            var artworkDelegate = getDelegate(1)
            var descriptionInput = findChild(artworkDelegate, "descriptionTextInput")
            descriptionInput.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)
            wait(TestsHost.smallSleepTime)
            compare(artworkDelegate.delegateModel.description, testDescription)
        }

        function test_editTitle() {
            var artworkDelegate = getDelegate(1)
            var titleInput = findChild(artworkDelegate, "titleTextInput")
            titleInput.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            wait(TestsHost.smallSleepTime)
            compare(artworkDelegate.delegateModel.title, testTitle)
        }

        function test_autoCompleteKeywordBasic() {
            verify(typeof workflowHost.autoCompleteBox === "undefined")

            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_W)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_A)

            wait(TestsHost.smallSleepTime)

            verify(typeof workflowHost.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_E)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            compare(artworkDelegate.delegateModel.keywordscount, 1)
            compare(artworkDelegate.delegateModel.keywordsstring, "weather")
        }

        function test_autoCompleteCancelWhenShortText() {
            verify(typeof workflowHost.autoCompleteBox === "undefined")

            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_U)

            wait(TestsHost.smallSleepTime)

            verify(typeof workflowHost.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Backspace)

            wait(TestsHost.smallSleepTime)

            verify(typeof workflowHost.autoCompleteBox === "undefined")
        }

        function test_autoCompletePreset() {
            verify(typeof workflowHost.autoCompleteBox === "undefined")

            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
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

            verify(typeof workflowHost.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return, Qt.ControlModifier)
            wait(TestsHost.smallSleepTime)

            compare(artworkDelegate.delegateModel.keywordscount, 3)
            compare(artworkDelegate.delegateModel.keywordsstring, "some, other, keywords")
        }

        function test_autoCompleteIntoNonEmptyEdit() {
            verify(typeof workflowHost.autoCompleteBox === "undefined")

            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
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
            var artworkDelegate = getDelegate(1)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            var editableTags = findChild(artworkDelegate, "editableTags")

            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasSpellCheckError", true, 2000)
        }

        function test_spellingIsCheckedForCorrect() {
            var artworkDelegate = getDelegate(1)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            var editableTags = findChild(artworkDelegate, "editableTags")

            keyClick(Qt.Key_I)
            keyClick(Qt.Key_N)
            keyClick(Qt.Key_Space)

            keyClick(Qt.Key_S)
            keyClick(Qt.Key_P)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            wait(1500)

            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_duplicateIsDetectedWithDescription() {
            var artworkDelegate = getDelegate(1)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)

            var editableTags = findChild(artworkDelegate, "editableTags")

            var descriptionInput = findChild(artworkDelegate, "descriptionTextInput")
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

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasDuplicate", true, 2000)
            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_doubleClickEditsKeyword() {
            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            var editableTags = findChild(artworkDelegate, "editableTags")

            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            compare(artworkDelegate.delegateModel.keywordsstring, testKeyword1)
            mouseDoubleClick(keywordWrapper)

            wait(TestsHost.normalSleepTime)

            for (var i = 0; i < testKeyword1.length - 1; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestsHost.normalSleepTime)

            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Enter)

            wait(TestsHost.normalSleepTime)

            compare(artworkDelegate.delegateModel.keywordsstring, testKeyword1[0] + testKeyword2)
        }

        function test_editInPlainText() {
            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.normalSleepTime)

            mainGrid.editInPlainText(0)

            wait(TestsHost.normalSleepTime)

            // hack to detect if plain text edit hasn't started
            keyClick(Qt.Key_Comma)
            keyClick(Qt.Key_Backspace)

            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Enter, Qt.ControlModifier)

            wait(TestsHost.smallSleepTime)

            compare(artworkDelegate.delegateModel.keywordsstring, testKeyword1 + ", " + testKeyword2)
        }

        function test_tabToNextItem() {
            var delegate1 = getDelegate(0)

            var descriptionInput1 = findChild(delegate1, "descriptionTextInput")
            descriptionInput1.forceActiveFocus()

            // tab to title
            keyClick(Qt.Key_Tab)
            // tab to keywords
            keyClick(Qt.Key_Tab)
            // tab to next delegate
            keyClick(Qt.Key_Tab)

            var delegate2 = getDelegate(1)

            var descriptionInput2 = findChild(delegate2, "descriptionTextInput")
            verify(descriptionInput2.activeFocus)
        }

        function test_tabToPreviousItem() {
            var delegate2 = getDelegate(1)

            var keywordsInput2 = findChild(delegate2, "nextTagTextInput")
            keywordsInput2.forceActiveFocus()

            // tab to title
            keyClick(Qt.Key_Backtab)
            // tab to description
            keyClick(Qt.Key_Backtab)
            // tab to prev delegate
            keyClick(Qt.Key_Backtab)

            var delegate1 = getDelegate(0)

            var keywordsInput1 = findChild(delegate1, "nextTagTextInput")
            verify(keywordsInput1.activeFocus)
        }

        function test_fixSpelling() {
            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_P)

            keyClick(Qt.Key_Comma)
            wait(TestsHost.smallSleepTime)

            var editableTags = findChild(artworkDelegate, "editableTags")
            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            tryCompare(keywordWrapper, "hasSpellCheckError", true, 2000)
            // wait for finding suggestions
            wait(TestsHost.normalSleepTime)

            var fixSpellingLink = findChild(artworkDelegate, "fixSpellingLink")
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

            compare(artworkDelegate.delegateModel.keywordsstring, "pet")
        }
    }
}
