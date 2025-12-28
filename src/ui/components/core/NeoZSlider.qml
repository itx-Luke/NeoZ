import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import NeoZ

// NeoZSlider - Theme-aware slider with glow effects
Slider {
    id: control

    // Theme properties
    property color trackColor: Style.border
    property color handleColor: Style.accentColor
    property color progressColor: Style.accentColor
    property bool enableGlow: Style.glowEnabled

    implicitWidth: 200
    implicitHeight: 24

    // Background track
    background: Rectangle {
        x: control.leftPadding
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: control.availableWidth
        height: 6
        radius: 3
        color: Style.darkenColor(Style.surface, 10)
        border.color: trackColor
        border.width: 1

        // Progress fill
        Rectangle {
            width: control.visualPosition * parent.width
            height: parent.height
            radius: parent.radius
            color: progressColor

            // Gradient overlay
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
                    duration: 60
                    easing.type: Easing.OutCubic
                }
            }
        }

        Behavior on color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }
    }

    // Handle
    handle: Rectangle {
        id: handleRect
        x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
        y: control.topPadding + control.availableHeight / 2 - height / 2
        width: 18
        height: 18
        radius: 9
        color: getHandleColor()
        border.color: Style.lightenColor(handleColor, 20)
        border.width: 2

        // Inner highlight
        Rectangle {
            anchors.centerIn: parent
            width: parent.width - 6
            height: parent.height - 6
            radius: width / 2
            color: Qt.rgba(1, 1, 1, 0.35)
        }

        // Scale on press
        scale: control.pressed ? 1.15 : 1.0
        Behavior on scale {
            NumberAnimation {
                duration: 100
                easing.type: Easing.OutCubic
            }
        }

        Behavior on color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }

        // Glow effect
        layer.enabled: control.enableGlow && Style.isGlass
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: handleColor
            shadowBlur: control.hovered ? 1.2 : 0.8
            shadowOpacity: control.hovered ? 0.6 : 0.3
            autoPaddingEnabled: true
        }

        function getHandleColor() {
            if (!control.enabled)
                return Style.textSecondary;
            if (control.pressed)
                return Style.darkenColor(handleColor, 15);
            if (control.hovered)
                return Style.lightenColor(handleColor, 10);
            return handleColor;
        }
    }
}
