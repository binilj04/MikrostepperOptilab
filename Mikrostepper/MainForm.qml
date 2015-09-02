import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.2
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.2
import QtQuick.Window 2.2

Item {
    id: root
    property alias keysModel: settingspage.keysModel

    width: 1280
    height: 720

    Dialog {
        id: dgSettings
        width: 900
        height: 600
        standardButtons: StandardButton.RestoreDefaults | StandardButton.Ok | StandardButton.Cancel
        SettingsPage {
            id: settingspage
        }
        onAccepted: {
            settingspage.updateSettings()
            if (settingspage.requireRestart) requestreset.open()
        }
        onReset: confirmreset.open()
    }

    MessageDialog {
        id: confirmreset
        title: "Confirm Settings Reset"
        text: "Reset all settings. Are you sure?"
        standardButtons: StandardButton.Yes | StandardButton.No
        onYes: {
            settingspage.restoreSettings()
            requestreset.open()
        }
    }

    MessageDialog {
        id: requestreset
        title: "Restart to Apply Changes"
        text: "Some settings will take effect next time application started."
        standardButtons: StandardButton.Ok
    }

    MessageDialog {
        id: errorMessage
        title: "Camera Not Found"
        text: "Please plug-in Optilab device."
        standardButtons: StandardButton.Ok
        icon: StandardIcon.Warning
        onAccepted: Qt.quit()
    }

    Item {
        id: ribbon
        height: 150
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.left: parent.left
        Behavior on height { PropertyAnimation { duration: 150 } }
        z: 80

        Item {
            id: ribbonmenu
            height: parent.height - 120
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.left: parent.left


            ButtonText {
                id: buttonSimple1
                text: "camera"
                tooltip: "Capture image and record video."
                fontSize: 11
                anchors.left: parent.left
                anchors.leftMargin: 5
                anchors.verticalCenter: parent.verticalCenter
            }

            ButtonText {
                id: buttonText4
                width: 60
                text: "Settings"
                fontSize: 9
                anchors.rightMargin: 0
                anchors.right: parent.right
                textAlignment: Text.AlignLeft
                anchors.verticalCenter: parent.verticalCenter
                onClicked: actionViewSettings.trigger()
            }
        }

        Item {
            id: ribboncontent
            anchors.right: parent.right
            anchors.left: parent.left
            anchors.bottom: parent.bottom
            anchors.top: ribbonmenu.bottom

            ViewerRibbon {
                id: viewerRibbon
                anchors.fill: parent
            }
        }
    }

    Item {
        id: content
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.bottom: status.top
        anchors.top: ribbon.bottom

        OptilabViewer {
            id: singlecontent
            anchors.fill: parent
            focus: visible
        }
    }

    Rectangle {
        id: status
        height: 25
        color: "white"
        border.color: "#95a5a6"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        Behavior on height { PropertyAnimation { duration: 150 } }
        RowLayout {
            anchors.fill: parent
            Item {
                height: 2
                Layout.fillWidth: true
            }
            TextRegular {
                id: aestatus
                property string aestat: (camprop.autoexposure) ? "ON" : "OFF"
                text: "AE: %1".arg(aestat)
            }
            Rectangle {
                width: 2
                height: 20
                color: "#bdc3c7"
            }
            Item {
                Layout.minimumWidth: 100
            }
            TextRegular {
                id: recordtime
                visible: (optilab.recordingStatus != 0)
                Connections {
                    target: optilab
                    onRecordingTime: recordtime.text = time
                }
                font.pointSize: 9
            }
        }
    }

    states: [
        State {
            name: "no_ribbon"

            PropertyChanges {
                target: ribbon
                height: 0
            }

            PropertyChanges {
                target: status
                height: 0
            }
        }
    ]

    transitions: [
    ]

    function getIndex(name) {
        for (var i = 0; i < keysModel.count; ++i) {
            if (name === keysModel.get(i).name)
                return i
        }
        return -1
    }

    Action {
        id: actionWhiteBalance
        shortcut: keysModel.get(getIndex("WhiteBalance")).shortcut
        onTriggered: camprop.oneShotWB()
    }
    Action {
        id: actionAutoExposure
        shortcut: keysModel.get(getIndex("AutoExposure")).shortcut
        onTriggered: camprop.autoexposure = !camprop.autoexposure
    }
    Action {
        id: actionParamsDefault
        shortcut: keysModel.get(getIndex("ParamsDefault")).shortcut
        onTriggered: camprop.loadDefaultParameters()
    }
    Action {
        id: actionParamsA
        shortcut: keysModel.get(getIndex("ParamsA")).shortcut
        onTriggered: camprop.loadParametersA()
    }
    Action {
        id: actionParamsB
        shortcut: keysModel.get(getIndex("ParamsB")).shortcut
        onTriggered: camprop.loadParametersB()
    }
    Action {
        id: actionParamsC
        shortcut: keysModel.get(getIndex("ParamsC")).shortcut
        onTriggered: camprop.loadParametersC()
    }
    Action {
        id: actionParamsD
        shortcut: keysModel.get(getIndex("ParamsD")).shortcut
        onTriggered: camprop.loadParametersD()
    }
    Action {
        id: actionGammaUp
        shortcut: keysModel.get(getIndex("GammaUp")).shortcut
        onTriggered: camprop.gamma += 1
    }
    Action {
        id: actionGammaDown
        shortcut: keysModel.get(getIndex("GammaDown")).shortcut
        onTriggered: camprop.gamma -= 1
    }
    Action {
        id: actionContrastUp
        shortcut: keysModel.get(getIndex("ContrastUp")).shortcut
        onTriggered: camprop.contrast += 1
    }
    Action {
        id: actionContrastDown
        shortcut: keysModel.get(getIndex("ContrastDown")).shortcut
        onTriggered: camprop.contrast -= 1
    }
    Action {
        id: actionSaturationUp
        shortcut: keysModel.get(getIndex("SaturationUp")).shortcut
        onTriggered: camprop.saturation += 1
    }
    Action {
        id: actionSaturationDown
        shortcut: keysModel.get(getIndex("SaturationDown")).shortcut
        onTriggered: camprop.saturation -= 1
    }
    Action {
        id: actionTargetUp
        shortcut: keysModel.get(getIndex("TargetUp")).shortcut
        onTriggered: {
            if (camprop.autoexposure) camprop.aeTarget += 1
        }
    }
    Action {
        id: actionTargetDown
        shortcut: keysModel.get(getIndex("TargetDown")).shortcut
        onTriggered: {
            if (camprop.autoexposure) camprop.aeTarget -= 1
        }
    }
    Action {
        id: actionAetimeUp
        shortcut: keysModel.get(getIndex("AetimeUp")).shortcut
        onTriggered: {
            if (!camprop.autoexposure) camprop.exposureTime += 10
        }
    }
    Action {
        id: actionAetimeDown
        shortcut: keysModel.get(getIndex("AetimeDown")).shortcut
        onTriggered: {
            if (!camprop.autoexposure) camprop.exposureTime -= 10
        }
    }
    Action {
        id: actionGainUp
        shortcut: keysModel.get(getIndex("GainUp")).shortcut
        onTriggered: {
            if (!camprop.autoexposure) camprop.aeGain += 1
        }
    }
    Action {
        id: actionGainDown
        shortcut: keysModel.get(getIndex("GainDown")).shortcut
        onTriggered: {
            if (!camprop.autoexposure) camprop.aeGain -= 1
        }
    }
    Action {
        id: actionViewSettings
        onTriggered: {
            settingspage.initSettings()
            dgSettings.open()
        }
        shortcut: keysModel.get(getIndex("View.Settings")).shortcut
    }

    Action {
        id: actionSingleCapture
        shortcut: keysModel.get(getIndex("Single.SingleCapture")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.singleCapture()
    }
    Action {
        id: actionCountCapture
        shortcut: keysModel.get(getIndex("Single.CountCapture")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.countCapture()
    }
    Action {
        id: actionCountIntervalUp
        shortcut: keysModel.get(getIndex("Single.CountIntervalUp")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.countIntervalUp()
    }
    Action {
        id: actionCountIntervalDown
        shortcut: keysModel.get(getIndex("Single.CountIntervalDown")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.countIntervalDown()
    }
    Action {
        id: actionCountUp
        shortcut: keysModel.get(getIndex("Single.CountUp")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.countUp()
    }
    Action {
        id: actionCountDown
        shortcut: keysModel.get(getIndex("Single.CountDown")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.countDown()
    }
    Action {
        id: actionDurationCapture
        shortcut: keysModel.get(getIndex("Single.DurationCapture")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.durationCapture()
    }
    Action {
        id: actionDurationIntervalUp
        shortcut: keysModel.get(getIndex("Single.DurationIntervalUp")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.durIntervalUp()
    }
    Action {
        id: actionDurationIntervalDown
        shortcut: keysModel.get(getIndex("Single.DurationIntervalDown")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.durIntervalDown()
    }
    Action {
        id: actionDurationUp
        shortcut: keysModel.get(getIndex("Single.DurationUp")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.durationUp()
    }
    Action {
        id: actionDurationDown
        shortcut: keysModel.get(getIndex("Single.DurationDown")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.durationDown()
    }
    Action {
        id: actionStartRecording
        shortcut: keysModel.get(getIndex("Single.StartRecording")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.startRecording()
    }
    Action {
        id: actionPauseRecording
        shortcut: keysModel.get(getIndex("Single.PauseRecording")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.pauseRecording()
    }
    Action {
        id: actionStopRecording
        shortcut: keysModel.get(getIndex("Single.StopRecording")).shortcut
        enabled: viewerRibbon.visible
        onTriggered: viewerRibbon.stopRecording()
    }

    Action {
        id: actionHideRibbon
        shortcut: "F11"
        onTriggered: {
            if (root.state === "")
                root.state = "no_ribbon"
            else
                root.state = ""
        }
    }

    Component.onDestruction: {
        dgSettings.visible = false
    }

    Component.onCompleted: {
        if (!camera.isAvailable()) {
            errorMessage.open()
            Qt.quit()
        }
        settingspage.initSettings()
    }
}

