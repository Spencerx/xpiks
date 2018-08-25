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
    }

    TestCase {
        name: "ArtworkEdit"
        when: windowShown
        property var titleEdit
        property var descriptionEdit
        property var keywordsEdit

        function initTestCase() {
            TestsHost.setup()

            titleEdit = findChild(artworkEditView, "titleTextInput")
            descriptionEdit = findChild(artworkEditView, "descriptionTextInput")
            keywordsEdit = findChild(artworkEditView, "nextTagTextInput")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }

        function test_editTitleSmokeTest() {
            var testTitle = "test title here"
            titleEdit.forceActiveFocus()
            titleEdit.text = testTitle
            compare(artworkProxy.title, testTitle)
        }

        function test_editDescriptionSmokeTest() {
            var testDescription = "test description"
            descriptionEdit.forceActiveFocus()
            descriptionEdit.text = testDescription
            compare(artworkProxy.description, testDescription)
        }

        function test_addKeywordsSmokeTest() {
            compare(artworkProxy.keywordsCount, 0)

            var testKeyword = "keyword"
            keywordsEdit.forceActiveFocus()
            keywordsEdit.text = testKeyword
            keyClick(Qt.Key_Comma)

            compare(artworkProxy.keywordsCount, 1)
            compare(artworkProxy.getKeywordsString(), testKeyword)
        }
    }
}
