import QtQuick 2.3

Rectangle {
    id: rectangle1
    property alias source: item1.source
    property alias text: text1.text
    width: 280
    height: 80
    radius: 10
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#ecf0f1"
        }

        GradientStop {
            position: 1
            color: "#7f8c8d"
        }
    }
    border.color: "#bdc3c7"
    border.width: 1

    Image {
        id: item1
        fillMode: Image.PreserveAspectFit
        anchors.leftMargin: 10
        anchors.left: text1.right
        anchors.rightMargin: 5
        anchors.right: parent.right
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.top: parent.top
    }

    TextRegular {
        id: text1
        width: 0.6*parent.width
        text: ""
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap
        anchors.left: parent.left
        anchors.leftMargin: 5
        anchors.verticalCenter: parent.verticalCenter
    }
}

