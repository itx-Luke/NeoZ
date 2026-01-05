import QtQuick
import "../style"

// Circular Radar Visualization for Threats
Rectangle {
    id: root

    property int threatsBlocked: 47
    property int activeConnections: 12

    width: 180
    height: 180
    radius: width / 2
    color: CyberpunkColors.deepSpace
    border.width: 2
    border.color: CyberpunkColors.glassBorder

    // Radar rings
    Repeater {
        model: 3
        Rectangle {
            anchors.centerIn: parent
            width: parent.width * (0.3 + index * 0.25)
            height: width
            radius: width / 2
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(0, 0.8, 1, 0.15)
        }
    }

    // Cross lines
    Rectangle {
        anchors.centerIn: parent
        width: 1
        height: parent.height - 20
        color: Qt.rgba(0, 0.8, 1, 0.2)
    }
    Rectangle {
        anchors.centerIn: parent
        width: parent.width - 20
        height: 1
        color: Qt.rgba(0, 0.8, 1, 0.2)
    }

    // Rotating sweep line
    Rectangle {
        id: sweepLine
        anchors.centerIn: parent
        width: 2
        height: parent.height / 2 - 10
        transformOrigin: Item.Bottom
        y: -height / 2

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: CyberpunkColors.neonBlue
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }

        RotationAnimation on rotation {
            from: 0
            to: 360
            duration: 3000
            loops: Animation.Infinite
        }
    }

    // Sweep glow trail
    Canvas {
        id: sweepGlow
        anchors.fill: parent
        opacity: 0.3

        property real angle: sweepLine.rotation * Math.PI / 180

        Connections {
            target: sweepLine
            function onRotationChanged() {
                sweepGlow.angle = sweepLine.rotation * Math.PI / 180;
                sweepGlow.requestPaint();
            }
        }

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();

            var cx = width / 2;
            var cy = height / 2;
            var radius = width / 2 - 15;

            // Create gradient arc behind sweep
            ctx.beginPath();
            ctx.moveTo(cx, cy);
            ctx.arc(cx, cy, radius, angle - 0.5, angle, false);
            ctx.closePath();

            var gradient = ctx.createRadialGradient(cx, cy, 0, cx, cy, radius);
            gradient.addColorStop(0, "transparent");
            gradient.addColorStop(1, "#00D4FF");
            ctx.fillStyle = gradient;
            ctx.fill();
        }
    }

    // Safe connection dots (green)
    Repeater {
        model: 5
        Rectangle {
            property real angle: Math.random() * Math.PI * 2
            property real dist: 30 + Math.random() * 40

            x: parent.width / 2 + Math.cos(angle) * dist - width / 2
            y: parent.height / 2 + Math.sin(angle) * dist - height / 2
            width: 6
            height: 6
            radius: 3
            color: CyberpunkColors.matrixGreen

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 0.4
                    duration: 800 + Math.random() * 400
                }
                NumberAnimation {
                    to: 1.0
                    duration: 800 + Math.random() * 400
                }
            }
        }
    }

    // Threat dots (red) - blocked
    Repeater {
        model: 3
        Rectangle {
            property real angle: Math.random() * Math.PI * 2
            property real dist: 50 + Math.random() * 25

            x: parent.width / 2 + Math.cos(angle) * dist - width / 2
            y: parent.height / 2 + Math.sin(angle) * dist - height / 2
            width: 6
            height: 6
            radius: 3
            color: CyberpunkColors.dangerRed

            SequentialAnimation on opacity {
                loops: Animation.Infinite
                NumberAnimation {
                    to: 0.3
                    duration: 400
                }
                NumberAnimation {
                    to: 1.0
                    duration: 400
                }
            }
        }
    }

    // Center dot
    Rectangle {
        anchors.centerIn: parent
        width: 12
        height: 12
        radius: 6
        color: CyberpunkColors.neonBlue

        SequentialAnimation on scale {
            loops: Animation.Infinite
            NumberAnimation {
                to: 0.8
                duration: 500
            }
            NumberAnimation {
                to: 1.0
                duration: 500
            }
        }
    }

    // Counter overlay
    Rectangle {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 8
        width: counterText.width + 16
        height: 20
        radius: 10
        color: Qt.rgba(0, 0, 0, 0.7)

        Text {
            id: counterText
            anchors.centerIn: parent
            text: "🛡️ " + root.threatsBlocked + " blocked"
            color: CyberpunkColors.matrixGreen
            font.pixelSize: 10
            font.bold: true
        }
    }
}
