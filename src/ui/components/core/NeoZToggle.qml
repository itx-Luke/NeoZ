import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import NeoZ

// NeoZToggle - Theme-aware switch with smooth animation
Switch {
    id: control

    // Theme properties
    property color trackColor: Style.border
    property color activeTrackColor: Style.accentColor
    property color handleColor: Style.surface
    property color activeHandleColor: Style.accentColor

    implicitWidth: 50
    implicitHeight: 26

    // Track background
    indicator: Rectangle {
        id: trackRect
        implicitWidth: 50
        implicitHeight: 26
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: height / 2
        color: getTrackColor()
        border.color: Style.border
        border.width: 1

        Behavior on color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }

        // Handle
        Rectangle {
            id: handleRect
            x: control.checked ? parent.width - width - 2 : 2
            anchors.verticalCenter: parent.verticalCenter
            width: 22
            height: 22
            radius: 11
            color: getHandleColor()
            border.color: Style.lightenColor(handleColor, 20)
            border.width: 1

            // Inner highlight
            Rectangle {
                anchors.centerIn: parent
                width: parent.width - 6
                height: parent.height - 6
                radius: width / 2
                color: Qt.rgba(1, 1, 1, 0.3)
                opacity: 0.6
            }

            // Smooth position animation
            Behavior on x {
                NumberAnimation {
                    duration: Style.motionDuration * Style.animationSpeed
                    easing.type: Easing.InOutQuad
                }
            }

            Behavior on color {
                ColorAnimation {
                    duration: Style.motionDuration * Style.animationSpeed
                }
            }

            // Handle shadow/glow
            layer.enabled: Style.isGlass
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: control.checked ? activeHandleColor : Style.shadowColor
                shadowBlur: 0.8
                shadowOpacity: 0.4
                autoPaddingEnabled: true
            }
        }

        function getTrackColor() {
            if (!control.enabled)
                return Style.darkenColor(trackColor, 30);
            return control.checked ? activeTrackColor : trackColor;
        }

        function getHandleColor() {
            if (!control.enabled)
                return Style.textSecondary;
            return control.checked ? activeHandleColor : handleColor;
        }
    }

    // Label
    contentItem: Text {
        text: control.text
        font: Style.bodyFont
        color: control.enabled ? Style.textPrimary : Style.textSecondary
        verticalAlignment: Text.AlignVCenter
        leftPadding: control.indicator.width + 10
    }
}
