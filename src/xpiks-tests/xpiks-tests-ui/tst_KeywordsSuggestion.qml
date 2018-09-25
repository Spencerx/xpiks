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

        sourceComponent: KeywordsSuggestion {
            anchors.fill: parent
        }
    }

    ClipboardHelper {
        id: clipboard
    }

    TestCase {
        id: testCase
        name: "SuggestKeywords"
        when: windowShown && (loader.status == Loader.Ready)
        property var suggestKeywordsDialog: loader.item
        property var searchInput
        property var searchButton
        property var suggestionsRepeater

        function initTestCase() {
            searchInput = findChild(suggestKeywordsDialog, "queryTextInput")
            searchButton = findChild(suggestKeywordsDialog, "searchButton")
            suggestionsRepeater = findChild(suggestKeywordsDialog, "suggestionsRepeater")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestsHost.cleanup()
            TestUtils.clearEdit(searchInput)
        }

        function setupSearch() {
            searchInput.forceActiveFocus()
            keyClick(Qt.Key_V)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_C)
            keyClick(Qt.Key_T)
            keyClick(Qt.Key_O)
            keyClick(Qt.Key_R)

            suggestKeywordsDialog.keywordsSuggestor.selectedSourceIndex = 3
        }

        function test_searchLocalArtworks() {
            setupSearch()
            mouseClick(searchButton)
            tryCompare(suggestionsRepeater, "count", 3, 3000)
        }

        function test_combineSearchResults() {
            setupSearch()
            mouseClick(searchButton)
            tryCompare(suggestionsRepeater, "count", 3, 3000)

            mouseClick(suggestionsRepeater.itemAt(0))
            mouseClick(suggestionsRepeater.itemAt(1))
            mouseClick(suggestionsRepeater.itemAt(2))
            wait(TestsHost.normalSleepTime)

            compare(suggestKeywordsDialog.keywordsSuggestor.suggestedKeywordsCount, 3)
            compare(suggestKeywordsDialog.keywordsSuggestor.getSuggestedKeywords().sort(),
                    ["graphic", "line", "vector"])
        }

        function test_removeCommonAppendsToOther() {
            setupSearch()
            mouseClick(searchButton)
            tryCompare(suggestionsRepeater, "count", 3, 3000)

            mouseClick(suggestionsRepeater.itemAt(0))
            wait(TestsHost.smallSleepTime)

            var suggestedKeywordsCount = suggestKeywordsDialog.keywordsSuggestor.suggestedKeywordsCount
            var otherKeywordsCount = suggestKeywordsDialog.keywordsSuggestor.otherKeywordsCount

            var suggestedEditableTags = findChild(suggestKeywordsDialog, "suggestedEditableTags")
            var repeater = findChild(suggestedEditableTags, "repeater")
            var keyword0 = repeater.itemAt(0)
            mouseClick(keyword0, keyword0.width - keyword0.height/2, keyword0.height/2)
            wait(TestsHost.smallSleepTime)

            compare(suggestKeywordsDialog.keywordsSuggestor.suggestedKeywordsCount, suggestedKeywordsCount - 1)
            compare(suggestKeywordsDialog.keywordsSuggestor.otherKeywordsCount, otherKeywordsCount + 1)
        }

        function test_clearSuggestedAppendsToOther() {
            setupSearch()
            mouseClick(searchButton)
            tryCompare(suggestionsRepeater, "count", 3, 3000)

            mouseClick(suggestionsRepeater.itemAt(0))
            wait(TestsHost.smallSleepTime)

            var suggestedKeywordsCount = suggestKeywordsDialog.keywordsSuggestor.suggestedKeywordsCount
            suggestKeywordsDialog.keywordsSuggestor.clearSuggested()
            wait(TestsHost.smallSleepTime)

            compare(suggestKeywordsDialog.keywordsSuggestor.suggestedKeywordsCount, 0)
            compare(suggestKeywordsDialog.keywordsSuggestor.otherKeywordsCount, suggestedKeywordsCount)
        }

        function test_removeOtherAppendsToSuggested() {
            setupSearch()
            mouseClick(searchButton)
            tryCompare(suggestionsRepeater, "count", 3, 3000)

            mouseClick(suggestionsRepeater.itemAt(0))
            wait(TestsHost.smallSleepTime)

            suggestKeywordsDialog.keywordsSuggestor.clearSuggested()
            wait(TestsHost.smallSleepTime)

            var suggestedKeywordsCount = suggestKeywordsDialog.keywordsSuggestor.suggestedKeywordsCount
            var otherKeywordsCount = suggestKeywordsDialog.keywordsSuggestor.otherKeywordsCount

            var otherEditableTags = findChild(suggestKeywordsDialog, "otherEditableTags")
            var repeater = findChild(otherEditableTags, "repeater")
            var keyword0 = repeater.itemAt(0)
            mouseClick(keyword0, keyword0.width - keyword0.height/2, keyword0.height/2)
            wait(TestsHost.smallSleepTime)

            compare(suggestKeywordsDialog.keywordsSuggestor.suggestedKeywordsCount, suggestedKeywordsCount + 1)
            compare(suggestKeywordsDialog.keywordsSuggestor.otherKeywordsCount, otherKeywordsCount - 1)
        }
    }
}
