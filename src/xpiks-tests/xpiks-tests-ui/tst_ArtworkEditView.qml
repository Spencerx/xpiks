import QtQuick 2.0
import QtTest 1.1
import xpiks 1.0
import XpiksTests 1.0
import "../../xpiks-qt/StackViews"
import "UiTestsStubPlugin"

Item {
    id: root
    width: 800
    height: 600

    property string path: ''
    property bool isselected: false
    property string testsHostMention: TestsHost.scoreme

    QtObject {
        id: keywordsWrapper
        property bool keywordsModel: false
    }

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    ArtworkEditView {
        id: artworkEditView
        anchors.fill: parent
        artworkIndex: 0
        keywordsModel: artworkProxy.getBasicModelObject()
    }

    ClipboardHelper {
        id: clipboard
    }

    TestCase {
        name: "ArtworkEdit"
        when: windowShown
        property var titleEdit
        property var descriptionEdit
        property var keywordsEdit
        property var nextArtworkButton
        property var prevArtworkButton
        property var rosterListView
        property var copyLink
        property var editableTags

        function initTestCase() {
            TestsHost.setup()

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
            TestsHost.cleanup()
        }

        function cleanup() {
            // assigning .text directly breaks binding
            artworkProxy.clearModel()
        }

        function getRandomInt(max) {
            return Math.floor(Math.random() * Math.floor(max));
        }

        function getDelegateInstanceAt(contentItem, delegateObjectName, index) {
            for(var i = 0; i < contentItem.children.length; ++i) {
                var item = contentItem.children[i];
                // We have to check for the specific objectName we gave our
                // delegates above, since we also get some items that are not
                // our delegates here.
                if (item.objectName == delegateObjectName && item.delegateIndex == index)
                    return item;
            }
            return undefined;
        }

        function keyboardEnterSomething() {
            var keys = [Qt.Key_A, Qt.Key_B, Qt.Key_C, Qt.Key_D, Qt.Key_E, Qt.Key_F, Qt.Key_0, Qt.Key_1]
            var values = ['a', 'b', 'c', 'd', 'e', 'f', '0', '1']
            var count = keys.length + getRandomInt(keys.length)
            var text = "";
            for (var i = 0; i < count; i++) {
                var keyIndex = getRandomInt(keys.length)
                keyClick(keys[keyIndex])
                text += values[keyIndex]
            }
            return text
        }

        function test_editTitleSimple() {
            titleEdit.forceActiveFocus()
            var testTitle = keyboardEnterSomething()
            compare(artworkProxy.title, testTitle)
        }

        function test_editDescriptionSimple() {
            descriptionEdit.forceActiveFocus()
            var testDescription = keyboardEnterSomething()
            compare(artworkProxy.description, testDescription)
        }

        function test_addKeywordsSmoke() {
            compare(artworkProxy.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = keyboardEnterSomething()
            keyClick(Qt.Key_Comma)

            compare(artworkProxy.keywordsCount, 1)
            compare(artworkProxy.getKeywordsString(), testKeyword)
        }

        function test_switchCurrentArtworkButtons() {
            titleEdit.forceActiveFocus()
            var testTitle = keyboardEnterSomething()
            descriptionEdit.forceActiveFocus()
            var testDescription = keyboardEnterSomething()

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)

            mouseClick(nextArtworkButton)

            compare(rosterListView.currentIndex, 1)
            console.log(artworkProxy.description)
            verify(artworkProxy.description !== testDescription)
            verify(artworkProxy.title !== testTitle)

            mouseClick(prevArtworkButton)

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)
        }

        function test_switchCurrentArtworkMouseClick() {
            titleEdit.forceActiveFocus()
            var testTitle = keyboardEnterSomething()
            descriptionEdit.forceActiveFocus()
            var testDescription = keyboardEnterSomething()

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)

            var nextItem = getDelegateInstanceAt(rosterListView.contentItem,
                                                 "rosterDelegateItem",
                                                 rosterListView.currentIndex + 1)
            mouseClick(nextItem)

            compare(rosterListView.currentIndex, 1)
            console.log(artworkProxy.description)
            verify(artworkProxy.description !== testDescription)
            verify(artworkProxy.title !== testTitle)

            var prevItem = getDelegateInstanceAt(rosterListView.contentItem,
                                                 "rosterDelegateItem",
                                                 rosterListView.currentIndex - 1)
            mouseClick(prevItem)

            compare(rosterListView.currentIndex, 0)
            compare(artworkProxy.description, testDescription)
            compare(artworkProxy.title, testTitle)
        }

        function test_copyFromMoreLink() {
            compare(copyLink.enabled, false)

            keywordsEdit.forceActiveFocus()
            var testKeyword1 = keyboardEnterSomething()
            keyClick(Qt.Key_Comma)
            var testKeyword2 = keyboardEnterSomething()
            // do not press comma - test adding keyword on losing focus

            compare(copyLink.enabled, true)
            copyLink.forceActiveFocus()

            mouseClick(copyLink)

            compare(clipboard.getText(), testKeyword1 + ", " + testKeyword2)
        }

        function test_doubleClickKeyword() {
            keywordsEdit.forceActiveFocus()
            var testKeyword = keyboardEnterSomething()

            var repeater = findChild(editableTags, "repeater")
            var keywordWrapper = repeater.itemAt(0)

            mouseDoubleClick(keywordWrapper)
        }
    }
}
