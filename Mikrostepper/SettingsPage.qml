import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Layouts 1.0
import QtQuick.Controls.Styles 1.3
import QuickCam 1.0

Rectangle {
    id: root
    property alias keysModel: _model
    property bool requireRestart: false
    property bool updatingCamera: false
    width: 900
    height: 600

    function initSettings() {
        for (var i = 0; i < _model.count; ++i) {
            var key = appsettings.readShortcut(_model.get(i).name, _model.get(i).keys)
            _model.set(i, { "shortcut": key })
        }
    }

    function updateSettings() {
        for (var i = 0; i < _model.count; ++i) {
            var key = appsettings.readShortcut(_model.get(i).name, _model.get(i).keys)
            if (key !== _model.get(i).shortcut)
                appsettings.saveShortcut(_model.get(i).name, _model.get(i).shortcut)
        }
    }

    function restoreSettings() {
        for (var i = 0; i < _model.count; ++i) {
            var key = appsettings.readShortcut(_model.get(i).name, _model.get(i).keys)
            if (key !== _model.get(i).keys)
                appsettings.resetShortcut(_model.get(i).name)
        }
    }

    function updateCamera() {
        updatingCamera = true
        sliderHue.value = camprop.hue
        sliderContrast.value = camprop.contrast
        sliderSaturation.value = camprop.saturation
        sliderGamma.value = camprop.gamma
        sliderBrightness.value = camprop.brightness
        checkboxAutoExposure.checked = camprop.autoexposure
        sliderGain.value = camprop.aeGain
        sliderTime.value = camprop.exposureTime / 1000
        sliderTarget.value = camprop.aeTarget
        sliderTemperature.value = camprop.whiteBalanceTemperature
        sliderTint.value = camprop.whiteBalanceTint
        if (camprop.isColor)
            checkBoxColor.checked = true
        else
            checkBoxBW.checked = true
        if (camprop.isHFlip !== checkBoxHFlip.checked)
            checkBoxHFlip.checked = camprop.isHFlip
        if (camprop.isVFlip !== checkBoxVFlip.checked)
            checkBoxVFlip.checked = camprop.isVFlip
        if (camprop.isBin)
            checkBoxBin.checked = true
        else
            checkBoxSkip.checked = true
        sliderFrameRate.value = camprop.frameRate

        camprop.gamma += 10
        camprop.gamma -= 10
        camprop.aeGain += 10
        camprop.aeGain -= 10
        camprop.exposureTime += 5
        camprop.exposureTime -= 5

        updatingCamera = false
    }

    BusyDialog {
        id: initbusy
    }

    RowLayout {
        id: rowLayout1
        spacing: 0
        anchors.fill: parent

        Rectangle {
            id: menu
            color: "#ecf0f1"
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            Layout.preferredWidth: 200
            ExclusiveGroup { id: group1 }
            ColumnLayout {
                id: rowLayout2
                anchors.fill: parent

                Item {
                    id: spacer2
                    Layout.minimumHeight: 50
                }

                ButtonText {
                    id: buttonText1
                    text: "Camera"
                    textDefault: "#000000"
                    checked: true
                    checkable: true
                    fontSize: 8
                    textAlignment: 1
                    Layout.alignment: Qt.AlignCenter
                    exclusiveGroup: group1
                    onCheckedChanged: {
                        if (checked) root.state = ""
                    }
                }

                ButtonText {
                    id: buttonText3
                    text: "Keyboard"
                    textDefault: "#000000"
                    checkable: true
                    fontSize: 8
                    textAlignment: 1
                    Layout.alignment: Qt.AlignCenter
                    exclusiveGroup: group1
                    onCheckedChanged: {
                        if (checked) root.state = "keyboard settings"
                    }
                }

                Item {
                    id: spacer
                    Layout.fillHeight: true
                }
            }

        }

        Rectangle {
            id: lineasd
            width: 2
            height: root.height - 40
            color: "#bdc3c7"
        }

        Rectangle {
            id: content
            color: "#ecf0f1"
            anchors.bottom: parent.bottom
            anchors.top: parent.top
            Layout.fillWidth: true

            Item {
                id: cameraContent
                anchors.fill: parent

                Rectangle {
                    id: preview
                    height: (9/16) * width
                    color: "#2c3e50"
                    anchors.leftMargin: 20
                    anchors.left: scrollView.right
                    anchors.top: parent.top
                    anchors.topMargin: 20
                    anchors.right: parent.right
                    anchors.rightMargin: 20
                    border.color: "#bdc3c7"
                    CameraItem {
                        id: previewitem
                        property size offsetSize: optilab.calculateAspectRatio(preview.width, preview.height)
                        anchors.fill: parent
                        Connections {
                            target: camera
                            onFrameReady: previewitem.source = frame
                        }
                        anchors {
                            leftMargin: offsetSize.width; rightMargin: offsetSize.width
                            topMargin: offsetSize.height; bottomMargin: offsetSize.height
                        }

                        Item {
                            id: anchTL
                            x: 50
                            y: 50
                        }

                        Item {
                            id: anchBR
                            x: 100
                            y: 100
                        }

                        Rectangle {
                            id: wbRect
                            visible: camprop.cameraType() === 2
                            anchors {
                                top: anchTL.top; left: anchTL.left
                                bottom: anchBR.bottom; right: anchBR.right
                            }
                            color: "transparent"
                            border.width: 2
                            border.color: "blue"

                            property real sz: 25

                            MouseArea {
                                id: maMove
                                anchors.fill: parent
                                anchors.margins: 2
                                cursorShape: Qt.SizeAllCursor
                                property var lastXY
                                onPressed: lastXY = Qt.point(mouse.x, mouse.y)
                                onPositionChanged: {
                                    if (pressed) {
                                        var diffX = mouse.x - lastXY.x
                                        var diffY = mouse.y - lastXY.y
                                        if (anchTL.x + diffX > 0 && anchBR.x + diffX < previewitem.width) {
                                            anchTL.x += diffX
                                            anchBR.x += diffX
                                        }
                                        if (anchTL.y + diffY > 0 && anchBR.y + diffY < previewitem.height) {
                                            anchTL.y += diffY
                                            anchBR.y += diffY
                                        }
                                    }
                                }
                            }
                            MouseArea {
                                id: maUp
                                height: 2
                                anchors {
                                    right: parent.right; left: parent.left
                                    rightMargin: 2; leftMargin: 2
                                    top: parent.top
                                }
                                cursorShape: Qt.SizeVerCursor
                                drag.target: anchTL
                                drag.axis: Drag.YAxis
                                drag.minimumY: 0
                                drag.maximumY: anchBR.y - wbRect.sz
                            }
                            MouseArea {
                                id: maBot
                                height: 2
                                anchors {
                                    right: parent.right; left: parent.left
                                    rightMargin: 2; leftMargin: 2
                                    bottom: parent.bottom
                                }
                                cursorShape: Qt.SizeVerCursor
                                drag.target: anchBR
                                drag.axis: Drag.YAxis
                                drag.minimumY: anchTL.y + wbRect.sz
                                drag.maximumY: previewitem.height
                            }
                            MouseArea {
                                id: maLeft
                                width: 2
                                anchors {
                                    top: parent.top; bottom: parent.bottom
                                    topMargin: 2; bottomMargin: 2
                                    left: parent.left
                                }
                                cursorShape: Qt.SizeHorCursor
                                drag.target: anchTL
                                drag.axis: Drag.XAxis
                                drag.minimumX: 0
                                drag.maximumX: anchBR.x - wbRect.sz
                            }
                            MouseArea {
                                id: maRight
                                width: 2
                                anchors {
                                    top: parent.top; bottom: parent.bottom
                                    topMargin: 2; bottomMargin: 2
                                    right: parent.right
                                }
                                cursorShape: Qt.SizeHorCursor
                                drag.target: anchBR
                                drag.axis: Drag.XAxis
                                drag.minimumX: anchTL.x + wbRect.sz
                                drag.maximumX: previewitem.width
                            }
                            MouseArea {
                                id: maTL
                                width: 2; height: 2
                                anchors {
                                    top: parent.top; left: parent.left
                                }
                                cursorShape: Qt.SizeFDiagCursor
                                drag.target: anchTL
                                drag.axis: Drag.XAndYAxis
                                drag.minimumX: 0
                                drag.minimumY: 0
                                drag.maximumX: anchBR.x - wbRect.sz
                                drag.maximumY: anchBR.y - wbRect.sz

                            }
                            MouseArea {
                                id: maTR
                                width: 2; height: 2
                                anchors {
                                    top: parent.top; right: parent.right
                                }
                                cursorShape: Qt.SizeBDiagCursor
                                property var lastXY
                                onPressed: lastXY = Qt.point(mouse.x, mouse.y)
                                onPositionChanged: {
                                    if (pressed) {
                                        var diffX = mouse.x - lastXY.x
                                        var diffY = mouse.y - lastXY.y
                                        if (anchBR.x + diffX > anchTL.x + wbRect.sz && anchBR.x + diffX < previewitem.width)
                                            anchBR.x += diffX
                                        if (anchTL.y + diffY > 0 && anchTL.y + diffY < anchBR.y - wbRect.sz)
                                            anchTL.y += diffY
                                    }
                                }
                            }
                            MouseArea {
                                id: maBL
                                width: 2; height: 2
                                anchors {
                                    bottom: parent.bottom; left: parent.left
                                }
                                cursorShape: Qt.SizeBDiagCursor
                                property var lastXY
                                onPressed: lastXY = Qt.point(mouse.x, mouse.y)
                                onPositionChanged: {
                                    if (pressed) {
                                        var diffX = mouse.x - lastXY.x
                                        var diffY = mouse.y - lastXY.y
                                        if (anchTL.x + diffX > 0 && anchTL.x + diffX < anchBR.x - wbRect.sz) {
                                            anchTL.x += diffX
                                        }
                                        if (anchBR.y + diffY > anchTL.y + wbRect.sz && anchBR.y + diffY < previewitem.height) {
                                            anchBR.y += diffY
                                        }
                                    }
                                }

                            }
                            MouseArea {
                                id: maBR
                                width: 2; height: 2
                                anchors {
                                    bottom: parent.bottom; right: parent.right
                                }
                                cursorShape: Qt.SizeFDiagCursor
                                drag.target: anchBR
                                drag.axis: Drag.XAndYAxis
                                drag.minimumX: anchTL.x + wbRect.sz
                                drag.minimumY: anchTL.y + wbRect.sz
                                drag.maximumX: previewitem.width
                                drag.maximumY: previewitem.height
                            }

                            onXChanged: camprop.whiteBalanceBox = getRect()
                            onYChanged: camprop.whiteBalanceBox = getRect()
                            onWidthChanged: camprop.whiteBalanceBox = getRect()
                            onHeightChanged: camprop.whiteBalanceBox = getRect()
                            function getRect() {
                                var wRatio = camera.size().width / previewitem.width
                                var hRatio = camera.size().height / previewitem.height
                                var rx = wRatio * wbRect.x
                                var ry = hRatio * wbRect.y
                                var rw = wRatio * wbRect.width
                                var rh = hRatio * wbRect.height
                                return Qt.rect(rx, ry, rw, rh)
                            }
                        }

                        Text {
                            id: wbText
                            visible: camprop.cameraType() === 2
                            text: "White Balance"
                            anchors.left: wbRect.left
                            anchors.top: wbRect.bottom
                            font.pointSize: 6
                            color: "blue"
                        }
                    }
                }

                ScrollView {
                    id: scrollView
                    width: 275
                    anchors.left: parent.left
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.leftMargin: 20
                    anchors.topMargin: 20
                    anchors.bottomMargin: 20
                    clip: true
//                    contentHeight: columnLayout1.height

                    ColumnLayout {
                        id: columnLayout1
                        width: 250
                        height: 950
                        anchors.left: parent.left
                        anchors.top: parent.top

                        TextBlack {
                            id: textBlack1
                            text: "Color"
                            font.pointSize: 10
                        }

                        TextRegular {
                            visible: camprop.controlAvailable("hue")
                            text: "Hue: %1".arg(Math.round(sliderHue.value))
                        }

                        KeySlider {
                            id: sliderHue
                            visible: camprop.controlAvailable("hue")
                            minimumValue: camprop.controlMin("hue")
                            maximumValue: camprop.controlMax("hue")
                            stepSize: 1
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.hue = value
                            }
                        }

                        TextRegular {
                            id: textRegular3
                            visible: camprop.controlAvailable("saturation")
                            text: "Saturation: %1".arg(Math.round(sliderSaturation.value))
                        }

                        KeySlider {
                            id: sliderSaturation
                            maximumValue: camprop.controlMax("saturation")
                            minimumValue: camprop.controlMin("saturation")
                            stepSize: 1
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.saturation = value
                            }
                        }

                        TextRegular {
                            visible: camprop.controlAvailable("brightness")
                            text: "Brightness: %1".arg(Math.round(sliderBrightness.value))
                        }

                        KeySlider {
                            id: sliderBrightness
                            visible: camprop.controlAvailable("brightness")
                            minimumValue: camprop.controlMin("brightness")
                            maximumValue: camprop.controlMax("brightness")
                            stepSize: 1
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.brightness = value
                            }
                        }

                        TextRegular {
                            id: textRegular2
                            visible: camprop.controlAvailable("contrast")
                            text: "Contrast: %1".arg(Math.round(sliderContrast.value))
                        }

                        KeySlider {
                            id: sliderContrast
                            visible: camprop.controlAvailable("contrast")
                            minimumValue: camprop.controlMin("contrast")
                            maximumValue: camprop.controlMax("contrast")
                            stepSize: 1
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.contrast = value
                            }
                        }

                        TextRegular {
                            id: textRegular1
                            visible: camprop.controlAvailable("gamma")
                            text: "Gamma: %1".arg(Math.round(sliderGamma.value))
                        }

                        KeySlider {
                            id: sliderGamma
                            visible: camprop.controlAvailable("gamma")
                            maximumValue: camprop.controlMax("gamma")
                            minimumValue: camprop.controlMin("gamma")
                            stepSize: 1.0
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.gamma = value
                            }
                        }

                        Item { height: 15 }

                        TextBlack {
                            id: textBlack2
                            text: "Exposure"
                            font.pointSize: 10
                        }

                        CheckBox {
                            id: checkboxAutoExposure
                            text: "Automatic Exposure"
                            checked: camprop.autoexposure
                            onCheckedChanged: {
                                if (!updatingCamera)
                                    camprop.autoexposure = checked
                            }
                        }

                        TextRegular {
                            id: textRegular4
                            text: "Target: %1".arg(Math.round(sliderTarget.value))
                        }

                        KeySlider {
                            id: sliderTarget
                            maximumValue: camprop.controlMax("aetarget")
                            minimumValue: camprop.controlMin("aetarget")
                            stepSize: 1
                            enabled: checkboxAutoExposure.checked
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.aeTarget = value
                            }
                        }

                        TextRegular {
                            id: textRegular5
                            property real multiplier: (camprop.cameraType() === 1) ? 100 : 1
                            text: "Exposure Time: %1 ms".arg(sliderTime.value * multiplier)
                        }

                        KeySlider {
                            id: sliderTime
                            maximumValue: camprop.controlMax("aeexposure")
                            minimumValue: camprop.controlMin("aeexposure")
                            stepSize: 0.1
                            enabled: !checkboxAutoExposure.checked
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: if (!checkboxAutoExposure.checked && !updatingCamera) camprop.exposureTime = value * 1000
                        }

                        TextRegular {
                            id: textRegular6
                            text: "Gain: %1".arg(sliderGain.value)
                        }

                        KeySlider {
                            id: sliderGain
                            maximumValue: camprop.controlMax("aegain")
                            minimumValue: camprop.controlMin("aegain")
                            stepSize: 1
                            enabled: !checkboxAutoExposure.checked
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: if (!checkboxAutoExposure.checked && !updatingCamera) camprop.aeGain = value
                        }

                        Item { height: 15 }

                        TextBlack {
                            id: textBlack3
                            visible: camprop.controlAvailable("awbtemp")
                            text: "Manual White Balance"
                            font.pointSize: 10
                        }

                        TextRegular {
                            visible: camprop.controlAvailable("awbtemp")
                            text: "Temperature: %1".arg(Math.round(sliderTemperature.value))
                        }

                        KeySlider {
                            id: sliderTemperature
                            visible: camprop.controlAvailable("awbtemp")
                            minimumValue: camprop.controlMin("awbtemp")
                            maximumValue: camprop.controlMax("awbtemp")
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.whiteBalanceTemperature = value
                            }
                        }

                        TextRegular {
                            visible: camprop.controlAvailable("awbtint")
                            text: "Tint: %1".arg(Math.round(sliderTint.value))
                        }

                        KeySlider {
                            id: sliderTint
                            visible: camprop.controlAvailable("awbtint")
                            minimumValue: camprop.controlMin("awbtint")
                            maximumValue: camprop.controlMax("awbtint")
                            stepSize: 1
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (!updatingCamera)
                                    camprop.whiteBalanceTint = value
                            }
                        }

                        TextBlack {
                            text: "Miscellaneous"
                            font.pointSize: 10
                        }

                        TextRegular {
                            visible: camprop.controlAvailable("framerate")
                            text: "Frame Rate: %1".arg(sliderFrameRate.label)
                        }

                        KeySlider {
                            id: sliderFrameRate
                            visible: camprop.controlAvailable("framerate")
                            property string label: (camprop.cameraType() === 2) ? "Slowest" : "Normal"
                            minimumValue: 0
                            maximumValue: camprop.controlMax("framerate")
                            stepSize: 1
                            Layout.alignment: Qt.AlignCenter
                            onValueChanged: {
                                if (camprop.cameraType() === 2) {
                                    switch (value) {
                                    case 0:
                                        label = "Slowest"
                                        break
                                    case 1:
                                        label = "Slow"
                                        break
                                    case 2:
                                        label = "Fast"
                                        break
                                    case 3:
                                        label = "Fastest"
                                        break
                                    }
                                }
                                else if (camprop.cameraType() === 1) {
                                    switch (value) {
                                    case 0:
                                        label = "Normal"
                                        break
                                    case 1:
                                        label = "Super"
                                        break
                                    }
                                }
                                if (!updatingCamera) {
                                    camprop.frameRate = value
                                }
                            }
                        }

                        TextRegular {
                            text: "Color Mode"
                        }

                        ExclusiveGroup { id: isColorGroup }

                        RadioButton {
                            id: checkBoxColor
                            text: "Color"
                            exclusiveGroup: isColorGroup
                            onCheckedChanged: camprop.isColor = checked
                        }

                        RadioButton {
                            id: checkBoxBW
                            text: "Black/White"
                            exclusiveGroup: isColorGroup
                        }

                        TextRegular {
                            text: "Mirror"
                        }

                        CheckBox {
                            id: checkBoxHFlip
                            text: "Horizontal"
                            onCheckedChanged: camprop.isHFlip = checked

                        }

                        CheckBox {
                            id: checkBoxVFlip
                            text: "Vertical"
                            onCheckedChanged: camprop.isVFlip = checked
                        }

                        TextRegular {
                            visible: camprop.cameraType() === 2
                            text: "Sampling method"
                        }

                        ExclusiveGroup { id: isBinGroup }

                        RadioButton {
                            id: checkBoxBin
                            visible: camprop.cameraType() === 2
                            text: "Bin (better image, slower)"
                            exclusiveGroup: isBinGroup
                            onCheckedChanged: camprop.isBin = checked
                        }

                        RadioButton {
                            id: checkBoxSkip
                            visible: camprop.cameraType() === 2
                            text: "Skip (fast)"
                            exclusiveGroup: isBinGroup
                        }
                    }
                }

                TextBlack {
                    id: textBlack4
                    text: "Parameter Group"
                    anchors.topMargin: 15
                    anchors.left: preview.left
                    font.pointSize: 10
                    anchors.top: button3.bottom
                }

                ExclusiveGroup { id: group2 }

                GridLayout {
                    id: gridlayout
                    width: 250
                    anchors.leftMargin: 20
                    anchors.topMargin: 10
                    anchors.left: textBlack4.left
                    anchors.top: textBlack4.bottom
                    columnSpacing: 20
                    rowSpacing: 10
                    rows: 5
                    flow: GridLayout.TopToBottom
                    RadioButton {
                        id: radioButton1
                        text: qsTr("A Group")
                        Layout.columnSpan: 2
                        checked: true
                        exclusiveGroup: group2
                        onCheckedChanged: {
                            if (checked) camprop.loadParametersA()
                            updatingCamera = true
                            camUpdating.start()
                        }
                    }

                    RadioButton {
                        id: radioButton2
                        text: qsTr("B Group")
                        Layout.columnSpan: 2
                        exclusiveGroup: group2
                        onCheckedChanged: {
                            if (checked) camprop.loadParametersB()
                            updatingCamera = true
                            camUpdating.start()
                        }
                    }

                    RadioButton {
                        id: radioButton3
                        text: qsTr("C Group")
                        Layout.columnSpan: 2
                        exclusiveGroup: group2
                        onCheckedChanged: {
                            if (checked) camprop.loadParametersC()
                            updatingCamera = true
                            camUpdating.start()
                        }
                    }

                    RadioButton {
                        id: radioButton4
                        text: qsTr("D Group")
                        Layout.columnSpan: 2
                        exclusiveGroup: group2
                        onCheckedChanged: {
                            if (checked) camprop.loadParametersD()
                            updatingCamera = true
                            camUpdating.start()
                        }
                    }

                    Button {
                        id: button1
                        text: qsTr("Save")
                        Layout.alignment: Qt.AlignCenter
                        onClicked: {
                            if (radioButton1.checked) camprop.saveParametersA()
                            else if (radioButton2.checked) camprop.saveParametersB()
                            else if (radioButton3.checked) camprop.saveParametersC()
                            else if (radioButton4.checked) camprop.saveParametersD()
                        }
                    }

                    Button {
                        id: button2
                        text: qsTr("Default")
                        Layout.alignment: Qt.AlignCenter
                        onClicked: {
                            camprop.loadDefaultParameters()
                            updateCamera()
                        }
                    }
                }

                TextItalic {
                    id: textItalic1
                    text: "Camera parameters are not automatically saved by application. To save parameters, please use provided Parameter Group above."
                    font.italic: true
                    anchors.top: gridlayout.bottom
                    anchors.topMargin: 25
                    anchors.right: gridlayout.right
                    anchors.left: gridlayout.left
                    wrapMode: Text.WordWrap
                }

                Button {
                    id: button3
                    x: 393
                    text: qsTr("One Shot WB")
                    anchors.top: preview.bottom
                    anchors.topMargin: 10
                    anchors.horizontalCenter: preview.horizontalCenter
                    onClicked: camprop.oneShotWB()
                }

            }

            Item {
                id: keyboardContent
                anchors.fill: parent
                visible: false

                GridLayout {
                    anchors.fill: parent
                    anchors.margins: 20
                    columns: 3

                    TableView {
                        id: keyboardView
                        model: _model
                        TableViewColumn { role: "command"; title: "Command"; width: 350 }
                        TableViewColumn { role: "shortcut"; title: "Shortcut"; width: 100 }
                        Layout.columnSpan: 3
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                    }
                    Button {
                        id: buttonKeyResetAll
                        text: "Reset All"
                        onClicked: {
                            for (var i = 0; i < _model.count; ++i) {
                                var defkey = _model.get(i).keys
                                _model.set(i, { "shortcut": defkey})
                            }
                        }
                    }
                    Item { width: 2 }
                    Item { width: 2 }
                    TextRegular { text: "Shortcut: " }
                    TextField {
                        id: fieldKeyShortcut
                        placeholderText: "Type to set shortcut"
                        echoMode: TextInput.Normal
                        readOnly: true
                        text: (keyboardView.currentRow >= 0) ? _model.get(keyboardView.currentRow).shortcut : ""
                        enabled: (keyboardView.selection.count > 0)
                        Layout.fillWidth: true

                        focus: fieldKeyShortcut.enabled
                        Keys.onPressed: procKeyEvent(event)
                    }
                    Button {
                        id: buttonKeyReset
                        text: "Reset"
                        enabled: (keyboardView.selection.count > 0)
                        onClicked: {
                            var defkey = _model.get(keyboardView.currentRow).keys
                            _model.set(keyboardView.currentRow, { "shortcut": defkey})
                        }
                    }
                }
            }
       }
    }

    states: [
        State {
            name: "keyboard settings"

            PropertyChanges {
                target: keyboardContent
                visible: true
            }

            PropertyChanges {
                target: cameraContent
                visible: false
            }
        }
    ]

    ListModel {
        id: _model

        // Set View
        ListElement {
            name: "View.Settings"
            command: "(Global) Open Settings Page"
            shortcut: "F2"
            keys: "F2"
        }
        ListElement {
            name: "View.HideRibbon"
            command: "(Global) Hide/show menu and status"
            shortcut: "F11"
            keys: "F11"
        }
        ListElement {
            name: "View.Fullscreen"
            command: "(Global) Toggle fullscreen"
            shortcut: "F12"
            keys: "F12"
        }

        // Camera
        ListElement {
            name: "WhiteBalance"
            command: "(Global) One shot white balance"
            shortcut: "Space"
            keys: "Space"
        }
        ListElement {
            name: "AutoExposure"
            command: "(Global) Toggle automatic exposure"
            shortcut: "E"
            keys: "E"
        }
        ListElement {
            name: "ParamsDefault"
            command: "(Global) Load default parameter group"
            shortcut: "F4"
            keys: "F4"
        }
        ListElement {
            name: "ParamsA"
            command: "(Global) Load parameter group A"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "ParamsB"
            command: "(Global) Load parameter group B"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "ParamsC"
            command: "(Global) Load parameter group C"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "ParamsD"
            command: "(Global) Load parameter group D"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "GammaUp"
            command: "(Global) Increase brightness"
            shortcut: "."
            keys: "."
        }
        ListElement {
            name: "GammaDown"
            command: "(Global) Decrease brightness"
            shortcut: ","
            keys: ","
        }
        ListElement {
            name: "ContrastUp"
            command: "(Global) Increase contrast"
            shortcut: "'"
            keys: "'"
        }
        ListElement {
            name: "ContrastDown"
            command: "(Global) Decrease contrast"
            shortcut: ";"
            keys: ";"
        }
        ListElement {
            name: "SaturationUp"
            command: "(Global) Increase saturation"
            shortcut: "Shift+>"
            keys: "Shift+>"
        }
        ListElement {
            name: "SaturationDown"
            command: "(Global) Decrease saturation"
            shortcut: "Shift+<"
            keys: "Shift+<"
        }
        ListElement {
            name: "TargetUp"
            command: "(Global) Increase auto exposure target area"
            shortcut: "]"
            keys: "]"
        }
        ListElement {
            name: "TargetDown"
            command: "(Global) Decrease auto exposure target area"
            shortcut: "["
            keys: "["
        }
        ListElement {
            name: "AetimeUp"
            command: "(Global) Increase exposure time"
            shortcut: "Shift+}"
            keys: "Shift+}"
        }
        ListElement {
            name: "AetimeDown"
            command: "(Global) Decrease exposure time"
            shortcut: "Shift+{"
            keys: "Shift+{"
        }
        ListElement {
            name: "GainUp"
            command: "(Global) Increase exposure gain"
            shortcut: 'Shift+"'
            keys: 'Shift+"'
        }
        ListElement {
            name: "GainDown"
            command: "(Global) Decrease exposure gain"
            shortcut: 'Shift+:'
            keys: 'Shift+:'
        }

        // Single View
        ListElement {
            name: "Single.SingleCapture"
            command: "(Camera) Single Capture"
            shortcut: "F5"
            keys: "F5"
        }
        ListElement {
            name: "Single.CountCapture"
            command: "(Camera) Start serial capture mode count"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.CountIntervalUp"
            command: "(Camera) Increase interval time for capture mode count"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.CountIntervalDown"
            command: "(Camera) Decrease interval time for capture mode count"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.CountUp"
            command: "(Camera) Increase frame count for capture mode count"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.CountDown"
            command: "(Camera) Decrease frame count for capture mode count"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.DurationCapture"
            command: "(Camera) Start serial capture mode duration"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.DurationIntervalUp"
            command: "(Camera) Increase interval time for capture mode duration"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.DurationIntervalDown"
            command: "(Camera) Decrease interval time for capture mode duration"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.DurationUp"
            command: "(Camera) Increase duration time for capture mode count"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.DurationDown"
            command: "(Camera) Decrease duration time for capture mode count"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.StartRecording"
            command: "(Camera) Start Recording"
            shortcut: "F6"
            keys: "F6"
        }
        ListElement {
            name: "Single.PauseRecording"
            command: "(Camera) Pause Recording"
            shortcut: ""
            keys: ""
        }
        ListElement {
            name: "Single.StopRecording"
            command: "(Camera) Stop Recording"
            shortcut: "F7"
            keys: "F7"
        }
    }

    function getIndex(name) {
        for (var i = 0; i < _model.count; ++i) {
            if (name === _model.get(i).name)
                return i
        }
        return -1
    }

    function procKeyEvent(event) {
        var rejectedKeys = [Qt.Key_unknown, Qt.Key_Control, Qt.Key_Alt, Qt.Key_Shift, Qt.Key_Escape,
                            Qt.Key_CapsLock, Qt.Key_NumLock, Qt.Key_AltGr, Qt.Key_ScrollLock, Qt.Key_Print,
                            Qt.Key_Pause, Qt.Key_Backspace, Qt.Key_Return, Qt.Key_Enter, Qt.Key_SysReq, Qt.Key_Clear,
                            Qt.Key_Meta, Qt.Key_Menu, Qt.Key_Tab]
        var invalidModifiers = [Qt.MetaModifier, Qt.KeypadModifier]
        for (var k in rejectedKeys) {
            if (event.key === rejectedKeys[k]) return
        }
        for (var m in invalidModifiers) {
            if (event.modifiers === invalidModifiers[m]) return
        }
        var key = appsettings.keyCodeToString(event.key + event.modifiers)
        for (var i = 0; i < _model.count; ++i) {
            if (_model.get(i).shortcut === key)
                _model.set(i, { "shortcut": "" })
        }
        _model.setProperty(keyboardView.currentRow, "shortcut", key)
        event.key.accepted = true
    }

    Component.onCompleted: {
        var lastParams = camprop.getCurrentParameterTeam()
        if (lastParams === 0) {
            radioButton1.checked = true
            camprop.loadParametersA()
        }
        else if (lastParams === 1) radioButton2.checked = true
        else if (lastParams === 2) radioButton3.checked = true
        else radioButton4.checked = true
        if (camprop.cameraType() === 1)
            camprop.setFrameRate(2);
        updateCamera()
    }

    Timer {
        id: aeUpdater
        repeat: true
        interval: 100
        running: checkboxAutoExposure.checked
        onTriggered: {
            sliderTime.value = camprop.exposureTime / 1000.0
            sliderGain.value = camprop.aeGain
        }
    }

    Timer {
        id: camUpdating
        interval: 1000
        onTriggered: updateCamera()
    }
}

