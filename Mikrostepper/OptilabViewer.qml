import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QuickCam 1.0

Rectangle {
    id: bg
    width: 1280
    height: 720
    color: "#34495e"
    border.width: 0
    focus: visible

    property alias screenWidth: viewer.width
    property alias screenHeight: viewer.height

    function updateAspectRatio() {
        viewer.offsetSize = optilab.calculateAspectRatio(bg.width, bg.height)
    }

    onWidthChanged: updateAspectRatio()
    onHeightChanged: updateAspectRatio()

    Connections {
        target: camera
        onFrameReady: viewer.source = frame
    }

    CameraItem {
        id: viewer
        property size offsetSize : parent.updateAspectRatio()
        antialiasing: true
        anchors.fill: parent
        anchors {
            leftMargin: offsetSize.width; rightMargin: offsetSize.width
            topMargin: offsetSize.height; bottomMargin: offsetSize.height
        }

        Timer {
            id: timerfocus
            interval: 500
            onTriggered: {
                focus.visible = false
                focus.border.color = "yellow"
            }
        }
    }
}

