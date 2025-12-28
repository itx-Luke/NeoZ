import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import NeoZ
import "../components"
import "../style"

Item {
    id: root

    // Property to track dropdown expanded state at root level
    property bool themeDropdownExpanded: false

    ScrollView {
        anchors.fill: parent
        anchors.margins: 30
        clip: true
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 25

            // === HEADER ===
            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                Text {
                    text: "⚙️"
                    font.pixelSize: 24
                }
                Text {
                    text: "Settings"
                    color: Style.textPrimary
                    font.pixelSize: 24
                    font.bold: true
                }
            }

            // ============================================
            // SECTION: DEVICES & ADB
            // ============================================
            Text {
                text: "Devices & ADB"
                color: Style.primary
                font.pixelSize: 16
                font.bold: true
            }

            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: contentColDevices.implicitHeight + 60
                glassOpacity: 0.25

                ColumnLayout {
                    id: contentColDevices
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    // ADB Path Row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: "ADB Path"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 100
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.maximumWidth: 350
                            height: 36
                            radius: 6
                            color: Qt.rgba(0, 0, 0, 0.4)
                            border.color: Style.surfaceHighlight
                            border.width: 1

                            TextInput {
                                id: adbPathInput
                                anchors.fill: parent
                                anchors.margins: 10
                                text: Backend.adbPath || "C:\\Android\\platform-tools\\adb.exe"
                                color: Style.textPrimary
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                selectByMouse: true
                            }
                        }

                        Rectangle {
                            width: 80
                            height: 36
                            radius: 6
                            color: "#3300E5FF"
                            border.color: Style.primary
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "Test ADB"
                                color: Style.primary
                                font.pixelSize: 12
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: Backend.testAdbConnection()
                            }
                        }

                        // Status indicator
                        Rectangle {
                            width: 140
                            height: 36
                            radius: 6
                            color: Backend.adbStatus === "Connected" ? "#1A00E676" : "#1AFFA726"
                            border.color: Backend.adbStatus === "Connected" ? Style.success : Style.warning
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: Backend.adbStatus || "Not Connected"
                                color: Backend.adbStatus === "Connected" ? Style.success : Style.warning
                                font.pixelSize: 11
                            }
                        }
                    }

                    // Preferred Device Row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: "Preferred Device"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 100
                        }

                        ComboBox {
                            id: deviceCombo
                            Layout.preferredWidth: 280
                            model: Backend.availableDevices || ["Auto-detect emulator"]

                            background: Rectangle {
                                color: Qt.rgba(0, 0, 0, 0.4)
                                border.color: Style.surfaceHighlight
                                border.width: 1
                                radius: 6
                            }

                            contentItem: Text {
                                text: deviceCombo.displayText
                                color: Style.textPrimary
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 10
                            }
                        }
                    }
                }
            }

            // ============================================
            // SECTION: AI INTEGRATION (GEMINI)
            // ============================================
            Text {
                text: "AI Integration (Gemini)"
                color: Style.primary
                font.pixelSize: 16
                font.bold: true
                Layout.topMargin: 10
            }

            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: contentColAI.implicitHeight + 60
                glassOpacity: 0.25

                ColumnLayout {
                    id: contentColAI
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 15

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: "API Key"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 100
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            Layout.maximumWidth: 300
                            height: 36
                            radius: 6
                            color: Qt.rgba(0, 0, 0, 0.4)
                            border.color: Style.surfaceHighlight
                            border.width: 1

                            TextInput {
                                id: apiKeyInput
                                anchors.fill: parent
                                anchors.margins: 10
                                echoMode: TextInput.Password
                                color: Style.textPrimary
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                selectByMouse: true

                                Text {
                                    anchors.fill: parent
                                    text: "Enter Gemini API Key..."
                                    color: Style.textSecondary
                                    font.pixelSize: 12
                                    verticalAlignment: Text.AlignVCenter
                                    visible: !apiKeyInput.text && !apiKeyInput.activeFocus
                                }
                            }
                        }

                        Rectangle {
                            width: 90
                            height: 36
                            radius: 6
                            color: apiKeyInput.text.length > 0 ? "#3300E5FF" : "#0DFFFFFF"
                            border.color: apiKeyInput.text.length > 0 ? Style.primary : Style.surfaceHighlight
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "Save Key"
                                color: apiKeyInput.text.length > 0 ? Style.primary : Style.textSecondary
                                font.pixelSize: 12
                            }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: apiKeyInput.text.length > 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                                enabled: apiKeyInput.text.length > 0
                                onClicked: {
                                    Backend.setGeminiApiKey(apiKeyInput.text);
                                    apiKeyInput.text = "";
                                }
                            }
                        }

                        // AI Status indicator
                        Rectangle {
                            width: 100
                            height: 36
                            radius: 6
                            color: Backend.aiStatus === "Online" ? "#1A00E676" : "#1AFF1744"
                            border.color: Backend.aiStatus === "Online" ? Style.success : Style.danger
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: Backend.aiStatus || "Offline"
                                color: Backend.aiStatus === "Online" ? Style.success : Style.danger
                                font.pixelSize: 11
                            }
                        }
                    }

                    Text {
                        text: "Get your API key from ai.google.dev • Stored locally in ~/.neoz/config.json"
                        color: Style.textSecondary
                        font.pixelSize: 11
                        font.italic: true
                    }
                }
            }

            // ============================================
            // SECTION: GENERAL PREFERENCES
            // ============================================
            Text {
                text: "General"
                color: Style.primary
                font.pixelSize: 16
                font.bold: true
                Layout.topMargin: 10
            }

            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: contentColGeneral.implicitHeight + 60
                glassOpacity: 0.25

                ColumnLayout {
                    id: contentColGeneral
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    // Theme Mode Row
                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        Text {
                            text: "Theme Mode"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 180
                        }

                        Text {
                            text: "Visual theme and background style"
                            color: Qt.alpha(Style.textSecondary, 0.6)
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }

                        // Advanced Glass Dropdown for Theme Mode
                        Item {
                            id: themeModeDropdown
                            width: 200
                            height: 40
                            z: expanded ? 1000 : 1
                            Layout.alignment: Qt.AlignTop
                            clip: false

                            property int currentIndex: ThemeManager.currentTheme
                            property var themes: [
                                {
                                    name: "Dark Minimal",
                                    desc: "Clean matte dark",
                                    color: "#3D3D4A",
                                    themeIndex: 0
                                },
                                {
                                    name: "Gradient Variant",
                                    desc: "Animated liquid glass",
                                    color: "#00E5FF",
                                    themeIndex: 1
                                },
                                {
                                    name: "Frost Glass",
                                    desc: "Premium light glass",
                                    color: "#EC4899",
                                    themeIndex: 2
                                },
                            ]
                            property alias expanded: root.themeDropdownExpanded

                            // Main Button - Glass Style
                            Rectangle {
                                id: dropdownButton
                                width: parent.width
                                height: 40
                                anchors.top: parent.top
                                radius: 10
                                color: Qt.rgba(0, 0, 0, 0.5)
                                border.width: 1
                                border.color: themeModeDropdown.expanded ? Style.primary : Qt.alpha(Style.surfaceHighlight, 0.5)

                                // Gradient shimmer overlay
                                Rectangle {
                                    anchors.fill: parent
                                    radius: parent.radius
                                    opacity: 0.15
                                    gradient: Gradient {
                                        orientation: Gradient.Horizontal
                                        GradientStop {
                                            position: 0.0
                                            color: Style.primary
                                        }
                                        GradientStop {
                                            position: 0.5
                                            color: "#9945FF"
                                        }
                                        GradientStop {
                                            position: 1.0
                                            color: "#00D9FF"
                                        }
                                    }
                                }

                                RowLayout {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 8

                                    // Theme color circle
                                    Rectangle {
                                        width: 24
                                        height: 24
                                        radius: 12
                                        color: themeModeDropdown.currentIndex === 0 ? "transparent" : themeModeDropdown.themes[themeModeDropdown.currentIndex].color
                                        border.width: 2
                                        border.color: Qt.alpha(Style.surfaceHighlight, 0.5)

                                        // Gradient overlay for first option
                                        Rectangle {
                                            anchors.fill: parent
                                            radius: parent.radius
                                            visible: themeModeDropdown.currentIndex === 0
                                            gradient: Gradient {
                                                orientation: Gradient.Horizontal
                                                GradientStop {
                                                    position: 0.0
                                                    color: "#00E5FF"
                                                }
                                                GradientStop {
                                                    position: 0.5
                                                    color: "#9945FF"
                                                }
                                                GradientStop {
                                                    position: 1.0
                                                    color: "#FF6B6B"
                                                }
                                            }
                                        }
                                    }

                                    // Selected theme name
                                    Text {
                                        text: themeModeDropdown.themes[themeModeDropdown.currentIndex].name
                                        color: Style.textPrimary
                                        font.pixelSize: 12
                                        font.weight: Font.Medium
                                        Layout.fillWidth: true
                                    }

                                    // Dropdown arrow
                                    Text {
                                        text: themeModeDropdown.expanded ? "▲" : "▼"
                                        color: Style.primary
                                        font.pixelSize: 10
                                    }
                                }

                                MouseArea {
                                    anchors.fill: parent
                                    cursorShape: Qt.PointingHandCursor
                                    onClicked: themeModeDropdown.expanded = !themeModeDropdown.expanded
                                }

                                // Glow effect when expanded
                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: -2
                                    radius: parent.radius + 2
                                    color: "transparent"
                                    border.width: 2
                                    border.color: Qt.alpha(Style.primary, themeModeDropdown.expanded ? 0.4 : 0)
                                    z: -1

                                    Behavior on border.color {
                                        ColorAnimation {
                                            duration: 200
                                        }
                                    }
                                }
                            }

                            // Dropdown Panel - Black Galaxy Glass UI
                            Rectangle {
                                id: dropdownPanel
                                anchors.top: dropdownButton.bottom
                                anchors.topMargin: 6
                                anchors.left: parent.left
                                anchors.right: parent.right
                                height: themeModeDropdown.expanded ? (156 + 16) : 0
                                radius: 12
                                color: "#12121C"  // More opaque base color
                                border.width: 1
                                border.color: Qt.alpha(Style.primary, 0.5)
                                clip: true
                                visible: height > 0
                                z: 10000  // Higher than globalDropdownBlocker

                                // Panel is interactive - clicks handled by item MouseAreas

                                Behavior on height {
                                    NumberAnimation {
                                        duration: 400
                                        easing.type: Easing.OutCubic
                                    }
                                }

                                Behavior on opacity {
                                    NumberAnimation {
                                        duration: 300
                                        easing.type: Easing.InOutQuad
                                    }
                                }

                                opacity: themeModeDropdown.expanded ? 1.0 : 0.0

                                // Solid background - fully opaque
                                Rectangle {
                                    anchors.fill: parent
                                    radius: parent.radius
                                    color: "#16161F"
                                    opacity: 1.0
                                }

                                // Subtle gradient overlay for accent
                                Rectangle {
                                    anchors.fill: parent
                                    radius: parent.radius
                                    opacity: 0.15  // Reduced from 0.25
                                    gradient: Gradient {
                                        GradientStop {
                                            position: 0.0
                                            color: Style.primary
                                        }
                                        GradientStop {
                                            position: 0.4
                                            color: "#9945FF"
                                        }
                                        GradientStop {
                                            position: 1.0
                                            color: "transparent"
                                        }
                                    }
                                }

                                // Top edge highlight
                                Rectangle {
                                    anchors.top: parent.top
                                    anchors.left: parent.left
                                    anchors.right: parent.right
                                    height: 1
                                    anchors.margins: 1
                                    color: Qt.alpha(Style.surfaceHighlight, 0.3)
                                }

                                // Inner border glow
                                Rectangle {
                                    anchors.fill: parent
                                    anchors.margins: 1
                                    radius: parent.radius - 1
                                    color: "transparent"
                                    border.width: 1
                                    border.color: Qt.rgba(0, 0, 0, 0.4)
                                }

                                Column {
                                    anchors.fill: parent
                                    anchors.margins: 8
                                    spacing: 4

                                    // ===== THEME OPTION 1: Dark Minimal =====
                                    Rectangle {
                                        id: darkMinimalOption
                                        width: parent.width
                                        height: 48
                                        radius: 8
                                        color: ThemeManager.currentTheme === 0 ? Qt.alpha(Style.primary, 0.25) : (darkMinimalMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent")

                                        MouseArea {
                                            id: darkMinimalMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                console.log("[SettingsView] Dark Minimal clicked");
                                                Backend.setTheme(0);  // Persist theme
                                                ThemeManager.setTheme(0);
                                                themeModeDropdown.expanded = false;
                                                root.themeDropdownExpanded = false;
                                            }
                                        }

                                        Row {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 10

                                            // Gradient circle
                                            Rectangle {
                                                width: 28
                                                height: 28
                                                radius: 14
                                                gradient: Gradient {
                                                    orientation: Gradient.Horizontal
                                                    GradientStop {
                                                        position: 0.0
                                                        color: "#00E5FF"
                                                    }
                                                    GradientStop {
                                                        position: 0.5
                                                        color: "#9945FF"
                                                    }
                                                    GradientStop {
                                                        position: 1.0
                                                        color: "#FF6B6B"
                                                    }
                                                }
                                            }

                                            Column {
                                                anchors.verticalCenter: parent.verticalCenter
                                                Text {
                                                    text: "Dark Minimal"
                                                    color: ThemeManager.currentTheme === 0 ? Style.primary : Style.textPrimary
                                                    font.pixelSize: 12
                                                    font.bold: ThemeManager.currentTheme === 0
                                                }
                                                Text {
                                                    text: "Clean matte dark"
                                                    color: Qt.alpha(Style.textSecondary, 0.7)
                                                    font.pixelSize: 10
                                                }
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                                width: 10
                                            }

                                            Text {
                                                text: "✓"
                                                color: Style.primary
                                                font.pixelSize: 14
                                                font.bold: true
                                                visible: ThemeManager.currentTheme === 0
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                        }
                                    }

                                    // ===== THEME OPTION 2: Gradient Variant =====
                                    Rectangle {
                                        id: gradientVariantOption
                                        width: parent.width
                                        height: 48
                                        radius: 8
                                        color: ThemeManager.currentTheme === 1 ? Qt.alpha(Style.primary, 0.25) : (gradientMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent")

                                        MouseArea {
                                            id: gradientMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                console.log("[SettingsView] Gradient Variant clicked");
                                                Backend.setTheme(1);  // Persist theme
                                                ThemeManager.setTheme(1);
                                                themeModeDropdown.expanded = false;
                                                root.themeDropdownExpanded = false;
                                            }
                                        }

                                        Row {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 10

                                            // Cyan circle
                                            Rectangle {
                                                width: 28
                                                height: 28
                                                radius: 14
                                                color: "#00E5FF"
                                                border.width: 2
                                                border.color: Qt.alpha(Style.surfaceHighlight, 0.5)
                                            }

                                            Column {
                                                anchors.verticalCenter: parent.verticalCenter
                                                Text {
                                                    text: "Gradient Variant"
                                                    color: ThemeManager.currentTheme === 1 ? Style.primary : Style.textPrimary
                                                    font.pixelSize: 12
                                                    font.bold: ThemeManager.currentTheme === 1
                                                }
                                                Text {
                                                    text: "Animated liquid glass"
                                                    color: Qt.alpha(Style.textSecondary, 0.7)
                                                    font.pixelSize: 10
                                                }
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                                width: 10
                                            }

                                            Text {
                                                text: "✓"
                                                color: Style.primary
                                                font.pixelSize: 14
                                                font.bold: true
                                                visible: ThemeManager.currentTheme === 1
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                        }
                                    }

                                    // ===== THEME OPTION 3: Frost Glass =====
                                    Rectangle {
                                        id: frostGlassOption
                                        width: parent.width
                                        height: 48
                                        radius: 8
                                        color: ThemeManager.currentTheme === 2 ? Qt.alpha(Style.primary, 0.25) : (frostGlassMouse.containsMouse ? Qt.rgba(1, 1, 1, 0.08) : "transparent")

                                        MouseArea {
                                            id: frostGlassMouse
                                            anchors.fill: parent
                                            hoverEnabled: true
                                            cursorShape: Qt.PointingHandCursor
                                            onClicked: {
                                                console.log("[SettingsView] Frost Glass clicked");
                                                Backend.setTheme(2);
                                                ThemeManager.setTheme(2);
                                                themeModeDropdown.expanded = false;
                                                root.themeDropdownExpanded = false;
                                            }
                                        }

                                        Row {
                                            anchors.fill: parent
                                            anchors.margins: 10
                                            spacing: 10

                                            // Rose-pink gradient circle
                                            Rectangle {
                                                width: 28
                                                height: 28
                                                radius: 14
                                                gradient: Gradient {
                                                    orientation: Gradient.Horizontal
                                                    GradientStop {
                                                        position: 0.0
                                                        color: "#EC4899"
                                                    }
                                                    GradientStop {
                                                        position: 0.5
                                                        color: "#8B5CF6"
                                                    }
                                                    GradientStop {
                                                        position: 1.0
                                                        color: "#7C3AED"
                                                    }
                                                }
                                            }

                                            Column {
                                                anchors.verticalCenter: parent.verticalCenter
                                                Text {
                                                    text: "Frost Glass"
                                                    color: ThemeManager.currentTheme === 2 ? Style.primary : Style.textPrimary
                                                    font.pixelSize: 12
                                                    font.bold: ThemeManager.currentTheme === 2
                                                }
                                                Text {
                                                    text: "Premium light glass"
                                                    color: Qt.alpha(Style.textSecondary, 0.7)
                                                    font.pixelSize: 10
                                                }
                                            }

                                            Item {
                                                Layout.fillWidth: true
                                                width: 10
                                            }

                                            Text {
                                                text: "✓"
                                                color: Style.primary
                                                font.pixelSize: 14
                                                font.bold: true
                                                visible: ThemeManager.currentTheme === 2
                                                anchors.verticalCenter: parent.verticalCenter
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    // Separator
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Qt.alpha(Style.surfaceHighlight, 0.3)
                    }

                    // Start with Windows Row
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Start with Windows"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 180
                        }
                        Text {
                            text: "Launch Neo-Z automatically on system startup"
                            color: Qt.alpha(Style.textSecondary, 0.6)
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }
                        NeonToggle {
                            checked: false
                            activeColor: Style.primary
                        }
                    }

                    // Separator
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Qt.alpha(Style.surfaceHighlight, 0.3)
                    }

                    // Language Row
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Language"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 180
                        }
                        Text {
                            text: "Select your preferred language"
                            color: Qt.alpha(Style.textSecondary, 0.6)
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }
                        ComboBox {
                            id: languageCombo
                            model: ["English", "Español", "Deutsch", "Français", "日本語"]
                            implicitWidth: 130

                            background: Rectangle {
                                color: Qt.rgba(0, 0, 0, 0.4)
                                border.color: Style.surfaceHighlight
                                border.width: 1
                                radius: 6
                            }

                            contentItem: Text {
                                text: languageCombo.displayText
                                color: Style.textPrimary
                                font.pixelSize: 12
                                verticalAlignment: Text.AlignVCenter
                                leftPadding: 10
                            }
                        }
                    }
                }
            }

            // ============================================
            // SECTION: SAFETY
            // ============================================
            Text {
                text: "Safety"
                color: Style.primary
                font.pixelSize: 16
                font.bold: true
                Layout.topMargin: 10
            }

            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: contentColSafety.implicitHeight + 60
                glassOpacity: 0.25

                ColumnLayout {
                    id: contentColSafety
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    // Max Sensitivity Change Row
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Max Change Per Step"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 180
                        }
                        Text {
                            text: "Maximum sensitivity adjustment allowed per step"
                            color: Qt.alpha(Style.textSecondary, 0.6)
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }
                        Rectangle {
                            width: 60
                            height: 32
                            radius: 6
                            color: Qt.rgba(0, 0, 0, 0.4)
                            border.color: Style.primary
                            border.width: 1

                            Text {
                                anchors.centerIn: parent
                                text: "30%"
                                color: Style.primary
                                font.bold: true
                                font.pixelSize: 13
                            }
                        }
                    }

                    // Separator
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Qt.alpha(Style.surfaceHighlight, 0.3)
                    }

                    // Require Confirmation Row
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Require Confirmation"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 180
                        }
                        Text {
                            text: "Ask before applying sensitivity changes"
                            color: Qt.alpha(Style.textSecondary, 0.6)
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }
                        NeonToggle {
                            checked: true
                            activeColor: Style.primary
                        }
                    }
                }
            }

            // ============================================
            // SECTION: PRIVACY & DATA
            // ============================================
            Text {
                text: "Privacy & Data"
                color: Style.primary
                font.pixelSize: 16
                font.bold: true
                Layout.topMargin: 10
            }

            GlassPanel {
                Layout.fillWidth: true
                Layout.preferredHeight: contentColPrivacy.implicitHeight + 60
                glassOpacity: 0.25

                ColumnLayout {
                    id: contentColPrivacy
                    anchors.fill: parent
                    anchors.margins: 20
                    spacing: 16

                    // Analytics Row
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Enable Analytics"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 180
                        }
                        Text {
                            text: "Help improve Neo-Z with anonymous usage data"
                            color: Qt.alpha(Style.textSecondary, 0.6)
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }
                        NeonToggle {
                            checked: false
                            activeColor: Style.primary
                        }
                    }

                    // Separator
                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: Qt.alpha(Style.surfaceHighlight, 0.3)
                    }

                    // Logging Row
                    RowLayout {
                        Layout.fillWidth: true
                        Text {
                            text: "Enable Logging"
                            color: Style.textSecondary
                            font.pixelSize: 13
                            Layout.preferredWidth: 180
                        }
                        Text {
                            text: "Save application logs for troubleshooting"
                            color: Qt.alpha(Style.textSecondary, 0.6)
                            font.pixelSize: 11
                            Layout.fillWidth: true
                        }
                        NeonToggle {
                            checked: true
                            activeColor: Style.primary
                        }
                    }

                    // Info text
                    Rectangle {
                        Layout.fillWidth: true
                        Layout.topMargin: 4
                        height: 32
                        radius: 6
                        color: Qt.alpha(Style.primary, 0.08)
                        border.color: Qt.alpha(Style.primary, 0.2)
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 8

                            Text {
                                text: "🔒"
                                font.pixelSize: 12
                            }
                            Text {
                                text: "All data is stored locally. No data is sent to external servers unless AI features are used."
                                color: Style.textSecondary
                                font.pixelSize: 11
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }

            // Spacer at bottom
            Item {
                Layout.fillHeight: true
                Layout.minimumHeight: 30
            }
        }
    }
}
