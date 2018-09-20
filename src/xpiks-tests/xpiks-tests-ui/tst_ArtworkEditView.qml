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
        id: keywordsWrapper
        property bool keywordsModel: false
    }

    QtObject {
        id: appHost
        property bool leftSideCollapsed: false
    }

    QtObject {
        id: mainStackView
        function push(obj) { }
    }

    Loader {
        id: loader
        anchors.fill: parent
        asynchronous: true
        focus: true

        sourceComponent: ArtworkEditView {
            anchors.fill: parent
            componentParent: root
        }
    }

    Loader {
        asynchronous: true
        focus: false

        sourceComponent: Item {
            UICommandListener {
                commandDispatcher: dispatcher
                commandIDs: [ UICommand.FixSpellingSingle ]
                onDispatched: {
                    Common.launchDialog("Dialogs/SpellCheckSuggestionsDialog.qml",
                                        root,
                                        {})
                }
            }
        }
    }

    ClipboardHelper {
        id: clipboard
    }

    TestCase {
        id: testCase
        name: "ArtworkEdit"
        when: windowShown && (loader.status == Loader.Ready)
        property var titleEdit
        property var descriptionEdit
        property var keywordsEdit
        property var nextArtworkButton
        property var prevArtworkButton
        property var rosterListView
        property var copyLink
        property var editableTags
        property var artworkEditView: loader.item

        function initTestCase() {
            titleEdit = findChild(artworkEditView, "titleTextInput")
            descriptionEdit = findChild(artworkEditView, "descriptionTextInput")
            keywordsEdit = findChild(artworkEditView, "nextTagTextInput")

            nextArtworkButton = findChild(artworkEditView, "selectNextButton")
            prevArtworkButton = findChild(artworkEditView, "selectPrevButton")

            rosterListView = findChild(artworkEditView, "rosterListView")

            copyLink = findChild(artworkEditView, "copyLink")

            editableTags = findChild(artworkEditView, "editableTags")
        }

        function cleanupTestCase() {
            TestsHost.cleanupTest()
        }

        function cleanup() {
            TestUtils.clearEdit(keywordsEdit)
            TestsHost.cleanup()
        }

        function test_editTitleSimple() {
            titleEdit.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            compare(artworkEditView.artworkProxy.title, testTitle)
        }

        function test_editDescriptionSimple() {
            descriptionEdit.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)
            compare(artworkEditView.artworkProxy.description, testDescription)
        }

        function test_addKeywordsSmoke() {
            compare(artworkEditView.artworkProxy.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(artworkEditView.artworkProxy.keywordsCount, 1)
            compare(artworkEditView.artworkProxy.getKeywordsString(), testKeyword)
        }

        function test_switchCurrentArtworkButtons() {
            titleEdit.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            descriptionEdit.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)

            compare(rosterListView.currentIndex, 0)
            compare(artworkEditView.artworkProxy.description, testDescription)
            compare(artworkEditView.artworkProxy.title, testTitle)

            mouseClick(nextArtworkButton)
            wait(TestsHost.smallSleepTime)

            compare(rosterListView.currentIndex, 1)
            console.log(artworkEditView.artworkProxy.description)
            verify(artworkEditView.artworkProxy.description !== testDescription)
            verify(artworkEditView.artworkProxy.title !== testTitle)

            mouseClick(prevArtworkButton)
            wait(TestsHost.smallSleepTime)

            compare(rosterListView.currentIndex, 0)
            compare(artworkEditView.artworkProxy.description, testDescription)
            compare(artworkEditView.artworkProxy.title, testTitle)
        }

        function test_switchCurrentArtworkMouseClick() {
            titleEdit.forceActiveFocus()
            var testTitle = TestUtils.keyboardEnterSomething(testCase)
            descriptionEdit.forceActiveFocus()
            var testDescription = TestUtils.keyboardEnterSomething(testCase)

            compare(rosterListView.currentIndex, 0)
            compare(artworkEditView.artworkProxy.description, testDescription)
            compare(artworkEditView.artworkProxy.title, testTitle)

            var nextItem = TestUtils.getDelegateInstanceAt(rosterListView.contentItem,
                                                           "rosterDelegateItem",
                                                           rosterListView.currentIndex + 1)
            mouseClick(nextItem)
            wait(TestsHost.smallSleepTime)

            compare(rosterListView.currentIndex, 1)
            console.log(artworkEditView.artworkProxy.description)
            verify(artworkEditView.artworkProxy.description !== testDescription)
            verify(artworkEditView.artworkProxy.title !== testTitle)

            var prevItem = TestUtils.getDelegateInstanceAt(rosterListView.contentItem,
                                                           "rosterDelegateItem",
                                                           rosterListView.currentIndex - 1)
            mouseClick(prevItem)
            wait(TestsHost.smallSleepTime)

            compare(rosterListView.currentIndex, 0)
            compare(artworkEditView.artworkProxy.description, testDescription)
            compare(artworkEditView.artworkProxy.title, testTitle)
        }

        function test_copyFromMoreLink() {
            compare(copyLink.enabled, false)

            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            // do not press comma - test adding keyword on losing focus

            compare(copyLink.enabled, true)
            copyLink.forceActiveFocus()

            mouseClick(copyLink)

            compare(clipboard.getText(), testKeyword1 + ", " + testKeyword2)
        }

        function test_autoCompleteKeywordBasic() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_W)
            keyClick(Qt.Key_E)
            keyClick(Qt.Key_A)

            wait(TestsHost.smallSleepTime)

            verify(typeof artworkEditView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_H)
            keyClick(Qt.Key_E)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return)
            wait(TestsHost.smallSleepTime)

            compare(artworkEditView.artworkProxy.keywordsCount, 1)
            compare(artworkEditView.artworkProxy.getKeywordsString(), "weather")
        }

        function test_autoCompleteCancelWhenShortText() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

            keywordsEdit.forceActiveFocus()

            keyClick(Qt.Key_T)
            keyClick(Qt.Key_R)
            keyClick(Qt.Key_U)

            wait(TestsHost.smallSleepTime)

            verify(typeof artworkEditView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Backspace)

            wait(TestsHost.smallSleepTime)

            verify(typeof artworkEditView.autoCompleteBox === "undefined")
        }

        function test_autoCompletePreset() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

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

            verify(typeof artworkEditView.autoCompleteBox !== "undefined")

            keyClick(Qt.Key_Down)
            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Return, Qt.ControlModifier)
            wait(TestsHost.smallSleepTime)

            compare(artworkEditView.artworkProxy.keywordsCount, 3)
            compare(artworkEditView.artworkProxy.getKeywordsString(), "some, other, keywords")
        }

        function test_autoCompleteIntoNonEmptyEdit() {
            verify(typeof artworkEditView.autoCompleteBox === "undefined")

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

            wait(TestsHost.smallSleepTime)

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

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            wait(1500)

            compare(keywordWrapper.hasSpellCheckError, false)
        }

        function test_duplicateIsDetectedWithDescription() {
            descriptionEdit.forceActiveFocus()
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
            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.smallSleepTime)

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            compare(artworkEditView.artworkProxy.getKeywordsString(), testKeyword1)
            mouseDoubleClick(keywordWrapper)

            wait(TestsHost.smallSleepTime)

            for (var i = 0; i < testKeyword1.length - 1; i++) {
                keyClick(Qt.Key_Backspace)
            }

            wait(TestsHost.smallSleepTime)

            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Enter)

            wait(TestsHost.smallSleepTime)

            compare(artworkEditView.artworkProxy.getKeywordsString(), testKeyword1[0] + testKeyword2)
        }

        function test_editInPlainText() {
            keywordsEdit.forceActiveFocus()
            var testKeyword1 = TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            wait(TestsHost.normalSleepTime)

            artworkEditView.editInPlainText()

            wait(TestsHost.normalSleepTime)

            // hack to detect if plain text edit hasn't started
            keyClick(Qt.Key_Comma)
            keyClick(Qt.Key_Backspace)

            keyClick(Qt.Key_Comma)
            var testKeyword2 = TestUtils.keyboardEnterSomething(testCase)

            wait(TestsHost.smallSleepTime)

            keyClick(Qt.Key_Enter, Qt.ControlModifier)

            wait(TestsHost.smallSleepTime)

            compare(artworkEditView.artworkProxy.getKeywordsString(), testKeyword1 + ", " + testKeyword2)
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

            var fixSpellingLink = findChild(artworkEditView, "fixSpellingLink")
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

            compare(artworkEditView.artworkProxy.getKeywordsString(), "pet")
        }

        function test_clearKeywords() {
            compare(artworkEditView.artworkProxy.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)
            TestUtils.keyboardEnterSomething(testCase)
            keyClick(Qt.Key_Comma)

            compare(artworkEditView.artworkProxy.keywordsCount, 2)

            artworkEditView.clearKeywords()
            compare(artworkEditView.artworkProxy.keywordsCount, 0)
            compare(artworkEditView.artworkProxy.getKeywordsString(), "")
        }
    }
}
