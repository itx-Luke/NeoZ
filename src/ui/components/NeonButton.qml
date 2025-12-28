import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import NeoZ

Button {
    id: control
    text: "Button"

    property color accentColor: Style.primary
    property bool glow: true

    contentItem: Text {
        text: control.text
        color: Style.textPrimary // White text
        font: Style.bodyFont
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        radius: 8
        color: "transparent"

        // Liquid Gradient Background
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0.0
                color: control.down ? Qt.darker(control.accentColor, 1.2) : Qt.alpha(control.accentColor, 0.8)
            }
            GradientStop {
                position: 1.0
                color: control.down ? Qt.darker(control.accentColor, 1.5) : Qt.alpha(control.accentColor, 0.4)
            }
        }

        // Glass Border
        border.width: 1
        border.color: Qt.rgba(1, 1, 1, 0.3)

        // Inner Shine
        Rectangle {
            anchors.fill: parent
            anchors.margins: 1
            radius: parent.radius - 1
            color: "transparent"
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.1)
        }

        // Enhanced Glow effect
        layer.enabled: control.glow && !control.down && control.enabled
        layer.effect: MultiEffect {
            shadowEnabled: true
            shadowColor: control.accentColor
            shadowBlur: 1.5
            shadowOpacity: 0.8
            autoPaddingEnabled: true
        }
    }
}
