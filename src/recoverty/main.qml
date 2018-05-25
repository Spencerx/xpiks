import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.4

Window {
    id: applicationWindow
    visible: true
    width: 640
    height: 480
    title: "Recoverty"

    Button {
        text: qsTr("Restart main app")
        anchors.centerIn: parent
        onClicked: {
            applicationWindow.visibility = "Minimized"
            closingTimer.start()
        }
    }

    Timer {
        id: closingTimer
        interval: 1000
        running: false
        repeat: false
        onTriggered: {
            restartModel.recover()
            Qt.quit()
        }
    }
}
