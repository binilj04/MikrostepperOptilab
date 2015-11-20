import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

Dialog {
    id: root
    width: 640
    height: 480
    property alias imageModel: immodel
    property int totalCount: 9999

    Connections {
        target: optilab
        onImageSaved: {
            immodel.append({"imageFile": imgPath})
            pathView1.incrementCurrentIndex()
        }
    }

    onVisibleChanged: {
        if (!visible) {
            immodel.clear()
        }
    }

    contentItem: Rectangle {
        id: rectangle1
        width: 640
        height: 480

        ListModel {
            id: immodel
        }

        ListView {
            id: pathView1
            width: parent.width / 1.25
            anchors.bottom: buttonSave.top
            anchors.bottomMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter
            orientation: Qt.Horizontal
            snapMode: ListView.SnapOneItem
            clip: true

            model: immodel
            delegate: Component {
                Image {
                    width: pathView1.width
                    height: pathView1.height
                    fillMode: Image.PreserveAspectFit
                    source: optilab.fromLocalFile(imageFile)
                    anchors.verticalCenter: parent.verticalCenter
                    mipmap: true
                    asynchronous: true
                }
            }
        }

        FileDialog {
            id: dgSave
            selectFolder: true
            title: "Select Folder"
            onAccepted: {
                for (var i = 0; i < immodel.count; ++i) {
                    var name = optilab.fromLocalFile(immodel.get(i).imageFile)
                    optilab.copyToFolder(name, folder)
                }
                imageModel.clear()
                root.close()
            }
        }

        ButtonSimple {
            id: buttonCancel
            text: "Cancel"
            tooltip: "Cancel capture operation and discard images"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 30
            anchors.horizontalCenterOffset: 1.5*width
            anchors.horizontalCenter: parent.horizontalCenter
            drawBorder: true
            onClicked: {
                optilab.flushCommands()
                imageModel.clear()
                root.close()
            }
        }
        ButtonSimple {
            id: buttonSave
            text: "Save"
            tooltip: "Save images to disk"
            visible: (immodel.count == totalCount)
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 30
            anchors.horizontalCenterOffset: -1.5 * width
            anchors.horizontalCenter: parent.horizontalCenter
            drawBorder: true
            onClicked: {
                dgSave.visible = true
            }
        }

        ToolButton {
            id: toolButton1
            iconSource: "Images/next.png"
            tooltip: "Previous image"
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            rotation: 180
            onClicked: pathView1.decrementCurrentIndex()
        }

        ToolButton {
            id: toolButton2
            iconSource: "Images/next.png"
            tooltip: "Next image"
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: pathView1.incrementCurrentIndex()
        }
    }
}
