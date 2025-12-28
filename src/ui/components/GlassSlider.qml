import QtQuick
import "../style"

// GlassSlider - Premium glass slider with trail blur effect
// Supports center-zero mode with cyan/amber bidirectional colors
Rectangle {
    id: root

    property real value: 0.5
    property real from: 0
    property real to: 1
    property bool centerZero: false
    property string label: ""
    property string unit: ""
    property int decimals: 2
    property color positiveColor: "#6EEBFF"  // Ice cyan
    property color negativeColor: "#FFAB40"  // Muted amber
    property color neutralColor: "#E6EAF0"   // Frosted white

    implicitWidth: 200
    implicitHeight: 44
    radius: 10
    color: Qt.rgba(0.08, 0.1, 0.16, 0.7)
    border.color: Qt.rgba(1, 1, 1, 0.1)
    border.width: 1
    clip: true  // Prevent overflow

    // Ambient glow behind
    Rectangle {
        anchors.fill: parent
        anchors.margins: -8
        radius: parent.radius + 8
        color: "transparent"
        border.color: positiveColor
        border.width: 3
        opacity: 0.08
        z: -1
    }

    // Top glass shine
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 4
        anchors.rightMargin: 4
        anchors.topMargin: 1
        height: 1
        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 0.3
                color: Qt.rgba(1, 1, 1, 0.2)
            }
            GradientStop {
                position: 0.5
                color: Qt.rgba(1, 1, 1, 0.35)
            }
            GradientStop {
                position: 0.7
                color: Qt.rgba(1, 1, 1, 0.2)
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    // Track background
    Rectangle {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.verticalCenter: parent.verticalCenter
        anchors.leftMargin: 16
        anchors.rightMargin: 16
        height: 6
        radius: 3
        color: Qt.rgba(1, 1, 1, 0.08)

        // Center line for center-zero mode
        Rectangle {
            visible: root.centerZero
            anchors.centerIn: parent
            width: 2
            height: parent.height + 8
            radius: 1
            color: neutralColor
            opacity: 0.6
        }

        // Fill visualization
        Rectangle {
            id: fillRect
            height: parent.height
            radius: 3

            property real centerX: parent.width / 2
            property real thumbX: Math.max(0, Math.min(1, (root.value - root.from) / (root.to - root.from))) * parent.width

            x: root.centerZero ? Math.min(centerX, thumbX) : 0
            width: root.centerZero ? Math.abs(thumbX - centerX) : thumbX

            property color fillColor: {
                if (!root.centerZero)
                    return positiveColor;
                return root.value >= (root.from + root.to) / 2 ? positiveColor : negativeColor;
            }

            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(fillRect.fillColor.r, fillRect.fillColor.g, fillRect.fillColor.b, 0.5)
                }
                GradientStop {
                    position: 1.0
                    color: Qt.rgba(fillRect.fillColor.r, fillRect.fillColor.g, fillRect.fillColor.b, 0.9)
                }
            }

            Behavior on x {
                NumberAnimation {
                    duration: 60
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on width {
                NumberAnimation {
                    duration: 60
                    easing.type: Easing.OutCubic
                }
            }
        }

        // Thumb glow
        Rectangle {
            id: thumbGlow
            width: 32
            height: 32
            radius: 16
            anchors.verticalCenter: parent.verticalCenter
            x: thumb.x + thumb.width / 2 - width / 2
            color: thumb.color
            opacity: dragArea.pressed ? 0.4 : 0.15

            Behavior on opacity {
                NumberAnimation {
                    duration: 150
                }
            }
        }

        // Thumb
        Rectangle {
            id: thumb
            width: 18
            height: 18
            radius: 9
            anchors.verticalCenter: parent.verticalCenter
            x: Math.max(0, Math.min(1, (root.value - root.from) / (root.to - root.from))) * (parent.width - width)

            color: {
                if (!root.centerZero)
                    return positiveColor;
                var mid = (root.from + root.to) / 2;
                if (root.value > mid)
                    return positiveColor;
                if (root.value < mid)
                    return negativeColor;
                return neutralColor;
            }

            border.color: Qt.rgba(1, 1, 1, 0.5)
            border.width: 1

            // Inner highlight
            Rectangle {
                anchors.centerIn: parent
                width: parent.width - 6
                height: parent.height - 6
                radius: width / 2
                color: Qt.rgba(1, 1, 1, 0.35)
            }

            Behavior on x {
                NumberAnimation {
                    duration: 60
                    easing.type: Easing.OutCubic
                }
            }
            Behavior on color {
                ColorAnimation {
                    duration: 120
                }
            }

            scale: dragArea.pressed ? 1.2 : 1.0
            Behavior on scale {
                NumberAnimation {
                    duration: 120
                    easing.type: Easing.OutCubic
                }
            }
        }

        // Drag area
        MouseArea {
            id: dragArea
            anchors.fill: parent
            anchors.margins: -14

            onPressed: function (mouse) {
                updateValue(mouse.x + 14);
            }
            onPositionChanged: function (mouse) {
                if (pressed)
                    updateValue(mouse.x + 14);
            }

            function updateValue(mouseX) {
                var normalX = Math.max(0, Math.min(mouseX, track.width));
                root.value = root.from + (normalX / track.width) * (root.to - root.from);
            }
        }
    }

    // Value display on right
    Text {
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.verticalCenter: parent.verticalCenter
        text: root.value.toFixed(root.decimals) + root.unit
        color: thumb.color
        font.pixelSize: 11
        font.bold: true
        opacity: 0.9
    }

    // Hover detection
    MouseArea {
        id: hoverArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.NoButton
    }
}
