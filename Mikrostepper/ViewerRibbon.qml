import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Dialogs 1.2
import QuickCam 1.0

Rectangle {
    id: ribbonBar
    width: 720
    height: 120
    color: "white"
    anchors.top: parent.top
    anchors.right: parent.right
    anchors.left: parent.left

    property bool _lastAE
    property real _lastExpTime

    function singleCapture() {
        _lastAE = camprop.autoexposure
        camprop.autoexposure = false
        _lastExpTime = camprop.exposureTime
        var et = _lastExpTime / 2.295
        camprop.exposureTime = et
        captureTimer.start()
    }

    Timer {
        id: captureTimer
        interval: 100
        onTriggered: {
            optilab.captureToTemp("swrdaol.jpg")
            camprop.exposureTime = _lastExpTime
            camprop.autoexposure = _lastAE
        }
    }

    Connections {
        target: optilab
        onCaptureReady: {
            preview1.source = filename
            preview1.open()
        }
    }

    function countCapture() {
        var count = spinSCount.val
        var interval = ie1.totalInterval()
        if (count == 0 || interval === 0) return
        previewsc.totalCount = count
        previewsc.open()
        optilab.startSerialCaptureAsync(interval * 1000, count)
    }

    function countUp() { spinSCount.val += 1 }
    function countDown() { spinSCount.val -= 1 }
    function countIntervalUp() { ie1.addInterval() }
    function countIntervalDown() { ie1.minInterval() }

    function durationCapture() {
        var interval = ie2.totalInterval()
        var duration = ie3.totalInterval()
        var count = Math.round(duration / interval)
        if (count == 0 || interval === 0) return
        previewsc.totalCount = count
        previewsc.open()
        optilab.startSerialCaptureAsync(interval * 1000, count)
    }

    function durationUp() { ie3.addInterval() }
    function durationDown() { ie3.minInterval() }
    function durIntervalUp() { ie2.addInterval() }
    function durIntervalDown() { ie2.minInterval() }

    function startRecording() {
        if (!btnRecord.enabled) return
        if (optilab.recordingStatus == 0) {
            saverecording.open()
        }
        else {
            optilab.resumeRecording()
        }
    }
    function pauseRecording() {
        if (!btnPause.enabled) return
        optilab.pauseRecording()
    }
    function stopRecording() {
        if (!btnStop.enabled) return
        optilab.stopRecording()
    }

    Dialog {
        id: preview1
        title: "Preview"
        width: 640
        height: 480

        property alias source : singlePreview.source
        property alias savePath : singlePreview.savePath

        function show() { preview1.visible = true }
        function hide() { preview1.visible = false }

        onVisibleChanged: {
            if (!visible) preview1.source = ""
        }

        contentItem: PreviewImage {
            id: singlePreview
            onAccept: {
                optilab.scaleImage("swrdaol.jpg", 4100, 3075)
                optilab.copyFromTemp("swrdaol.jpg", preview1.savePath)
                preview1.source = ""
                preview1.hide()
            }
            onReject: {
                preview1.source = ""
                preview1.hide()
            }
        }
    }

    PreviewSC {
        id: previewsc
    }

    TimeEdit {
        id: ie1
        so: 1
    }

    TimeEdit {
        id: ie2
        so: 1
    }

    TimeEdit {
        id: ie3
        so: 5
    }

    FileDialog {
        id: saverecording
        nameFilters: [ "AVI video files (*.avi)"]
        selectExisting: false
        onAccepted: {
            optilab.initRecorder(fileUrl)
        }
    }

    ButtonRibbon {
        id: btnCapture
        y: 8
        text: "Capture Image"
        tooltip: "Capture image at high resolution"
        iconSource: "Images/capture.png"
        anchors.left: parent.left
        anchors.leftMargin: 10
        anchors.verticalCenterOffset: -10
        anchors.verticalCenter: parent.verticalCenter
        onClicked: singleCapture()
    }

    Rectangle {
        id: rectangle1
        y: 25
        width: 2
        height: 110
        color: "#bdc3c7"
        anchors.leftMargin: 10
        anchors.left: btnCapture.right
        anchors.verticalCenter: parent.verticalCenter
        border.width: 0
    }

    ButtonRibbon {
        id: btnSCap
        y: 10
        text: "Start Capture"
        tooltip: "Capture image frame count times every interval time"
        iconSource: "Images/serialcapture1.png"
        anchors.left: rectangle1.right
        anchors.leftMargin: 10
        anchors.verticalCenter: btnCapture.verticalCenter
        onClicked: countCapture()
    }

    TextRegular {
        id: textRegular1
        color: "#2c3e50"
        text: "Interval"
        font.pointSize: 9
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
        anchors.right: btnSInterval.left
        anchors.left: btnSInterval.right
        anchors.top: btnSCap.top
    }

    ButtonSimple {
        id: btnSInterval
        fontSize: 9
        drawBorder: true
        anchors.left: btnSCap.right
        anchors.leftMargin: 10
        anchors.topMargin: 0
        anchors.top: textRegular1.bottom
        onClicked: ie1.visible = true
        text: "%1%2:%3%4:%5%6".arg(ie1.ht).arg(ie1.ho).arg(
                  ie1.mt).arg(ie1.mo).arg(ie1.st).arg(ie1.so)
    }

    SpinInt {
        id: spinSCount
        x: 162
        title: "Frame Count"
        anchors.horizontalCenter: btnSInterval.horizontalCenter
        anchors.top: btnSInterval.bottom
        anchors.topMargin: 5
        minVal: 1
    }

    Rectangle {
        id: rectangle2
        y: 18
        width: 2
        height: 110
        color: "#bdc3c7"
        anchors.left: btnSInterval.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
        border.width: 0
    }

    ButtonRibbon {
        id: btnECap
        y: 10
        text: "Start Capture"
        tooltip: "Capture image during duration time  every interval time"
        iconSource: "Images/serialcapture2.png"
        anchors.left: rectangle2.right
        anchors.leftMargin: 10
        anchors.verticalCenter: btnSCap.verticalCenter
        onClicked: durationCapture()
    }

    ButtonSimple {
        id: btnEInterval
        fontSize: 9
        drawBorder: true
        anchors.top: textRegular2.bottom
        anchors.topMargin: 0
        anchors.left: btnECap.right
        anchors.leftMargin: 10
        onClicked: ie2.visible = true
        text: "%1%2:%3%4:%5%6".arg(ie2.ht).arg(ie2.ho).arg(
                  ie2.mt).arg(ie2.mo).arg(ie2.st).arg(ie2.so)
    }

    TextRegular {
        id: textRegular2
        x: 354
        color: "#2c3e50"
        text: "Interval"
        font.pointSize: 9
        anchors.top: btnECap.top
        anchors.horizontalCenter: btnEInterval.horizontalCenter
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    TextRegular {
        id: textRegular3
        x: 366
        color: "#2c3e50"
        text: "Duration"
        font.pointSize: 9
        anchors.horizontalCenter: btnEDuration.horizontalCenter
        anchors.top: btnEInterval.bottom
        anchors.topMargin: 3
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    ButtonSimple {
        id: btnEDuration
        x: 354
        fontSize: 9
        drawBorder: true
        anchors.top: textRegular3.bottom
        anchors.topMargin: 0
        onClicked: ie3.visible = true
        text: "%1%2:%3%4:%5%6".arg(ie3.ht).arg(ie3.ho).arg(
                  ie3.mt).arg(ie3.mo).arg(ie3.st).arg(ie3.so)
    }

    Rectangle {
        id: rectangle3
        y: 5
        width: 2
        height: 110
        color: "#bdc3c7"
        anchors.leftMargin: 10
        anchors.left: btnEInterval.right
        anchors.verticalCenter: parent.verticalCenter
        border.width: 0
    }

    TextRegular {
        id: textRegular4
        y: 99
        color: "#2c3e50"
        text: "Serial Capture"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        font.pointSize: 9
        anchors.right: rectangle2.left
        anchors.left: rectangle1.right
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    TextRegular {
        id: textRegular5
        y: 102
        color: "#2c3e50"
        text: "Elapsed Capture"
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        font.pointSize: 9
        anchors.right: rectangle3.left
        anchors.left: rectangle2.right
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    ButtonRibbon {
        id: btnRecord
        y: 10
        text: "Record"
        anchors.left: rectangle3.right
        anchors.leftMargin: 10
        anchors.verticalCenter: btnECap.verticalCenter
        tooltip: "Start recording"
        iconSource: "Images/record1.png"
        enabled: (optilab.recordingStatus != 1)
        onClicked: startRecording()
    }

    ButtonRibbon {
        id: btnPause
        y: 10
        text: "Pause"
        anchors.left: btnRecord.right
        anchors.leftMargin: 10
        anchors.verticalCenter: btnRecord.verticalCenter
        tooltip: "Pause recording"
        iconSource: "Images/pause1.png"
        enabled: (optilab.recordingStatus == 1)
        onClicked: pauseRecording()
    }

    ButtonRibbon {
        id: btnStop
        y: 10
        text: "Stop"
        anchors.left: btnPause.right
        anchors.leftMargin: 10
        anchors.verticalCenter: btnPause.verticalCenter
        iconSource: "Images/stop1.png"
        tooltip: "Stop recording"
        enabled: (optilab.recordingStatus != 0)
        onClicked: stopRecording()
    }

    Rectangle {
        id: rectangle4
        y: 18
        width: 2
        height: 110
        color: "#bdc3c7"
        border.width: 0
        anchors.left: btnStop.right
        anchors.leftMargin: 10
        anchors.verticalCenter: parent.verticalCenter
    }

    TextRegular {
        id: textRegular6
        y: 101
        color: "#2c3e50"
        text: "Recording"
        font.pointSize: 9
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 5
        anchors.right: rectangle4.left
        anchors.left: rectangle3.right
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }
}

