import QtQuick
import QtQuick.Effects
import NeoZ
import "../style"

// GlassPanel - Liquid Glass Container with Dynamic Island 3D Shine
// No internal animations - just static frosted glass with 3D shine border
Rectangle {
    id: panel

    default property alias content: contentContainer.data

    // Skin-reactive properties
    property real glassOpacity: Style.containerOpacity
    property bool glowEnabled: Style.glowEnabled
    property color glowColor: Style.primary
    property color borderColor: Style.surfaceHighlight
    property real borderWidth: 1

    // Dynamic radius based on theme
    radius: Style.isGlass ? 16 : 8

    // Frosted glass base color (no animations)
    color: Style.isGlass ? Qt.rgba(0.08, 0.07, 0.14, glassOpacity)  // Dark solid purple with dynamic opacity
    : Qt.rgba(1, 1, 1, 0.04)

    // Standard border
    border.width: panel.borderWidth
    border.color: Style.isGlass ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(1, 1, 1, 0.08)

    // Smooth theme transitions
    Behavior on color {
        ColorAnimation {
            duration: 300
        }
    }
    Behavior on radius {
        NumberAnimation {
            duration: 300
        }
    }

    // Layer effects for glow (glass only)
    layer.enabled: panel.glowEnabled && Style.isGlass
    layer.smooth: true
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: panel.glowColor
        shadowBlur: 1.2
        shadowOpacity: 0.4
        autoPaddingEnabled: true
    }

    // ===========================================
    // 3D SHINE BORDER (Dynamic Island Style)
    // Static highlight on top edge - no animation
    // ===========================================

    // Top 3D shine highlight (main effect)
    Rectangle {
        id: topShine
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 2
        anchors.rightMargin: 2
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
                position: 0.2
                color: Qt.rgba(1, 1, 1, 0.25)
            }
            GradientStop {
                position: 0.5
                color: Qt.rgba(1, 1, 1, 0.5)
            }  // Bright center
            GradientStop {
                position: 0.8
                color: Qt.rgba(1, 1, 1, 0.25)
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    // Secondary softer glow below the shine
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topShine.bottom
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        height: 12
        visible: Style.isGlass

        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0.0
                color: Qt.rgba(1, 1, 1, 0.08)
            }
            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    // Inner border glow (subtle)
    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        radius: parent.radius - 1
        color: "transparent"
        visible: Style.isGlass
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.06)
    }

    // ===========================================
    // DARK MINIMAL SKIN (Simple, no effects)
    // ===========================================
    Rectangle {
        id: darkMinimalBase
        anchors.fill: parent
        radius: parent.radius
        visible: !Style.isGlass
        color: "transparent"

        // Simple top highlight line
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 1
            color: Qt.rgba(1, 1, 1, 0.08)
        }
    }

    // ===========================================
    // CONTENT LAYER
    // ===========================================
    Item {
        id: contentContainer
        anchors.fill: parent
        anchors.margins: Style.spacing
        z: 10  // Above glass effects
    }
}
