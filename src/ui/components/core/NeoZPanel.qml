import QtQuick
import QtQuick.Effects
import NeoZ

// NeoZPanel - Theme-aware container with glass/minimal variants
Rectangle {
    id: panel

    // Content default property
    default property alias content: contentContainer.data

    // Theme properties
    property color backgroundColor: Style.surface
    property color borderColor: Style.border
    property int cornerRadius: Style.cornerRadius
    property int borderWidth: Style.borderWidth
    property real panelOpacity: Style.containerOpacity
    property bool enableShadow: true
    property bool enableGlass: Style.isGlass
    property real glassOpacity: 0.85
    property color glowColor: Style.primary
    property bool enableGlow: Style.glowEnabled

    implicitWidth: 200
    implicitHeight: 150
    radius: cornerRadius
    color: getBackgroundColor()
    border.width: borderWidth
    border.color: enableGlass ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(1, 1, 1, 0.08)

    Behavior on color {
        ColorAnimation {
            duration: Style.motionDuration * Style.animationSpeed
        }
    }
    Behavior on radius {
        NumberAnimation {
            duration: Style.motionDuration * Style.animationSpeed
        }
    }

    // Glow effect (glass only)
    layer.enabled: enableGlow && enableGlass
    layer.smooth: true
    layer.effect: MultiEffect {
        shadowEnabled: true
        shadowColor: glowColor
        shadowBlur: 1.2
        shadowOpacity: 0.4
        autoPaddingEnabled: true
    }

    // Top 3D shine highlight (glass mode)
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
        visible: enableGlass

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
            }
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

    // Secondary glow below shine
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topShine.bottom
        anchors.leftMargin: 8
        anchors.rightMargin: 8
        height: 12
        visible: enableGlass

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
        visible: enableGlass
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.06)
    }

    // Dark minimal top highlight line
    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: 1
        color: Qt.rgba(1, 1, 1, 0.08)
        visible: !enableGlass
    }

    // Content container
    Item {
        id: contentContainer
        anchors.fill: parent
        anchors.margins: Style.spacing
        z: 10
    }

    function getBackgroundColor() {
        if (enableGlass) {
            return Qt.rgba(0.08, 0.07, 0.14, glassOpacity);
        }
        return Qt.rgba(1, 1, 1, 0.04);
    }
}
