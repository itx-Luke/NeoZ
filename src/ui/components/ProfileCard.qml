import QtQuick
import QtQuick.Layouts
import "../style"

// Profile Selection Card (Gaming, Work, Battery, etc.)
Rectangle {
    id: root

    property string icon: "🎮"
    property string title: "Gaming Mode"
    property string description: "Max FPS, low latency"
    property bool active: false
    property color accentColor: CyberpunkColors.neonBlue

    signal clicked
    signal toggled(bool isActive)

    width: 180
    height: 100
    radius: 14

    color: mouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : CyberpunkColors.darkSlate
    border.width: active ? 2 : 1
    border.color: active ? accentColor : CyberpunkColors.glassBorder

    // Active glow
    Rectangle {
        visible: root.active
        anchors.fill: parent
        anchors.margins: -3
        radius: parent.radius + 3
        color: "transparent"
        border.width: 2
        border.color: Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.3)
        z: -1
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 14
        spacing: 12

        // Icon
        Rectangle {
            width: 44
            height: 44
            radius: 10
            color: Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.15)

            Text {
                anchors.centerIn: parent
                text: root.icon
                font.pixelSize: 22
            }
        }

        // Text content
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4

            Text {
                text: root.title
                color: CyberpunkColors.glowingWhite
                font.pixelSize: 14
                font.bold: true
            }

            Text {
                text: root.description
                color: CyberpunkColors.dimGray
                font.pixelSize: 11
                Layout.fillWidth: true
                elide: Text.ElideRight
            }
        }

        // Toggle switch
        Rectangle {
            width: 44
            height: 24
            radius: 12
            color: root.active ? accentColor : Qt.rgba(1, 1, 1, 0.1)

            Rectangle {
                width: 18
                height: 18
                radius: 9
                color: "#FFFFFF"
                anchors.verticalCenter: parent.verticalCenter
                x: root.active ? parent.width - width - 3 : 3

                Behavior on x {
                    NumberAnimation {
                        duration: 200
                        easing.type: Easing.OutCubic
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    root.active = !root.active;
                    root.toggled(root.active);
                }
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
        z: -1
    }

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
