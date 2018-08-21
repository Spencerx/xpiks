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
        property var moreLink
        property var moreMenu

        function initTestCase() {
            //moreLink = findChild(artworkEditView, "moreLinkHost")
            //moreMenu = findChild(artworkEditView, "keywordsMoreMenuObject")
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }
    }
}
