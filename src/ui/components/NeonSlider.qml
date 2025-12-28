import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NeoZ

ColumnLayout {
    id: root
    spacing: 5

    property alias value: control.value
    property alias from: control.from
    property alias to: control.to
    property alias stepSize: control.stepSize
    property string label: "Label"
    property string suffix: ""
    property color accentColor: Style.primary

    RowLayout {
        Layout.fillWidth: true
        Text {
            text: root.label
            color: Style.textSecondary
            font.pixelSize: 12
            Layout.fillWidth: true
        }
        Text {
            text: control.value.toFixed(2) + root.suffix
            color: root.accentColor
            font.bold: true
            font.pixelSize: 12
        }
    }

    Slider {
        id: control
        Layout.fillWidth: true

        background: Rectangle {
            x: control.leftPadding
            y: control.topPadding + control.availableHeight / 2 - height / 2
            implicitWidth: 200
            implicitHeight: 4
            width: control.availableWidth
            height: implicitHeight
            radius: 2
            color: Qt.rgba(1, 1, 1, 0.1)

            Rectangle {
                width: control.visualPosition * parent.width
                height: parent.height
                color: root.accentColor
                radius: 2
            }
        }

        handle: Rectangle {
            x: control.leftPadding + control.visualPosition * (control.availableWidth - width)
            y: control.topPadding + control.availableHeight / 2 - height / 2
            implicitWidth: 16
            implicitHeight: 16
            radius: 8
            color: root.accentColor
            border.color: "white"
            border.width: 1
        }
    }
}
