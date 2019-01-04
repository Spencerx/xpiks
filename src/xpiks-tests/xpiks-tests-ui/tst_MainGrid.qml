import QtQuick 2.2
import QtTest 1.1
import XpiksTests 1.0
import xpiks 1.0
import "../../xpiks-qt/StackViews"
import "../../xpiks-qt/Common.js" as Common
import "TestUtils.js" as TestUtils

Item {
    id: root
    width: 1200
    height: 800
    property int openedDialogsCount: 0

    function onDialogClosed() {
        openedDialogsCount -= 1
    }

    Component.onCompleted: TestsHost.setup(testCase.name)

    QtObject {
        id: appHost
        property bool leftSideCollapsed: false
        property bool listLayout: true
        property bool areActionsAllowed: true
        function collapseLeftPane() { }
    }

    QtObject {
        id: applicationWindow
        property int openedDialogsCount: 0
    }

    QtObject {
        id: removeAction
        property bool enabled: true
        function trigger() {
            dispatcher.dispatch(UICommand.RemoveSelected, {})
        }
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

    Loader {
        id: listenersLoader
        asynchronous: true
        focus: false

        sourceComponent: Item {
            UICommandListener {
                commandDispatcher: dispatcher
                commandIDs: [ UICommand.ReviewSpellingArtwork ]
                onDispatched: {
                    Common.launchDialog("Dialogs/SpellCheckSuggestionsDialog.qml",
                                        root,
                                        {})
                }
            }

            UICommandListener {
                commandDispatcher: dispatcher
                commandIDs: [ UICommand.InitSuggestionArtwork ]
                onDispatched: {
                    Common.launchDialog("Dialogs/KeywordsSuggestion.qml",
                                        root,
                                        {callbackObject: value.callbackObject});
                }
            }

            UICommandListener {
                commandDispatcher: dispatcher
                commandIDs: [UICommand.SetupFindInArtworks]
                onDispatched: {
                    Common.launchDialog("Dialogs/FindAndReplace.qml",
                                        root,
                                        { componentParent: root })
                }
            }
        }
    }

    TestCase {
        id: testCase
        name: "MainGrid"
        when: windowShown && (loader.status == Loader.Ready) && TestsHost.isReady && (listenersLoader.status == Loader.Ready)
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

            wait(TestsHost.normalSleepTime)
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
            filteredArtworksListModel.searchTerm = ''
        }

        function cleanup() {
            TestsHost.cleanup()
        }

        function getDelegate(index) {
            var artworkDelegate = TestUtils.getDelegateInstanceAt(artworksHost.contentItem,
                                                                  "artworkDelegate",
                                                                  index)
            artworksHost.positionViewAtIndex(index, GridView.Contain)
            return artworkDelegate
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

            TestUtils.keyboardEnterText('wea')

            wait(TestsHost.smallSleepTime)

            verify(typeof workflowHost.autoCompleteBox !== "undefined")

            TestUtils.keyboardEnterText('the')

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

            TestUtils.keyboardEnterText('tru')

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

            TestUtils.keyboardEnterText('interfac')

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

            TestUtils.keyboardEnterText('in')
            keyClick(Qt.Key_Space)

            TestUtils.keyboardEnterText('space')
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
            TestUtils.keyboardEnterText('space')

            keywordsEdit.forceActiveFocus()
            TestUtils.keyboardEnterText('space')

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

            compare(root.openedDialogsCount, 0)
            mainGrid.editInPlainText(0)
            wait(TestsHost.normalSleepTime)
            compare(root.openedDialogsCount, 1)

            // hack to detect if plain text edit hasn't started
            keyClick(Qt.Key_Comma)
            keyClick(Qt.Key_Backspace)

            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Enter, Qt.ControlModifier)
            compare(root.openedDialogsCount, 0)

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

            TestUtils.keyboardEnterText('tep')

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

            compare(root.openedDialogsCount, 0)
            mouseClick(fixSpellingLink)
            wait(TestsHost.normalSleepTime)
            compare(root.openedDialogsCount, 1)

            var spellSuggestor = dispatcher.getCommandTarget(UICommand.ReviewSpellingArtwork)
            spellSuggestor.selectSomething()
            wait(TestsHost.smallSleepTime)

            var replaceButton = findChild(root, "replaceButton")
            verify(replaceButton.enabled)
            mouseClick(replaceButton)
            wait(TestsHost.smallSleepTime)
            compare(root.openedDialogsCount, 0)

            compare(artworkDelegate.delegateModel.keywordsstring, "pet")
        }

        function test_filterText() {
            var originalFilterText = filteredArtworksListModel.searchTerm
            verify(artworksHost.count > 1)

            var artworkDelegate = getDelegate(0)
            var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
            TestUtils.clearEdit(keywordsEdit)
            keywordsEdit.forceActiveFocus()

            var keyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            var filterTextInput = findChild(mainGrid, "filterTextInput")
            filterTextInput.text = filterTextInput.text + ' ' + keyword

            var searchButton = findChild(mainGrid, "searchButton")
            mouseClick(searchButton)

            wait(TestsHost.normalSleepTime)

            compare(artworksHost.count, 1)

            filteredArtworksListModel.searchTerm = originalFilterText
            wait(TestsHost.normalSleepTime)
        }

        function test_undoRemove() {
            var artworkDelegate = getDelegate(0)
            var checkbox = findChild(artworkDelegate, "itemCheckedCheckbox")
            mouseClick(checkbox)

            var removeToolButton = findChild(mainGrid, "removeToolButton")
            mouseClick(removeToolButton)
            wait(TestsHost.smallSleepTime)

            verify(artworksHost.count, 4)

            var undoLink = findChild(mainGrid, "undoLink")
            mouseClick(undoLink)
            wait(TestsHost.smallSleepTime)

            compare(artworksHost.count, 5)
        }

        function test_addToUserDictionary() {
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

            dispatcher.dispatch(UICommand.AddToUserDictionary, testKeyword)

            tryCompare(keywordWrapper, "hasSpellCheckError", false, 2000)
        }

        function test_suggestLocalKeywords() {
            var artworkDelegate = getDelegate(2)
            mainGrid.suggestKeywords(2)

            wait(TestsHost.normalSleepTime)

            var suggestKeywordsDialog = findChild(root, "keywordsSuggestionComponent")
            var searchInput = findChild(suggestKeywordsDialog, "queryTextInput")

            // setupSearch() from tst_KeywordsSuggestion.qml
            searchInput.forceActiveFocus()
            TestUtils.keyboardEnterText('vector')

            suggestKeywordsDialog.keywordsSuggestor.selectedSourceIndex = 3
            // --

            var searchButton = findChild(suggestKeywordsDialog, "searchButton")
            var suggestionsRepeater = findChild(suggestKeywordsDialog, "suggestionsRepeater")

            mouseClick(searchButton)
            wait(TestsHost.smallSleepTime)
            tryCompare(suggestionsRepeater, "count", 3, 3000)

            mouseClick(suggestionsRepeater.itemAt(0))
            mouseClick(suggestionsRepeater.itemAt(1))
            mouseClick(suggestionsRepeater.itemAt(2))
            wait(TestsHost.normalSleepTime)

            tryCompare(suggestKeywordsDialog.keywordsSuggestor, "suggestedKeywordsCount", 3, 2000)
            compare(suggestKeywordsDialog.keywordsSuggestor.getSuggestedKeywords().sort(),
                    ["graphic", "line", "vector"])

            var addSuggestedButton = findChild(suggestKeywordsDialog, "addSuggestedButton")
            mouseClick(addSuggestedButton)

            var closeButton = findChild(suggestKeywordsDialog, "closeButton")
            mouseClick(closeButton)

            wait(TestsHost.normalSleepTime)

            var keywordsString = artworkDelegate.delegateModel.keywordsstring
            verify(keywordsString.indexOf("graphic", 0) !== -1)
            verify(keywordsString.indexOf("line", 0) !== -1)
            verify(keywordsString.indexOf("vector", 0) !== -1)
            verify(keywordsString.indexOf("xpiks", 0) === -1)
        }

        function test_findAndReplace() {
            var startIndex = 1
            var count = 4
            var notIndex = 2;

            for (var i = 0; i < count; i++) {
                var artworkDelegate = getDelegate(i + startIndex)
                var descriptionInput = findChild(artworkDelegate, "descriptionTextInput")
                descriptionInput.forceActiveFocus()
                keyClick(Qt.Key_B)
                keyClick(Qt.Key_O)
                keyClick(Qt.Key_B)
                keyClick(Qt.Key_Tab)
            }

            wait(TestsHost.normalSleepTime)

            dispatcher.dispatch(UICommand.SetupFindInArtworks, {})
            wait(TestsHost.smallSleepTime)

            TestUtils.keyboardEnterText('bob')

            keyClick(Qt.Key_Tab)

            TestUtils.keyboardEnterText('cat')

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            var replacePreviewList = findChild(root, "replacePreviewList")
            compare(replacePreviewList.count, count)

            var replaceDelegate = TestUtils.getDelegateInstanceAt(replacePreviewList.contentItem,
                                                                  "imageDelegate",
                                                                  notIndex)
            replacePreviewList.positionViewAtIndex(notIndex, ListView.Contain)
            var checkbox = findChild(replaceDelegate, "applyReplaceCheckBox")
            mouseClick(checkbox)
            wait(TestsHost.smallSleepTime)

            var replaceButton = findChild(root, "replaceButton")
            mouseClick(replaceButton)
            wait(TestsHost.normalSleepTime)

            for (var i = 0; i < count; i++) {
                var artworkDelegate = getDelegate(i + startIndex)
                var descriptionInput = findChild(artworkDelegate, "descriptionTextInput")
                if (i != notIndex) {
                    tryCompare(descriptionInput, "text", "cat", 1000)
                } else {
                    tryCompare(descriptionInput, "text", "bob", 1000)
                }
            }
        }

        function test_findAndReplaceChangeSearch() {
            var startIndex = 1
            var count = 4
            var notIndex = 2;

            for (var i = 0; i < count; i++) {
                var artworkDelegate = getDelegate(i + startIndex)
                if (i % 2 == 0) {
                    var descriptionInput = findChild(artworkDelegate, "descriptionTextInput")
                    descriptionInput.forceActiveFocus()
                } else {
                    var keywordsEdit = findChild(artworkDelegate, "nextTagTextInput")
                    TestUtils.clearEdit(keywordsEdit)
                    keywordsEdit.forceActiveFocus()
                }

                TestUtils.keyboardEnterText('bob')
                keyClick(Qt.Key_Tab)
            }

            wait(TestsHost.normalSleepTime)

            dispatcher.dispatch(UICommand.SetupFindInArtworks, {})
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_B)
            keyClick(Qt.Key_O)
            keyClick(Qt.Key_B)

            keyClick(Qt.Key_Tab)

            keyClick(Qt.Key_C)
            keyClick(Qt.Key_A)
            keyClick(Qt.Key_T)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            var replacePreviewList = findChild(root, "replacePreviewList")
            compare(replacePreviewList.count, count)

            var cancelButton = findChild(root, "cancelReplaceButton")
            mouseClick(cancelButton)
            wait(TestsHost.normalSleepTime)

            var checkDescriptionCheckbox = findChild(root, "searchDescriptionCheckbox")
            mouseClick(checkDescriptionCheckbox)
            compare(checkDescriptionCheckbox.checked, false)

            var findButton = findChild(root, "findButton")
            mouseClick(findButton)
            wait(TestsHost.normalSleepTime)

            replacePreviewList = findChild(root, "replacePreviewList")
            compare(replacePreviewList.count, count/2)

            cancelButton = findChild(root, "cancelReplaceButton")
            mouseClick(cancelButton)
            wait(TestsHost.smallSleepTime)

            cancelButton = findChild(root, "cancelFindButton")
            mouseClick(cancelButton)
        }
    }
}
