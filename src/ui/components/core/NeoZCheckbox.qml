import QtQuick
import QtQuick.Controls
import NeoZ

// NeoZCheckbox - Theme-aware checkbox with animation
CheckBox {
    id: control

    // Theme properties
    property color checkColor: Style.accentColor
    property color backgroundColor: Style.surface
    property color borderColor: Style.border

    // Indicator box
    indicator: Rectangle {
        implicitWidth: 22
        implicitHeight: 22
        x: control.leftPadding
        y: parent.height / 2 - height / 2
        radius: 4
        color: getBackgroundColor()
        border.color: getBorderColor()
        border.width: Style.borderWidth

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

        // Check mark
        Rectangle {
            id: checkMark
            anchors.centerIn: parent
            width: 12
            height: 12
            radius: 2
            color: Style.textPrimary
            visible: control.checked
            opacity: control.checked ? 1.0 : 0.0
            scale: control.checked ? 1.0 : 0.5

            Behavior on opacity {
                NumberAnimation {
                    duration: Style.motionDuration * Style.animationSpeed
                }
            }
            Behavior on scale {
                NumberAnimation {
                    duration: Style.motionDuration * Style.animationSpeed
                    easing.type: Easing.OutBack
                }
            }
        }

        // Checkmark icon (text-based for simplicity)
        Text {
            anchors.centerIn: parent
            text: "✓"
            font.pixelSize: 14
            font.bold: true
            color: control.checked ? Style.textPrimary : "transparent"
            visible: control.checked

            Behavior on color {
                ColorAnimation {
                    duration: Style.motionDuration * Style.animationSpeed
                }
            }
        }

        function getBackgroundColor() {
            if (!control.enabled)
                return Style.darkenColor(backgroundColor, 30);
            return control.checked ? checkColor : backgroundColor;
        }

        function getBorderColor() {
            if (control.checked)
                return Style.lightenColor(checkColor, 10);
            if (control.hovered)
                return Style.accentColor;
            return borderColor;
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
