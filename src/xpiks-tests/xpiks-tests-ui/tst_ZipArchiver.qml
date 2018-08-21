import QtQuick 2.0
import QtTest 1.1
import XpiksTests 1.0
import "../../xpiks-qt/Dialogs"

Item {
    id: root
    width: 800
    height: 600
    property string scoreme: TestsHost.scoreme

    QtObject {
        id: applicationWindow
        property bool leftSideCollapsed: false
    }

    ZipArtworksDialog {
        id: zipDialog
        anchors.fill: parent
    }

    TestCase {
        name: "ZipArchiver"
        when: windowShown
    }
}
