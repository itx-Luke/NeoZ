import QtQuick
import QtQuick.Controls
import QtQuick.Effects
import NeoZ

// NeoZComboBox - Enhanced theme-aware dropdown
ComboBox {
    id: control

    // Theme properties
    property color backgroundColor: Style.surface
    property color borderColor: Style.border
    property color textColor: Style.textPrimary
    property color highlightColor: Style.accentColor
    property color popupBackground: Style.darkenColor(Style.background, 5)

    implicitWidth: 140
    implicitHeight: 40

    // Delegate items
    delegate: ItemDelegate {
        id: delegateItem
        width: control.width
        height: 36

        contentItem: Text {
            text: modelData
            color: delegateItem.highlighted ? highlightColor : Style.textSecondary
            font: Style.bodyFont
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            leftPadding: 12
        }

        background: Rectangle {
            color: delegateItem.highlighted ? Qt.alpha(highlightColor, 0.15) : "transparent"

            Behavior on color {
                ColorAnimation {
                    duration: 100 * Style.animationSpeed
                }
            }
        }

        highlighted: control.highlightedIndex === index
    }

    // Arrow indicator
    indicator: Canvas {
        id: arrowCanvas
        x: control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        rotation: control.popup.visible ? 180 : 0
        Behavior on rotation {
            NumberAnimation {
                duration: Style.motionDuration * Style.animationSpeed
                easing.type: Easing.InOutQuad
            }
        }

        Connections {
            target: control
            function onPressedChanged() {
                arrowCanvas.requestPaint();
            }
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = control.pressed ? highlightColor : Style.textSecondary;
            context.fill();
        }
    }

    // Content display
    contentItem: Text {
        leftPadding: 12
        rightPadding: control.indicator.width + control.spacing + 8

        text: control.displayText
        font: Style.bodyFont
        color: control.pressed ? highlightColor : textColor
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight

        Behavior on color {
            ColorAnimation {
                duration: Style.motionDuration * Style.animationSpeed
            }
        }
    }

    // Background
    background: Rectangle {
        implicitWidth: control.implicitWidth
        implicitHeight: control.implicitHeight
        radius: Style.cornerRadius
        color: getBackgroundColor()
        border.color: getBorderColor()
        border.width: Style.borderWidth

        // Glass shine
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.leftMargin: 2
            anchors.rightMargin: 2
            anchors.topMargin: 1
            height: 1
            visible: Style.isGlass
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 0.5
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

        function getBackgroundColor() {
            if (!control.enabled)
                return Style.darkenColor(backgroundColor, 30);
            if (control.pressed)
                return Style.darkenColor(backgroundColor, 10);
            if (control.hovered)
                return Style.lightenColor(backgroundColor, 5);
            return backgroundColor;
        }

        function getBorderColor() {
            if (control.pressed || control.popup.visible)
                return highlightColor;
            if (control.hovered)
                return Style.lightenColor(borderColor, 20);
            return borderColor;
        }
    }

    // Popup
    popup: Popup {
        y: control.height + 4
        width: control.width
        implicitHeight: contentItem.implicitHeight + padding * 2
        padding: 4

        contentItem: ListView {
            clip: true
            implicitHeight: Math.min(contentHeight, 200)
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            color: popupBackground
            radius: Style.cornerRadius
            border.color: borderColor
            border.width: Style.borderWidth

            // Glass gradient
            gradient: Style.isGlass ? popupGradient : null
            property Gradient popupGradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: Qt.alpha(popupBackground, 0.95)
                }
                GradientStop {
                    position: 1.0
                    color: Qt.alpha(popupBackground, 0.98)
                }
            }

            // Shadow
            layer.enabled: Style.isGlass
            layer.effect: MultiEffect {
                shadowEnabled: true
                shadowColor: Style.shadowColor
                shadowBlur: 1.5
                shadowVerticalOffset: 4
                shadowOpacity: 0.4
                autoPaddingEnabled: true
            }
        }

        enter: Transition {
            NumberAnimation {
                property: "opacity"
                from: 0.0
                to: 1.0
                duration: Style.motionDuration * Style.animationSpeed
            }
            NumberAnimation {
                property: "y"
                from: control.height - 10
                to: control.height + 4
                duration: Style.motionDuration * Style.animationSpeed
                easing.type: Easing.OutCubic
            }
        }

        exit: Transition {
            NumberAnimation {
                property: "opacity"
                from: 1.0
                to: 0.0
                duration: Style.motionDuration * Style.animationSpeed * 0.5
            }
        }
    }
}
