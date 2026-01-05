import QtQuick
import "../style"

// Orbiting Monitoring Orb (CPU, RAM, Disk, etc.)
Rectangle {
    id: root

    property string label: "CPU"
    property string icon: "🧠"
    property real value: 65
    property color orbColor: CyberpunkColors.cpuOrb
    property real orbitRadius: 80
    property real orbitSpeed: 8000 // ms for full orbit
    property real startAngle: 0

    width: 52
    height: 52
    radius: width / 2
    color: "transparent"

    // Orbital position calculation
    property real currentAngle: startAngle

    x: orbitRadius * Math.cos(currentAngle * Math.PI / 180)
    y: orbitRadius * Math.sin(currentAngle * Math.PI / 180)

    NumberAnimation on currentAngle {
        from: startAngle
        to: startAngle + 360
        duration: orbitSpeed
        loops: Animation.Infinite
    }

    // Outer glow
    Rectangle {
        anchors.centerIn: parent
        width: parent.width + 8
        height: width
        radius: width / 2
        color: "transparent"
        border.width: 2
        border.color: Qt.rgba(orbColor.r, orbColor.g, orbColor.b, 0.3)

        SequentialAnimation on border.color {
            loops: Animation.Infinite
            ColorAnimation {
                to: Qt.rgba(orbColor.r, orbColor.g, orbColor.b, 0.6)
                duration: 1000
            }
            ColorAnimation {
                to: Qt.rgba(orbColor.r, orbColor.g, orbColor.b, 0.3)
                duration: 1000
            }
        }
    }

    // Main orb body
    Rectangle {
        id: orbBody
        anchors.fill: parent
        radius: width / 2

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.lighter(orbColor, 1.4)
            }
            GradientStop {
                position: 0.5
                color: orbColor
            }
            GradientStop {
                position: 1.0
                color: Qt.darker(orbColor, 1.3)
            }
        }

        // Inner highlight
        Rectangle {
            width: parent.width * 0.6
            height: width
            radius: width / 2
            x: parent.width * 0.15
            y: parent.height * 0.1

            gradient: Gradient {
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(1, 1, 1, 0.4)
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
                }
            }
        }
    }

    // Icon
    Text {
        anchors.centerIn: parent
        text: root.icon
        font.pixelSize: 18
    }

    // Value badge
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: -8
        width: 32
        height: 16
        radius: 8
        color: CyberpunkColors.darkCharcoal
        border.width: 1
        border.color: orbColor

        Text {
            anchors.centerIn: parent
            text: Math.round(root.value) + "%"
            color: orbColor
            font.pixelSize: 9
            font.bold: true
        }
    }

    // Hover tooltip
    Rectangle {
        id: tooltip
        visible: mouseArea.containsMouse
        anchors.top: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.topMargin: 12
        width: tooltipText.width + 16
        height: 24
        radius: 6
        color: Qt.rgba(0, 0, 0, 0.9)
        border.width: 1
        border.color: orbColor
        z: 100

        Text {
            id: tooltipText
            anchors.centerIn: parent
            text: root.label + ": " + Math.round(root.value) + "%"
            color: CyberpunkColors.glowingWhite
            font.pixelSize: 11
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
    }

    // Pulse animation on high values
    SequentialAnimation on scale {
        running: root.value > 80
        loops: Animation.Infinite
        NumberAnimation {
            to: 1.1
            duration: 500
        }
        NumberAnimation {
            to: 1.0
            duration: 500
        }
    }
}
