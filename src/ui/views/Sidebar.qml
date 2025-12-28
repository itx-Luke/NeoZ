import QtQuick
import QtQuick.Layouts
import NeoZ
import "../components"
import "../style"

Rectangle {
    id: root
    color: "transparent" // Transparent to show background
    width: 200

    // ===========================================
    // SIDEBAR BACKGROUND - THEME AWARE
    // ===========================================
    Rectangle {
        id: sidebarBg
        anchors.fill: parent
        color: "transparent"

        // --- GLASS THEME: Animated Liquid Gradient ---
        Rectangle {
            anchors.fill: parent
            visible: Style.isGlass
            clip: true

            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(0.06, 0.08, 0.15, Style.sidebarOpacity)
                    SequentialAnimation on color {
                        running: Style.isGlass
                        loops: Animation.Infinite
                        ColorAnimation {
                            to: Qt.rgba(0.1, 0.06, 0.18, Style.sidebarOpacity)
                            duration: 5000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: Qt.rgba(0.06, 0.08, 0.15, Style.sidebarOpacity)
                            duration: 5000
                            easing.type: Easing.InOutSine
                        }
                    }
                }
                GradientStop {
                    position: 1.0
                    color: Qt.rgba(0.02, 0.03, 0.08, Style.sidebarOpacity * 0.8)
                }
            }

            // Top shine
            Rectangle {
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.top: parent.top
                height: parent.height * 0.3
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
                opacity: 0.5
                SequentialAnimation on opacity {
                    running: Style.isGlass
                    loops: Animation.Infinite
                    NumberAnimation {
                        to: 0.7
                        duration: 3000
                        easing.type: Easing.InOutSine
                    }
                    NumberAnimation {
                        to: 0.5
                        duration: 3000
                        easing.type: Easing.InOutSine
                    }
                }
            }
        }

        // --- DARK MINIMAL THEME: Static Matte ---
        Rectangle {
            anchors.fill: parent
            visible: !Style.isGlass
            color: Qt.alpha(Style.surface, Style.sidebarOpacity)
        }

        // Right border
        Rectangle {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 1
            color: Style.isGlass ? Qt.rgba(1, 1, 1, 0.15) : Style.border
        }
    }

    property int currentIndex: 0

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 8

        // --- LOGO / TITLE ---
        RowLayout {
            Layout.fillWidth: true
            Layout.bottomMargin: 20
            spacing: 10

            // Logo placeholder (stylized NZ)
            Rectangle {
                width: 40
                height: 40
                radius: 8
                color: "#3300E5FF"
                border.color: Style.primary
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    text: "NZ"
                    color: Style.primary
                    font.pixelSize: 16
                    font.bold: true
                    font.family: "Consolas"
                }
            }

            Column {
                Text {
                    text: "Neo-Z"
                    color: Style.textPrimary
                    font.pixelSize: 18
                    font.bold: true
                }
                Text {
                    text: "Sensitivity Suite"
                    color: Style.textSecondary
                    font.pixelSize: 10
                }
            }
        }

        // --- NAVIGATION ITEMS ---
        IconLabel {
            iconSource: "qrc:/qt/qml/NeoZ/assets/dashboard.svg"
            labelText: "Dashboard"
            isActive: root.currentIndex === 0
            Layout.fillWidth: true
            onClicked: root.currentIndex = 0
        }

        IconLabel {
            iconSource: "qrc:/qt/qml/NeoZ/assets/sensitivity.svg"
            labelText: "Sensitivity"
            isActive: root.currentIndex === 1
            Layout.fillWidth: true
            onClicked: root.currentIndex = 1
        }

        IconLabel {
            iconSource: "qrc:/qt/qml/NeoZ/assets/history.svg"
            labelText: "History / Rollback"
            isActive: root.currentIndex === 2
            Layout.fillWidth: true
            onClicked: root.currentIndex = 2
        }

        IconLabel {
            iconSource: "qrc:/qt/qml/NeoZ/assets/script.svg"
            labelText: "Script Runner"
            isActive: root.currentIndex === 3
            Layout.fillWidth: true
            onClicked: root.currentIndex = 3
        }

        IconLabel {
            iconSource: "qrc:/qt/qml/NeoZ/assets/ai.svg"
            labelText: "Advanced / AI"
            isActive: root.currentIndex === 4
            Layout.fillWidth: true
            onClicked: root.currentIndex = 4
        }

        IconLabel {
            iconSource: "qrc:/qt/qml/NeoZ/assets/settings.svg"
            labelText: "Settings"
            isActive: root.currentIndex === 5
            Layout.fillWidth: true
            onClicked: root.currentIndex = 5
        }

        // --- SPACER ---
        Item {
            Layout.fillHeight: true
        }

        // --- BOTTOM: LOCAL PROFILE ---
        Rectangle {
            Layout.fillWidth: true
            height: 50
            color: Qt.rgba(1, 1, 1, 0.05)
            radius: 8

            RowLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Rectangle {
                    width: 30
                    height: 30
                    radius: 15
                    color: Style.primary

                    Text {
                        anchors.centerIn: parent
                        text: "LP"
                        color: Style.background
                        font.pixelSize: 11
                        font.bold: true
                    }
                }

                Column {
                    Layout.fillWidth: true
                    Text {
                        text: "Local Profile"
                        color: Style.textPrimary
                        font.pixelSize: 12
                        font.weight: Font.Medium
                    }
                    Text {
                        text: "Online"
                        color: Style.success
                        font.pixelSize: 10
                    }
                }
            }
        }
    }
}
