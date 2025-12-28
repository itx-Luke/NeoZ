import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import NeoZ

// NeoZButton - Base theme-aware button with state-based styling
Button {
    id: control

    // Theme properties
    property color backgroundColor: Style.primary
    property color textColor: Style.textPrimary
    property color borderColor: Style.border
    property bool enableGlow: Style.glowEnabled
    property string styleId: "primary" // "primary", "secondary", "ghost", "danger"

    implicitWidth: 120
    implicitHeight: 40

    contentItem: Text {
        text: control.text
        color: getTextColor()
        font: Style.bodyFont
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        Behavior on color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }
    }

    background: Rectangle {
        implicitWidth: control.implicitWidth
        implicitHeight: control.implicitHeight
        radius: Style.cornerRadius
        color: getBackgroundColor()
        border.color: getBorderColor()
        border.width: Style.borderWidth

        // Glass effect top shine
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            anchors.topMargin: 1
            height: 1
            radius: parent.radius
            visible: Style.isGlass
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 0.3
                    color: Qt.rgba(1, 1, 1, 0.15)
                }
                GradientStop {
                    position: 0.5
                    color: Qt.rgba(1, 1, 1, 0.25)
                }
                GradientStop {
                    position: 0.7
                    color: Qt.rgba(1, 1, 1, 0.15)
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
                }
            }
        }

        Behavior on color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }
        Behavior on border.color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }

        // Glow effect
        layer.enabled: control.enableGlow && !control.pressed && control.enabled && Style.isGlass
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: control.backgroundColor
            shadowBlur: 1.0
            shadowOpacity: 0.5
            autoPaddingEnabled: true
        }
    }

    function getBackgroundColor() {
        var baseColor = backgroundColor;

        // Style variants
        if (styleId === "secondary")
            baseColor = Style.surface;
        else if (styleId === "ghost")
            baseColor = "transparent";
        else if (styleId === "danger")
            baseColor = Style.danger;

        if (!control.enabled)
            return Style.darkenColor(baseColor, 50);
        if (control.pressed)
            return Style.darkenColor(baseColor, 20);
        if (control.hovered)
            return Style.lightenColor(baseColor, 10);
        return baseColor;
    }

    function getTextColor() {
        if (!control.enabled)
            return Style.textSecondary;
        if (styleId === "ghost")
            return Style.primary;
        return textColor;
    }

    function getBorderColor() {
        if (styleId === "ghost")
            return control.hovered ? Style.primary : Style.border;
        return borderColor;
    }
}
