import QtQuick
import QtQuick.Layouts
import NeoZ
import "."

// NeoZCard - Themed card container with optional title
NeoZPanel {
    id: card

    property string title: ""
    property alias cardContent: cardContentContainer.data

    // Override default content to add title support
    content: ColumnLayout {
        anchors.fill: parent
        spacing: 12

        // Title row
        Text {
            id: titleText
            Layout.fillWidth: true
            text: card.title
            font: Style.headerFont
            font.pixelSize: 16
            font.bold: true
            color: Style.textPrimary
            visible: card.title !== ""

            Behavior on color {
                ColorAnimation {
                    duration: Style.motionDuration * Style.animationSpeed
                }
            }
        }

        // Divider
        Rectangle {
            Layout.fillWidth: true
            height: 1
            color: Style.border
            visible: card.title !== ""
            opacity: 0.5
        }

        // Card content
        Item {
            id: cardContentContainer
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
