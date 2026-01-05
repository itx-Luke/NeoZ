import QtQuick
import QtQuick.Layouts
import "../style"

// Quick Optimization Tile (BlueStacks, RAM, FPS, Security)
Rectangle {
    id: root

    property string icon: "🎮"
    property string title: "BLUESTACKS"
    property string subtitle: "Emulator Optimizer"
    property string statusText: "ACTIVE"
    property bool isActive: true
    property color accentColor: CyberpunkColors.neonBlue

    signal clicked

    width: 200
    height: 140
    radius: 16
    color: CyberpunkColors.darkSlate
    border.width: 1
    border.color: mouseArea.containsMouse ? accentColor : CyberpunkColors.glassBorder

    // Animated glow border
    Rectangle {
        anchors.fill: parent
        anchors.margins: -2
        radius: parent.radius + 2
        color: "transparent"
        border.width: 2
        border.color: Qt.rgba(accentColor.r, accentColor.g, accentColor.b, pulseAnim.running ? pulseOpacity : 0.15)
        z: -1

        property real pulseOpacity: 0.15

        SequentialAnimation {
            id: pulseAnim
            running: root.isActive
            loops: Animation.Infinite
            NumberAnimation {
                target: parent
                property: "pulseOpacity"
                to: 0.5
                duration: 1000
                easing.type: Easing.InOutSine
            }
            NumberAnimation {
                target: parent
                property: "pulseOpacity"
                to: 0.15
                duration: 1000
                easing.type: Easing.InOutSine
            }
        }
    }

    // Glass highlight
    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: Qt.rgba(1, 1, 1, 0.06)
            }
            GradientStop {
                position: 0.2
                color: "transparent"
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 10

        // Header row
        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            // Animated icon
            Rectangle {
                width: 42
                height: 42
                radius: 12
                gradient: Gradient {
                    orientation: Gradient.Vertical
                    GradientStop {
                        position: 0
                        color: Qt.lighter(accentColor, 1.2)
                    }
                    GradientStop {
                        position: 1
                        color: accentColor
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: root.icon
                    font.pixelSize: 20
                }

                // Subtle rotation for activity
                RotationAnimation on rotation {
                    running: root.isActive && root.icon === "⚡"
                    from: -5
                    to: 5
                    duration: 500
                    loops: Animation.Infinite
                    easing.type: Easing.InOutSine
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Text {
                    text: root.title
                    color: CyberpunkColors.glowingWhite
                    font.pixelSize: 13
                    font.bold: true
                    font.letterSpacing: 1
                }

                Text {
                    text: root.subtitle
                    color: CyberpunkColors.dimGray
                    font.pixelSize: 10
                }
            }
        }

        Item {
            Layout.fillHeight: true
        }

        // Status row
        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            // Status indicator
            Rectangle {
                width: 8
                height: 8
                radius: 4
                color: root.isActive ? CyberpunkColors.matrixGreen : CyberpunkColors.dimGray

                SequentialAnimation on opacity {
                    running: root.isActive
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 0.4
                        duration: 600
                    }
                    NumberAnimation {
                        to: 1.0
                        duration: 600
                    }
                }
            }

            Text {
                text: root.statusText
                color: root.isActive ? CyberpunkColors.matrixGreen : CyberpunkColors.dimGray
                font.pixelSize: 11
                font.bold: true
                font.letterSpacing: 1
            }

            Item {
                Layout.fillWidth: true
            }

            // Arrow indicator
            Text {
                text: "→"
                color: CyberpunkColors.dimGray
                font.pixelSize: 16
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: root.clicked()
    }

    // Hover scale
    scale: mouseArea.containsMouse ? 1.02 : 1.0
    Behavior on scale {
        NumberAnimation {
            duration: 150
            easing.type: Easing.OutCubic
        }
    }
    Behavior on border.color {
        ColorAnimation {
            duration: 150
        }
    }
}
