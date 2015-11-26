import QtQuick 2.3
import QtQuick.Window 2.0
import QtQuick.Controls 1.2

Window {
    id: root
    width: 600
    height: 400
    flags: Qt.SplashScreen
    modality: Qt.ApplicationModal
    color: "transparent"

    Timer {
        id: timeout
        interval: 3000
        onTriggered: close()
    }

    Image {
        id: splashlogo
        anchors.fill: parent
        source: "qrc:///Images/SplashScreen.png"
    }

    Component.onCompleted: {
        visible = true
        timeout.start()
    }
}
