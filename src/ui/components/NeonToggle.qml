import QtQuick
import QtQuick.Controls
import NeoZ

// Custom Toggle Switch with neon styling
Item {
    id: root
    width: 50
    height: 24

    property bool checked: false
    property color activeColor: Style.primary
    property color inactiveColor: Qt.rgba(1, 1, 1, 0.2)

    signal toggled(bool value)

    Rectangle {
        id: track
        anchors.fill: parent
        radius: height / 2
        color: root.checked ? Qt.rgba(root.activeColor.r, root.activeColor.g, root.activeColor.b, 0.3) : root.inactiveColor
        border.color: root.checked ? root.activeColor : Qt.rgba(1, 1, 1, 0.3)
        border.width: 1

        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
        Behavior on border.color {
            ColorAnimation {
                duration: 150
            }
        }
    }

    Rectangle {
        id: handle
        width: 18
        height: 18
        radius: 9
        x: root.checked ? parent.width - width - 3 : 3
        anchors.verticalCenter: parent.verticalCenter
        color: root.checked ? root.activeColor : Qt.rgba(1, 1, 1, 0.6)

        Behavior on x {
            NumberAnimation {
                duration: 150
                easing.type: Easing.OutQuad
            }
        }
        Behavior on color {
            ColorAnimation {
                duration: 150
            }
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.checked ? "ON" : "OFF"
        color: root.checked ? root.activeColor : Style.textSecondary
        font.pixelSize: 9
        font.bold: true
        visible: false // Optional: set to true for text inside toggle
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.checked = !root.checked;
            root.toggled(root.checked);
        }
    }
}
