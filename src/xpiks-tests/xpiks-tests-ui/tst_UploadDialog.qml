import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600

    Component.onCompleted: {
        console.log(TestsHost.scoreme)
    }

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    QtObject {
        id: i18
        property string n: ""
    }

    UploadArtworks {
        id: uploadDialog
        anchors.fill: parent
    }

    TestCase {
        name: "UploadArtworks"
        when: windowShown

        function initTestCase() {
            TestsHost.setup()
        }

        function cleanupTestCase() {
            TestsHost.cleanup()
        }
    }
}
