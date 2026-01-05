import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import NeoZOptimizer 1.0

/**
 * ZerecaUIPanel - Zereca Control Panel with Popup Dialogs
 *
 * Structure:
 * ├── Header (Title + Status) - toggles with Expert Mode Header
 * ├── Button Row (Recommendations + Expert Mode buttons)
 * ├── OptimizationStatusGrid (4 tiles)
 * ├── Footer (Last outcome + Apply All/Revert)
 * ├── Expert Mode Content (shows when Expert Mode active)
 * └── Popups (Recommendations + Advanced Settings)
 */
Item {
    id: zerecaPanel

    // Colors - Dark Minimal Theme
    readonly property color accentCyan: "#00D4FF"
    readonly property color accentPurple: "#9D00FF"
    readonly property color accentGreen: "#00FF41"
    readonly property color accentOrange: "#FF8C00"
    readonly property color accentRed: "#FF4444"
    readonly property color bgDark: Qt.rgba(0.02, 0.02, 0.06, 0.95)

    // Expert Mode page state - controls full panel transition
    property bool showExpertMode: false

    ColumnLayout {
        anchors.fill: parent
        spacing: 12

        // ========== HEADER (Normal Mode) ==========
        Rectangle {
            visible: !showExpertMode
            opacity: showExpertMode ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
            Layout.fillWidth: true
            height: 48
            radius: 12
            color: bgDark
            border.width: 1
            border.color: Qt.rgba(0, 0.8, 1, 0.3)

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 16
                anchors.rightMargin: 16

                Text {
                    text: "OPTIMIZATION CENTER"
                    color: accentCyan
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                }

                Item {
                    Layout.fillWidth: true
                }

                // Status indicator
                Rectangle {
                    width: statusText.width + 20
                    height: 24
                    radius: 12
                    color: Bridge && Bridge.reconciling ? Qt.rgba(0, 0.8, 1, 0.2) : Qt.rgba(0.4, 0.4, 0.4, 0.2)
                    border.width: 1
                    border.color: Bridge && Bridge.reconciling ? accentCyan : Qt.rgba(0.6, 0.6, 0.6, 0.5)

                    Text {
                        id: statusText
                        anchors.centerIn: parent
                        text: Bridge ? Bridge.status : "Idle"
                        color: Bridge && Bridge.reconciling ? accentCyan : "#888888"
                        font.pixelSize: 10
                        font.weight: Font.Bold
                    }

                    SequentialAnimation on opacity {
                        loops: Animation.Infinite
                        running: Bridge && Bridge.reconciling
                        NumberAnimation {
                            to: 0.6
                            duration: 500
                        }
                        NumberAnimation {
                            to: 1.0
                            duration: 500
                        }
                    }
                }
            }
        }

        // ========== EXPERT MODE HEADER ==========
        Rectangle {
            visible: showExpertMode
            opacity: showExpertMode ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
            Layout.fillWidth: true
            height: 48
            radius: 12
            color: Qt.rgba(0.03, 0.03, 0.08, 0.98)
            border.width: 1
            border.color: accentPurple

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 16

                // Back Button
                Rectangle {
                    width: 80
                    height: 32
                    radius: 8
                    color: backBtnHover.containsMouse ? Qt.rgba(1, 1, 1, 0.12) : Qt.rgba(1, 1, 1, 0.06)
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.15)

                    Text {
                        anchors.centerIn: parent
                        text: "< BACK"
                        color: backBtnHover.containsMouse ? "#FFFFFF" : "#AAAAAA"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                    }

                    MouseArea {
                        id: backBtnHover
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        hoverEnabled: true
                        onClicked: showExpertMode = false
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                Text {
                    text: "EXPERT MODE"
                    color: accentPurple
                    font.pixelSize: 14
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                }

                Item {
                    Layout.fillWidth: true
                }

                // Status badge
                Rectangle {
                    width: 80
                    height: 24
                    radius: 12
                    color: Qt.rgba(0.6, 0, 1, 0.15)
                    border.width: 1
                    border.color: accentPurple

                    Text {
                        anchors.centerIn: parent
                        text: "ADVANCED"
                        color: accentPurple
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }
                }
            }
        }

        // ========== BUTTON ROW (Normal Mode) ==========
        RowLayout {
            visible: !showExpertMode
            opacity: showExpertMode ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
            Layout.fillWidth: true
            spacing: 10

            // Recommendations Button - Dark Minimal
            Rectangle {
                Layout.fillWidth: true
                height: 40
                radius: 8
                color: recBtnHover.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(1, 1, 1, 0.03)
                border.width: 1
                border.color: recBtnHover.containsMouse ? Qt.rgba(1, 1, 1, 0.2) : Qt.rgba(1, 1, 1, 0.08)

                Behavior on color {
                    ColorAnimation {
                        duration: 150
                    }
                }
                Behavior on border.color {
                    ColorAnimation {
                        duration: 150
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14
                    spacing: 10

                    Text {
                        text: "RECOMMENDATIONS"
                        color: recBtnHover.containsMouse ? "#FFFFFF" : "#AAAAAA"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        font.letterSpacing: 1
                    }

                    // Badge count
                    Rectangle {
                        visible: Bridge && Bridge.recommendations && Bridge.recommendations.length > 0
                        width: 18
                        height: 18
                        radius: 9
                        color: Qt.rgba(1, 1, 1, 0.15)
                        border.width: 1
                        border.color: Qt.rgba(1, 1, 1, 0.25)

                        Text {
                            anchors.centerIn: parent
                            text: Bridge && Bridge.recommendations ? Bridge.recommendations.length : 0
                            color: "#FFFFFF"
                            font.pixelSize: 9
                            font.weight: Font.Bold
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }

                MouseArea {
                    id: recBtnHover
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: recommendationsPopup.open()
                }
            }

            // Expert Mode Button - Dark Minimal
            Rectangle {
                Layout.fillWidth: true
                height: 40
                radius: 8
                color: advBtnHover.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(1, 1, 1, 0.03)
                border.width: 1
                border.color: advBtnHover.containsMouse ? Qt.rgba(1, 1, 1, 0.2) : Qt.rgba(1, 1, 1, 0.08)

                Behavior on color {
                    ColorAnimation {
                        duration: 150
                    }
                }
                Behavior on border.color {
                    ColorAnimation {
                        duration: 150
                    }
                }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 14
                    anchors.rightMargin: 14

                    Text {
                        text: "EXPERT MODE"
                        color: advBtnHover.containsMouse ? "#FFFFFF" : "#AAAAAA"
                        font.pixelSize: 11
                        font.weight: Font.Medium
                        font.letterSpacing: 1
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }

                MouseArea {
                    id: advBtnHover
                    anchors.fill: parent
                    hoverEnabled: true
                    cursorShape: Qt.PointingHandCursor
                    onClicked: zerecaPanel.showExpertMode = true
                }
            }
        }

        // ========== OPTIMIZATION STATUS GRID (Normal Mode - 2x2) ==========
        GridLayout {
            visible: !showExpertMode
            opacity: showExpertMode ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            columns: 2
            rowSpacing: 10
            columnSpacing: 10

            // CPU Tile
            OptimizationTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "CPU"
                icon: ""
                status: Bridge ? Bridge.cpuStatus : "Neutral"
                metric: Bridge ? Bridge.cpuUsage.toFixed(0) + "%" : "--"
                accentColor: accentCyan
                onApply: if (Bridge)
                    Bridge.applyOptimization("CPU")
                onRevert: if (Bridge)
                    Bridge.revertOptimization("CPU")
            }

            // GPU Tile
            OptimizationTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "GPU"
                icon: ""
                status: Bridge ? Bridge.gpuStatus : "Neutral"
                metric: Bridge ? Bridge.gpuUsage.toFixed(0) + "%" : "--"
                accentColor: accentPurple
                onApply: if (Bridge)
                    Bridge.applyOptimization("GPU")
                onRevert: if (Bridge)
                    Bridge.revertOptimization("GPU")
            }

            // RAM Tile
            OptimizationTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "RAM"
                icon: ""
                status: Bridge ? Bridge.ramStatus : "Neutral"
                metric: Bridge ? Bridge.ramUsage.toFixed(0) + "%" : "--"
                accentColor: accentGreen
                onApply: if (Bridge)
                    Bridge.applyOptimization("RAM")
                onRevert: if (Bridge)
                    Bridge.revertOptimization("RAM")
            }

            // Power Tile
            OptimizationTile {
                Layout.fillWidth: true
                Layout.fillHeight: true
                title: "POWER"
                icon: ""
                status: Bridge ? Bridge.powerStatus : "Neutral"
                metric: "Plan"
                accentColor: accentOrange
                onApply: if (Bridge)
                    Bridge.applyOptimization("Power")
                onRevert: if (Bridge)
                    Bridge.revertOptimization("Power")
            }
        }

        // ========== FOOTER (Normal Mode) ==========
        Rectangle {
            visible: !showExpertMode
            opacity: showExpertMode ? 0 : 1
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
            Layout.fillWidth: true
            height: 36
            radius: 8
            color: Qt.rgba(0, 0, 0, 0.3)
            border.width: 1
            border.color: Qt.rgba(1, 1, 1, 0.1)

            RowLayout {
                anchors.fill: parent
                anchors.leftMargin: 12
                anchors.rightMargin: 12

                Text {
                    text: "Last: "
                    color: "#666666"
                    font.pixelSize: 10
                }

                Text {
                    Layout.fillWidth: true
                    text: Bridge && Bridge.lastOutcome ? Bridge.lastOutcome : "No actions taken"
                    color: "#AAAAAA"
                    font.pixelSize: 10
                    elide: Text.ElideRight
                }

                // Apply All button
                Rectangle {
                    width: 70
                    height: 24
                    radius: 6
                    gradient: Gradient {
                        orientation: Gradient.Horizontal
                        GradientStop {
                            position: 0.0
                            color: accentCyan
                        }
                        GradientStop {
                            position: 1.0
                            color: accentPurple
                        }
                    }

                    Text {
                        anchors.centerIn: parent
                        text: "APPLY ALL"
                        color: "#FFFFFF"
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (Bridge)
                            Bridge.applyAll()
                    }
                }

                // Revert button
                Rectangle {
                    width: 60
                    height: 24
                    radius: 6
                    color: Qt.rgba(1, 0.27, 0.27, 0.2)
                    border.width: 1
                    border.color: accentRed

                    Text {
                        anchors.centerIn: parent
                        text: "REVERT"
                        color: accentRed
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: if (Bridge)
                            Bridge.revertAll()
                    }
                }
            }
        }

        // ========== EXPERT MODE CONTENT (Full Panel) ==========
        Rectangle {
            visible: showExpertMode
            opacity: showExpertMode ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 300
                }
            }
            Layout.fillWidth: true
            Layout.fillHeight: true
            radius: 12
            color: Qt.rgba(0.02, 0.02, 0.05, 0.95)
            border.width: 1
            border.color: Qt.rgba(0.6, 0, 1, 0.2)

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 12

                Text {
                    text: "Fine-tune system optimization parameters. Changes take effect immediately."
                    color: "#888888"
                    font.pixelSize: 11
                }

                // Tab buttons row
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Repeater {
                        model: ["CPU", "GPU", "Memory", "Kernel"]
                        Rectangle {
                            width: 80
                            height: 36
                            radius: 8
                            color: advancedSettingsPopup.activeTab === index ? Qt.rgba(0.6, 0, 1, 0.25) : Qt.rgba(1, 1, 1, 0.05)
                            border.width: 1
                            border.color: advancedSettingsPopup.activeTab === index ? accentPurple : Qt.rgba(1, 1, 1, 0.1)

                            Text {
                                anchors.centerIn: parent
                                text: modelData
                                color: advancedSettingsPopup.activeTab === index ? accentPurple : "#888888"
                                font.pixelSize: 11
                                font.weight: Font.Bold
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: advancedSettingsPopup.activeTab = index
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                    }
                }

                // Content area placeholder - uses popup's StackLayout
                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 8
                    color: Qt.rgba(0, 0, 0, 0.3)
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.08)

                    Text {
                        anchors.centerIn: parent
                        text: "Expert controls for " + ["CPU", "GPU", "Memory", "Kernel"][advancedSettingsPopup.activeTab] + " optimization\n\nClick tabs above to switch between optimization categories."
                        color: "#666666"
                        font.pixelSize: 12
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }

    // ==================== RECOMMENDATIONS POPUP ====================
    Popup {
        id: recommendationsPopup
        anchors.centerIn: parent
        width: 400
        height: 350
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: Qt.rgba(0.05, 0.05, 0.1, 0.98)
            radius: 16
            border.width: 2
            border.color: accentOrange
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 12

            // Header
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "RECOMMENDATIONS"
                    color: accentOrange
                    font.pixelSize: 16
                    font.weight: Font.Bold
                    font.letterSpacing: 2
                }

                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: 28
                    height: 28
                    radius: 14
                    color: Qt.rgba(1, 1, 1, 0.1)

                    Text {
                        anchors.centerIn: parent
                        text: "X"
                        color: "#FFFFFF"
                        font.pixelSize: 14
                    }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: recommendationsPopup.close()
                    }
                }
            }

            // Recommendations list
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                radius: 10
                color: Qt.rgba(0, 0, 0, 0.3)

                ListView {
                    anchors.fill: parent
                    anchors.margins: 10
                    model: Bridge ? Bridge.recommendations : []
                    spacing: 8
                    clip: true

                    delegate: Rectangle {
                        width: ListView.view.width
                        height: 36
                        radius: 8
                        color: Qt.rgba(1, 0.55, 0, 0.15)
                        border.width: 1
                        border.color: Qt.rgba(1, 0.55, 0, 0.3)

                        Text {
                            anchors.left: parent.left
                            anchors.leftMargin: 12
                            anchors.verticalCenter: parent.verticalCenter
                            text: modelData
                            color: "#DDDDDD"
                            font.pixelSize: 11
                            elide: Text.ElideRight
                            width: parent.width - 24
                        }
                    }
                }

                // Empty state
                Text {
                    anchors.centerIn: parent
                    visible: !Bridge || !Bridge.recommendations || Bridge.recommendations.length === 0
                    text: "No recommendations at this time"
                    color: "#666666"
                    font.pixelSize: 12
                }
            }

            // Clear button
            Rectangle {
                Layout.alignment: Qt.AlignRight
                width: 80
                height: 32
                radius: 8
                color: Qt.rgba(1, 1, 1, 0.1)
                border.width: 1
                border.color: Qt.rgba(1, 1, 1, 0.2)

                Text {
                    anchors.centerIn: parent
                    text: "Clear All"
                    color: "#AAAAAA"
                    font.pixelSize: 10
                }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        if (Bridge)
                            Bridge.clearRecommendations();
                        recommendationsPopup.close();
                    }
                }
            }
        }
    }

    // ==================== EXPERT COMMAND DECK ====================
    Popup {
        id: advancedSettingsPopup
        anchors.centerIn: parent
        width: 900
        height: 620
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        // Override lock states
        property bool cpuLocked: false
        property bool gpuLocked: false
        property bool memoryLocked: false
        property bool kernelLocked: false
        property int activeTab: 0  // 0=CPU, 1=GPU, 2=Memory, 3=Kernel

        background: Rectangle {
            color: Qt.rgba(0.03, 0.03, 0.08, 0.98)
            radius: 20
            border.width: 2
            border.color: accentPurple

            // Animated border glow
            Rectangle {
                anchors.fill: parent
                anchors.margins: -2
                radius: 22
                color: "transparent"
                border.width: 1
                border.color: Qt.rgba(0.6, 0, 1, 0.3)
                z: -1
            }
        }

        contentItem: ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 16

            // ========== HEADER ==========
            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: "ZERECA EXPERT MODE"
                    color: accentPurple
                    font.pixelSize: 18
                    font.weight: Font.Bold
                    font.letterSpacing: 3
                }

                Rectangle {
                    width: 60
                    height: 20
                    radius: 10
                    color: Qt.rgba(1, 0.27, 0.27, 0.3)
                    border.width: 1
                    border.color: accentRed

                    Text {
                        anchors.centerIn: parent
                        text: "EXPERT"
                        color: accentRed
                        font.pixelSize: 8
                        font.weight: Font.Bold
                    }
                }

                Item {
                    Layout.fillWidth: true
                }

                // Kill-switch hint
                Text {
                    text: "Ctrl+Shift+F12 = Reset All"
                    color: "#555555"
                    font.pixelSize: 9
                }

                Item {
                    width: 16
                }

                Rectangle {
                    width: 32
                    height: 32
                    radius: 16
                    color: closeHover.containsMouse ? Qt.rgba(1, 0.2, 0.2, 0.4) : Qt.rgba(1, 1, 1, 0.1)

                    Text {
                        anchors.centerIn: parent
                        text: "X"
                        color: closeHover.containsMouse ? accentRed : "#FFFFFF"
                        font.pixelSize: 16
                    }

                    MouseArea {
                        id: closeHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: advancedSettingsPopup.close()
                    }
                }
            }

            // ========== TAB BAR ==========
            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                Repeater {
                    model: [
                        {
                            name: "CPU",
                            icon: "",
                            color: accentCyan,
                            desc: "Reactor"
                        },
                        {
                            name: "GPU",
                            icon: "",
                            color: accentPurple,
                            desc: "Afterburner"
                        },
                        {
                            name: "MEMORY",
                            icon: "",
                            color: accentGreen,
                            desc: "Airlock"
                        },
                        {
                            name: "KERNEL",
                            icon: "",
                            color: accentOrange,
                            desc: "Deep Dive"
                        }
                    ]

                    Rectangle {
                        Layout.fillWidth: true
                        height: 50
                        radius: 12
                        color: advancedSettingsPopup.activeTab === index ? Qt.rgba(modelData.color.r, modelData.color.g, modelData.color.b, 0.25) : tabHover.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(1, 1, 1, 0.03)
                        border.width: advancedSettingsPopup.activeTab === index ? 2 : 1
                        border.color: advancedSettingsPopup.activeTab === index ? modelData.color : Qt.rgba(1, 1, 1, 0.1)

                        Behavior on color {
                            ColorAnimation {
                                duration: 150
                            }
                        }
                        Behavior on border.color {
                            ColorAnimation {
                                duration: 150
                            }
                        }

                        ColumnLayout {
                            anchors.centerIn: parent
                            spacing: 2

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: modelData.name
                                color: advancedSettingsPopup.activeTab === index ? modelData.color : "#888888"
                                font.pixelSize: 12
                                font.weight: Font.Bold
                            }

                            Text {
                                Layout.alignment: Qt.AlignHCenter
                                text: modelData.desc
                                color: "#555555"
                                font.pixelSize: 9
                                font.italic: true
                            }
                        }

                        MouseArea {
                            id: tabHover
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: advancedSettingsPopup.activeTab = index
                        }
                    }
                }
            }

            // ========== CONTENT STACK ==========
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: advancedSettingsPopup.activeTab

                // ===== TAB 0: CPU CONTROL =====
                Rectangle {
                    color: Qt.rgba(0, 0, 0, 0.2)
                    radius: 16
                    border.width: 1
                    border.color: Qt.rgba(0, 0.8, 1, 0.2)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        // Lock indicator
                        LockHeader {
                            title: "CPU CONTROL"
                            locked: advancedSettingsPopup.cpuLocked
                            accentColor: accentCyan
                            onToggleLock: advancedSettingsPopup.cpuLocked = !advancedSettingsPopup.cpuLocked
                        }

                        // Working CPU Boost Toggle (connected to backend)
                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            radius: 8
                            color: Qt.rgba(0, 0.8, 1, 0.1)
                            border.width: 1
                            border.color: Qt.rgba(0, 0.8, 1, 0.3)

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12

                                Text {
                                    text: "CPU BOOST"
                                    color: accentCyan
                                    font.pixelSize: 11
                                    font.weight: Font.Bold
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                ToggleSwitch {
                                    label: Bridge && Bridge.cpuBoostEnabled ? "ENABLED" : "DISABLED"
                                    checked: Bridge ? Bridge.cpuBoostEnabled : true
                                    onToggled: if (Bridge)
                                        Bridge.cpuBoostEnabled = checked
                                }
                            }
                        }

                        // Core Affinity Grid placeholder
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.3)

                            Text {
                                anchors.centerIn: parent
                                text: "Core Affinity Matrix\n(16 cores)"
                                color: "#666666"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }

                // ===== TAB 1: GPU CONTROL =====
                Rectangle {
                    color: Qt.rgba(0, 0, 0, 0.2)
                    radius: 16
                    border.width: 1
                    border.color: Qt.rgba(0.6, 0, 1, 0.2)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        LockHeader {
                            title: "GPU CONTROL"
                            locked: advancedSettingsPopup.gpuLocked
                            accentColor: accentPurple
                            onToggleLock: advancedSettingsPopup.gpuLocked = !advancedSettingsPopup.gpuLocked
                        }

                        // Working GPU Boost Toggle (connected to backend)
                        Rectangle {
                            Layout.fillWidth: true
                            height: 40
                            radius: 8
                            color: Qt.rgba(0.6, 0, 1, 0.1)
                            border.width: 1
                            border.color: Qt.rgba(0.6, 0, 1, 0.3)

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 12
                                anchors.rightMargin: 12

                                Text {
                                    text: "GPU BOOST"
                                    color: accentPurple
                                    font.pixelSize: 11
                                    font.weight: Font.Bold
                                }

                                Item {
                                    Layout.fillWidth: true
                                }

                                ToggleSwitch {
                                    label: Bridge && Bridge.gpuBoostEnabled ? "ENABLED" : "DISABLED"
                                    checked: Bridge ? Bridge.gpuBoostEnabled : true
                                    onToggled: if (Bridge)
                                        Bridge.gpuBoostEnabled = checked
                                }
                            }
                        }

                        // GPU settings placeholder
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.3)

                            Text {
                                anchors.centerIn: parent
                                text: "GPU Power Management\nLow Latency / Reflex"
                                color: "#666666"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }

                // ===== TAB 2: MEMORY CONTROL =====
                Rectangle {
                    color: Qt.rgba(0, 0, 0, 0.2)
                    radius: 16
                    border.width: 1
                    border.color: Qt.rgba(0, 1, 0.25, 0.2)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        LockHeader {
                            title: "MEMORY CONTROL"
                            locked: advancedSettingsPopup.memoryLocked
                            accentColor: accentGreen
                            onToggleLock: advancedSettingsPopup.memoryLocked = !advancedSettingsPopup.memoryLocked
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.3)

                            Text {
                                anchors.centerIn: parent
                                text: "Memory Optimization\nWorking Set Trim / Standby List"
                                color: "#666666"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }

                // ===== TAB 3: KERNEL CONTROL =====
                Rectangle {
                    color: Qt.rgba(0, 0, 0, 0.2)
                    radius: 16
                    border.width: 1
                    border.color: Qt.rgba(1, 0.55, 0, 0.2)

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 16
                        spacing: 16

                        LockHeader {
                            title: "KERNEL CONTROL"
                            locked: advancedSettingsPopup.kernelLocked
                            accentColor: accentOrange
                            onToggleLock: advancedSettingsPopup.kernelLocked = !advancedSettingsPopup.kernelLocked
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.fillHeight: true
                            radius: 10
                            color: Qt.rgba(0, 0, 0, 0.3)

                            Text {
                                anchors.centerIn: parent
                                text: "Kernel Parameters\nTimer Resolution / HPET"
                                color: "#666666"
                                font.pixelSize: 12
                                horizontalAlignment: Text.AlignHCenter
                            }
                        }
                    }
                }
            }
        }
    }

    // ==================== CUSTOM COMPONENTS ====================

    component LockHeader: RowLayout {
        property string title
        property bool locked
        property color accentColor
        signal toggleLock

        Layout.fillWidth: true

        Text {
            text: title
            color: accentColor
            font.pixelSize: 14
            font.weight: Font.Bold
            font.letterSpacing: 2
        }

        Item {
            Layout.fillWidth: true
        }

        Text {
            text: locked ? "Manual Override Active" : "Zereca Auto-Managed"
            color: locked ? accentRed : "#666666"
            font.pixelSize: 10
        }

        Rectangle {
            width: 32
            height: 32
            radius: 8
            color: locked ? Qt.rgba(1, 0.27, 0.27, 0.3) : Qt.rgba(1, 1, 1, 0.1)
            border.width: 1
            border.color: locked ? accentRed : Qt.rgba(1, 1, 1, 0.2)

            Text {
                anchors.centerIn: parent
                text: locked ? "L" : "U"
                color: locked ? accentRed : "#666666"
                font.pixelSize: 12
                font.weight: Font.Bold
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: toggleLock()
            }
        }
    }

    component OptimizationTile: Rectangle {
        id: tile
        property string title
        property string icon
        property string status  // "Applied", "Reverted", "Neutral"
        property string metric
        property color accentColor

        signal apply
        signal revert

        radius: 12
        color: Qt.rgba(0, 0, 0, 0.4)
        border.width: status === "Applied" ? 2 : 1
        border.color: status === "Applied" ? accentColor : Qt.rgba(1, 1, 1, 0.1)

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 12
            spacing: 6

            RowLayout {
                Layout.fillWidth: true

                Text {
                    text: title
                    color: accentColor
                    font.pixelSize: 12
                    font.weight: Font.Bold
                    font.letterSpacing: 1
                }

                Item {
                    Layout.fillWidth: true
                }

                Rectangle {
                    width: statusLabel.width + 12
                    height: 18
                    radius: 9
                    color: status === "Applied" ? Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.25) : Qt.rgba(1, 1, 1, 0.1)
                    border.width: 1
                    border.color: status === "Applied" ? accentColor : Qt.rgba(1, 1, 1, 0.2)

                    Text {
                        id: statusLabel
                        anchors.centerIn: parent
                        text: status
                        color: status === "Applied" ? accentColor : "#888888"
                        font.pixelSize: 8
                        font.weight: Font.Bold
                    }
                }
            }

            Text {
                text: metric
                color: "#FFFFFF"
                font.pixelSize: 24
                font.weight: Font.Bold
            }

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 6

                Rectangle {
                    Layout.fillWidth: true
                    height: 26
                    radius: 6
                    color: tileApplyHover.containsMouse ? Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.3) : Qt.rgba(accentColor.r, accentColor.g, accentColor.b, 0.15)
                    border.width: 1
                    border.color: accentColor

                    Text {
                        anchors.centerIn: parent
                        text: "APPLY"
                        color: accentColor
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }

                    MouseArea {
                        id: tileApplyHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: tile.apply()
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 26
                    radius: 6
                    color: tileRevertHover.containsMouse ? Qt.rgba(1, 1, 1, 0.1) : "transparent"
                    border.width: 1
                    border.color: Qt.rgba(1, 1, 1, 0.2)

                    Text {
                        anchors.centerIn: parent
                        text: "REVERT"
                        color: "#888888"
                        font.pixelSize: 9
                        font.weight: Font.Bold
                    }

                    MouseArea {
                        id: tileRevertHover
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: tile.revert()
                    }
                }
            }
        }
    }

    component ToggleSwitch: RowLayout {
        property string label
        property bool checked

        signal toggled(bool checked)

        spacing: 10

        Rectangle {
            width: 40
            height: 20
            radius: 10
            color: checked ? accentCyan : Qt.rgba(1, 1, 1, 0.2)

            Rectangle {
                x: checked ? 22 : 2
                y: 2
                width: 16
                height: 16
                radius: 8
                color: "#FFFFFF"

                Behavior on x {
                    NumberAnimation {
                        duration: 150
                    }
                }
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    checked = !checked;
                    toggled(checked);
                }
            }
        }

        Text {
            text: label
            color: checked ? "#FFFFFF" : "#888888"
            font.pixelSize: 11
        }
    }
}
