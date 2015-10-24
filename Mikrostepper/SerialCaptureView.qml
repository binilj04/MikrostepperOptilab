import QtQuick 2.3

ListView {
    id: root

    implicitWidth: 300
    implicitHeight: 600

    delegate: SerialCaptureModel {
        source: optilab.fromLocalFile(path)
        text: path
    }

    property int durations: 200

    add: Transition {
        ParallelAnimation {
            NumberAnimation { properties: "scale"; from: 0.0; duration: durations }
            NumberAnimation { properties: "x"; from: root.width + 100; duration: durations }
            NumberAnimation { properties: "y"; from: root.height + 100; duration: durations }
        }
    }
    remove: Transition {
        ParallelAnimation {
            NumberAnimation { properties: "scale"; to: 0.0; duration: durations }
            NumberAnimation { properties: "x"; to: root.width + 100; duration: durations }
            NumberAnimation { properties: "y"; to: -100; duration: durations }
        }
    }
    displaced: Transition {
        NumberAnimation { properties: "y"; duration: durations }
    }
}
