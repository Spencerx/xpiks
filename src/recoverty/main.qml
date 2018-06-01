import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.1
import QtQuick.Controls.Styles 1.4

Window {
    id: applicationWindow
    visible: true
    width: 565
    height: 424
    title: "Recoverty"

    Rectangle {
        anchors.fill: parent
        color: "#f8f8fb"

        ColumnLayout {
            anchors.fill: parent
            spacing: 0

            Item {
                height: 50
            }

            Image {
                source: "qrc:/turtle.svg"
                sourceSize.width: 237
                sourceSize.height: 141
                width: 237
                height: 141
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.horizontalCenterOffset: 20
            }

            Item {
                height: 60
            }

            Text {
                text: qsTr("Oh no! %1 crashed!").arg(restartModel.appName)
                font.bold: true
                font.pixelSize: 25
                anchors.horizontalCenter: parent.horizontalCenter
                renderType: Text.NativeRendering
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }

            Item {
                height: 40
            }

            Button {
                text: qsTr("Restart")
                width: 220
                height: 50
                focus: true
                enabled: true
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: {
                    applicationWindow.visibility = "Minimized"
                    closingTimer.start()
                }

                style: ButtonStyle {
                    background: Rectangle {
                        radius: 3
                        implicitWidth: control.width
                        implicitHeight: control.height > 24 ? control.height : 24
                        color: {
                            var result = "#1EBFCA";

                            if (control.enabled && control.hovered) {
                                result = "#17A1AB";
                            }

                            return result
                        }
                    }

                    label: Text {
                        font.bold: true
                        renderType: Text.NativeRendering
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        font.family: Qt.platform.os === "windows" ? "Arial" : "Helvetica"
                        text: control.text
                        font.pixelSize: 20
                        color: {
                            var defaultForeground = "#ffffff";
                            var result = defaultForeground;//"#22252C";

                            if (control.enabled) {
                                if (control.pressed) {
                                    result = "#c9c9c9"
                                } else {
                                    result = control.hovered ? "#ffffff" : defaultForeground;
                                }
                            } else {
                                result = defaultForeground;
                            }

                            return result
                        }
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }
        }

        /*Text {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
            anchors.horizontalCenter: parent.horizontalCenter
            visible: debug
            enabled: debug
            text: restartModel.arguments
        }*/

        Timer {
            id: closingTimer
            interval: 1000
            running: false
            repeat: false
            onTriggered: {
                restartModel.recover()
                Qt.quit()
            }
        }}
}
