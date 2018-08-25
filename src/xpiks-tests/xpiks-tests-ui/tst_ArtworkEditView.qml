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
    property string scoreme: TestsHost.scoreme

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

    TestCase {
        name: "ArtworkEdit"
        when: windowShown
        property var titleEdit
        property var descriptionEdit
        property var keywordsEdit
        property var nextArtworkButton
        property var prevArtworkButton
        property var rosterListView

        function initTestCase() {
            TestsHost.setup()

            titleEdit = findChild(artworkEditView, "titleTextInput")
            descriptionEdit = findChild(artworkEditView, "descriptionTextInput")
            keywordsEdit = findChild(artworkEditView, "nextTagTextInput")

            nextArtworkButton = findChild(artworkEditView, "selectNextButton")
            prevArtworkButton = findChild(artworkEditView, "selectPrevButton")

            rosterListView = findChild(artworkEditView, "rosterListView")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function cleanup() {
            // assigning .text directly breaks binding
            titleEdit.selectAll()
            titleEdit.cut()
            descriptionEdit.selectAll()
            descriptionEdit.cut()
            artworkProxy.clearModel()
        }

        function getRandomInt(max) {
          return Math.floor(Math.random() * Math.floor(max));
        }

        function keyboardEnterSomething() {
            var keys = [Qt.Key_A, Qt.Key_B, Qt.Key_C, Qt.Key_D, Qt.Key_E, Qt.Key_0, Qt.Key_1]
            var values = ['a', 'b', 'c', 'd', 'e', '0', '1']
            var count = 1 + getRandomInt(keys.length - 1)
            var text = "";
            for (var i = 0; i < count; i++) {
                var keyIndex = getRandomInt(keys.length)
                keyClick(keys[keyIndex])
                text += values[keyIndex]
            }
            return text
        }

        function test_editTitleSmokeTest() {
            titleEdit.forceActiveFocus()
            var testTitle = keyboardEnterSomething()
            compare(artworkProxy.title, testTitle)
        }

        function test_editDescriptionSmokeTest() {
            descriptionEdit.forceActiveFocus()
            var testDescription = keyboardEnterSomething()
            compare(artworkProxy.description, testDescription)
        }

        function test_addKeywordsSmokeTest() {
            compare(artworkProxy.keywordsCount, 0)

            keywordsEdit.forceActiveFocus()
            var testKeyword = keyboardEnterSomething()
            keyClick(Qt.Key_Comma)

            compare(artworkProxy.keywordsCount, 1)
            compare(artworkProxy.getKeywordsString(), testKeyword)
        }

        function test_switchCurrentArtworkTest() {
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
    }
}
