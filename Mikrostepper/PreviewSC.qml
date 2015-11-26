import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2

Dialog {
    id: root
    width: 640
    height: 480
    property alias imageModel: immodel
    property int totalCount: 9999
    property int current: -1

    signal abortSC

    Connections {
        target: optilab
        onImageSaved: {
            immodel.append({"imageFile": imgPath})
            current += 1
        }
    }

    onCurrentChanged: {
        if (current < 0) return
        var img = immodel.get(current).imageFile
        if (optilab.exists(img))
            view.source = optilab.fromLocalFile(img)
    }

    onVisibleChanged: {
        if (!visible) {
            abortSC()
            immodel.clear()
            current = -1
            view.source = ""
        }
    }

    ListModel {
        id: immodel
    }

    contentItem: Rectangle {
        id: rectangle1
        width: 640
        height: 480

        Image {
            id: view
            width: parent.width / 1.25
            anchors.bottom: indicator.top
            anchors.bottomMargin: 10
            anchors.top: parent.top
            anchors.topMargin: 30
            anchors.horizontalCenter: parent.horizontalCenter

            fillMode: Image.PreserveAspectFit
            mipmap: true
            asynchronous: true
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

        TextRegular {
            id: indicator
            anchors {
                bottom: buttonSave.top; bottomMargin: 10
                horizontalCenter: view.horizontalCenter
            }
            text: "%1/%2".arg(current + 1).arg(totalCount)
        }

        ButtonSimple {
            id: buttonCancel
            text: "Abort"
            visible: immodel.count != totalCount
            tooltip: "Cancel capture operation and discard images"
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
//            anchors.horizontalCenterOffset: 1.5*width
            anchors.horizontalCenter: parent.horizontalCenter
            drawBorder: true
            onClicked: {
                optilab.flushCommands()
                abortSC()
                imageModel.clear()
                root.close()
            }
        }
        ButtonSimple {
            id: buttonSave
            text: "Save"
            tooltip: "Save images to disk"
//            visible: (immodel.count == totalCount)
            visible: false
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 20
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
            onClicked: if (current > 0) current -= 1
        }

        ToolButton {
            id: toolButton2
            iconSource: "Images/next.png"
            tooltip: "Next image"
            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            onClicked: if (current < totalCount - 1) current += 1
        }
    }
}
