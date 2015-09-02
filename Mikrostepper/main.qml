import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import Leben.Asa 1.0

LebenWindow {
    id: win
    visible: true
    width: 1280
    height: 720
    title: qsTr("Microstepper")
//    x: (Screen.width - width) / 2
//    y: (Screen.height - height) / 2

    property rect _lastRect
    property int _winstate: 0
    function maximize() {
        _lastRect = Qt.rect(win.x, win.y, win.width, win.height)
//        var _w = Screen.desktopAvailableWidth
//        var _h = Screen.desktopAvailableHeight
//        var _x = (_w == Screen.width) ? 0 : Screen.width - _w
//        var _y = (_h == Screen.height) ? 0 : Screen.height - _h
        var _w = Screen.width - 1
        var _h = Screen.height - 1
        var _x = 0; var _y = 0;
        win.x = _x; win.y = _y
        win.width = _w; win.height = _h
        _winstate = 1
    }

    function restore() {
        win.x = _lastRect.x; win.y = _lastRect.y
        win.width = _lastRect.width; win.height = _lastRect.height
        _winstate = 0
    }

    function minimize() {
//        if (_winstate == 1) {
//            restore()
//            _winstate = 0
//        }
        win.showMinimized()
    }

    Item {
        id: base
        anchors.fill: parent
        Rectangle {
            id: titleBar
            height: 25
            color: "white"
            anchors {
                right: parent.right; left: parent.left; top: parent.top
            }
            z: 90
            RowLayout {
                anchors.fill: parent
                TextRegular {
                    id: title
                    Layout.fillWidth: true
                    text: "MICONOS Optilab"
                    font.pointSize: 10
                    font.letterSpacing: 1
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    color: "#2c3e50"
                }
                ToolButton {
                    id: hideRibbonBtn
                    iconSource: {
                        if (mainForm.state == "")
                            hovered ? "Images/HideRibbon-hover.png" : "Images/HideRibbon.png"
                        else
                            hovered ? "Images/ShowRibbon-hover.png" : "Images/ShowRibbon.png"
                    }
                    onClicked: mainForm.hideRibbon()
                }
                ToolButton {
                    id: minimizeBtn
                    iconSource: hovered ? "Images/MinimizeHover.png" : "Images/Minimize.png"
                    onClicked: win.minimize()
                }
                ToolButton {
                    id: maximizeBtn
                    iconSource: {
                        if (win._winstate == 1)
                            hovered ? "Images/RestoreHover.png" : "Images/Restore.png"
                        else
                            hovered ? "Images/MaximizeHover.png" : "Images/Maximize.png"
                    }
                    onClicked: {
                        if (win._winstate == 1)
                            win.restore()
                        else
                            win.maximize()
                    }
                }
                ToolButton {
                    id: closeBtn
                    iconSource: hovered ? "Images/CloseHover.png" : "Images/Close.png"
                    onClicked: Qt.quit()
                }
            }
        }
        MainForm {
            id: mainForm
            anchors {
                right: parent.right; left: parent.left
                top: titleBar.bottom; bottom: parent.bottom
                topMargin: 5
            }
            onFullscreen: {
                if (win.visibility === Window.FullScreen) {
                    win.showNormal()
                }
                else
                    win.showFullScreen()
            }
        }
    }

    Component.onCompleted: {
//        win.showMaximized()
    }
}
