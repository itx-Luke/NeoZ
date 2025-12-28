import QtQuick
import NeoZ

// NeoZProgressBar - Theme-aware progress indicator
Item {
    id: control

    property real value: 0.5  // 0.0 to 1.0
    property real from: 0.0
    property real to: 1.0
    property color trackColor: Style.border
    property color progressColor: Style.accentColor
    property bool indeterminate: false

    implicitWidth: 200
    implicitHeight: 8

    // Track background
    Rectangle {
        id: track
        anchors.fill: parent
        radius: height / 2
        color: Style.darkenColor(Style.surface, 10)
        border.color: trackColor
        border.width: 1

        Behavior on color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }
    }

    // Progress fill
    Rectangle {
        id: progressFill
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: control.indeterminate ? parent.width * 0.3 : normalizedValue * parent.width
        radius: track.radius
        visible: !control.indeterminate

        property real normalizedValue: Math.max(0, Math.min(1, (control.value - control.from) / (control.to - control.from)))

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: Qt.alpha(progressColor, 0.6)
            }
            GradientStop {
                position: 1.0
                color: progressColor
            }
        }

        Behavior on width {
            NumberAnimation {
                duration: 150 * Style.animationSpeed
                easing.type: Easing.OutCubic
            }
        }
    }

    // Indeterminate animation bar
    Rectangle {
        id: indeterminateBar
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        width: parent.width * 0.3
        radius: track.radius
        visible: control.indeterminate
        x: 0

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: Qt.alpha(progressColor, 0.4)
            }
            GradientStop {
                position: 0.5
                color: progressColor
            }
            GradientStop {
                position: 1.0
                color: Qt.alpha(progressColor, 0.4)
            }
        }

        SequentialAnimation on x {
            running: control.indeterminate
            loops: Animation.Infinite
            NumberAnimation {
                from: -indeterminateBar.width
                to: control.width
                duration: 1500
                easing.type: Easing.InOutQuad
            }
        }
    }

    // Glass shine
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 2
        anchors.rightMargin: 2
        height: 1
        radius: track.radius
        visible: Style.isGlass

        gradient: Gradient {
            orientation: Gradient.Horizontal
            GradientStop {
                position: 0.0
                color: "transparent"
            }
            GradientStop {
                position: 0.5
                color: Qt.rgba(1, 1, 1, 0.2)
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }
}
