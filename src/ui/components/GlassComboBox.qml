import QtQuick
import QtQuick.Controls
import NeoZ

ComboBox {
    id: control

    delegate: ItemDelegate {
        id: delegateItem
        width: control.width
        contentItem: Text {
            text: modelData
            color: highlighted ? Style.primary : Style.textSecondary
            font: control.font
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
        }
        background: Rectangle {
            color: highlighted ? Qt.alpha(Style.primary, 0.1) : "transparent"
        }
        highlighted: control.highlightedIndex === index
    }

    indicator: Canvas {
        id: canvas
        x: control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        width: 12
        height: 8
        contextType: "2d"

        Connections {
            target: control
            function onPressedChanged() {
                canvas.requestPaint();
            }
        }

        onPaint: {
            context.reset();
            context.moveTo(0, 0);
            context.lineTo(width, 0);
            context.lineTo(width / 2, height);
            context.closePath();
            context.fillStyle = control.pressed ? Style.primary : Style.textSecondary;
            context.fill();
        }
    }

    contentItem: Text {
        leftPadding: 10
        rightPadding: control.indicator.width + control.spacing

        text: control.displayText
        font: control.font
        color: control.pressed ? Style.primary : Style.textPrimary
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: Rectangle {
        implicitWidth: 120
        implicitHeight: 40
        radius: 6

        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0.0
                color: Qt.rgba(1, 1, 1, 0.1)
            }
            GradientStop {
                position: 1.0
                color: Qt.rgba(1, 1, 1, 0.05)
            }
        }

        border.color: control.pressed ? Style.primary : Qt.rgba(1, 1, 1, 0.1)
        border.width: 1
    }

    popup: Popup {
        y: control.height - 1
        width: control.width
        implicitHeight: contentItem.implicitHeight
        padding: 1

        contentItem: ListView {
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex

            ScrollIndicator.vertical: ScrollIndicator {}
        }

        background: Rectangle {
            color: Style.surface
            border.color: Style.surfaceHighlight
            radius: 6

            // Glass effect for popup
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: Qt.alpha(Style.surface, 0.95)
                }
                GradientStop {
                    position: 1.0
                    color: Qt.alpha(Style.surface, 0.98)
                }
            }
        }
    }
}
