import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

Rectangle {
    id: root
    width: 640
    height: 480
    anchors.fill: parent
    color: "#ecf0f1"
    border.color: "#aabdc3c7"
    border.width: 2

    property alias source : image.source
    property alias savePath : fileDialog.fileUrl

    signal accept
    signal reject

    FileDialog {
        id: fileDialog
        title: "Save Image"
        nameFilters: [ "Portable Network Graphics (*.png)",
            "JPEG Standard Format (*.jpg)",
            "Windows Bitmap (*.bmp)",
            "Tagged Image File Format (*.tiff)"]
        selectExisting: false
        selectMultiple: false
        onAccepted: {
            root.accept()
        }
        onRejected: {
            fileUrl = ""
        }
    }

    BusyIndicator {
        id: busy
        antialiasing: true
        anchors.centerIn: image
        running: image.status === Image.Loading | Image.Null | Image.Error
    }

    Image {
        id: image
        fillMode: Image.PreserveAspectFit
        anchors.right: parent.right
        anchors.rightMargin: 25
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: btnSave.top
        anchors.bottomMargin: 10
        anchors.left: parent.left
        anchors.leftMargin: 25

        asynchronous: true
        cache: false

        sourceSize.width: width
        sourceSize.height: height
    }

    ButtonSimple {
        id: btnDiscard
        x: 180
        y: 207
        text: "DISCARD"
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: (parent.width - width)/3
        drawBorder: true
        tooltip: "Don't save this image"
        fontSize: 9
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        onClicked: {
            reject()
        }
    }

    ButtonSimple {
        id: btnSave
        x: 50
        y: 205
        text: "SAVE"
        isDefault: true
        anchors.horizontalCenterOffset: -(parent.width - width)/3
        drawBorder: true
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        tooltip: "Save this image"
        fontSize: 9
        onClicked: fileDialog.open()
    }
}
