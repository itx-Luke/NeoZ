pragma Singleton
import QtQuick

// Style - Purely reactive to ThemeManager.currentSkin
// No setters. No theme logic. Everything reacts to ThemeManager skin properties.
QtObject {
    id: style

    // ===== Theme State (reactive to skin) =====
    readonly property bool isGlass: ThemeManager.currentSkin.gradientEnabled
    readonly property bool isDark: !ThemeManager.currentSkin.gradientEnabled
    readonly property bool isLightMode: ThemeManager.isLightMode

    // ===== Skin-Reactive Background Colors =====
    readonly property color loaderBgColor: ThemeManager.currentSkin.loaderBgColor
    readonly property color mainBgColor: ThemeManager.currentSkin.mainBgColor

    // ===== Skin-Reactive Container Properties =====
    readonly property real containerOpacity: ThemeManager.currentSkin.containerOpacity
    readonly property real containerBlur: ThemeManager.currentSkin.containerBlur
    readonly property real containerBorderOpacity: ThemeManager.currentSkin.containerBorderOpacity
    readonly property bool glowEnabled: ThemeManager.currentSkin.glowEnabled
    readonly property real glowIntensity: ThemeManager.currentSkin.glowIntensity
    readonly property bool animatedBackground: ThemeManager.currentSkin.animatedBackground
    readonly property bool shimmerEnabled: ThemeManager.currentSkin.shimmerEnabled

    // ===== Base Colors =====
    readonly property color background: isLightMode ? "#F5F7FA" : (isGlass ? "#0B0F1A" : "#0A0A0F")
    readonly property color surface: isLightMode ? "#FFFFFF" : (isGlass ? "#131826" : "#0D0D12")
    readonly property color surfaceHighlight: isLightMode ? "#E8ECF0" : (isGlass ? "#1E2538" : "#151518")

    // ===== Accent Colors =====
    readonly property color primary: ThemeManager.currentSkin.primaryColor
    readonly property color secondary: isGlass ? "#E040FB" : "#9C27B0"
    readonly property color accentSecondary: isGlass ? "#9945FF" : "#3D3D4A"
    readonly property color accentGradientStart: ThemeManager.currentSkin.accentGradientStart
    readonly property color accentGradientEnd: ThemeManager.currentSkin.accentGradientEnd

    // ===== Text Colors =====
    readonly property color textPrimary: isLightMode ? "#000000" : "#FFFFFF"  // Pure black for light mode
    readonly property color textSecondary: isLightMode ? "#334155" : (isGlass ? "#9AA4B2" : "#6B6B75")  // Darker slate for light mode

    // ===== Glass Control (legacy compatibility) =====
    readonly property real glassOpacity: containerOpacity
    readonly property real blurStrength: containerBlur > 0 ? 0.7 : 0.0
    readonly property real sidebarOpacity: isGlass ? 0.4 : 0.85
    readonly property real tileOpacity: isGlass ? 0.6 : 0.9

    // ===== Borders =====
    readonly property color border: isLightMode ? Qt.rgba(0, 0, 0, 0.1) : (isGlass ? Qt.rgba(1, 1, 1, 0.1) : Qt.rgba(1, 1, 1, 0.06))
    readonly property int borderWidth: 1
    readonly property int borderRadius: isGlass ? 14 : 8
    readonly property int radius: borderRadius

    // ===== Motion =====
    readonly property int motionDuration: isGlass ? 220 : 120

    // ===== Static Accent Colors =====
    readonly property color danger: "#E53935"
    readonly property color success: "#00C853"
    readonly property color warning: "#FF9800"
    readonly property color error: "#FF0000"

    // ===== Metrics =====
    readonly property int sidebarWidth: 260
    readonly property int sidebarCollapsedWidth: 80
    readonly property int headerHeight: 70
    readonly property int spacing: 16

    // ===== HUD Theme (plugin/skin support) =====
    readonly property var hud: ThemeManager.currentSkin.hud ?? defaultHud
    readonly property var defaultHud: ({
            tileBackground: "rgba(15, 15, 20, 0.9)",
            tileBorder: "rgba(255, 255, 255, 0.1)",
            tileBorderActive: "rgba(255, 255, 255, 0.25)",
            tileRadius: 8,
            tileGlowEnabled: false,
            statusEmulator: "#00e676",
            statusAdb: "#00e5ff",
            statusInput: "#00e676",
            statusDisplay: "#64b5f6",
            statusScript: "#ba68c8",
            statusAi: "#ba68c8",
            labelColor: "#6B6B75",
            valueColor: "#FFFFFF"
        })

    // HUD color accessors for easy component usage
    readonly property color hudTileBackground: hud.tileBackground
    readonly property color hudTileBorder: hud.tileBorder
    readonly property color hudTileBorderActive: hud.tileBorderActive
    readonly property int hudTileRadius: hud.tileRadius
    readonly property bool hudTileGlowEnabled: hud.tileGlowEnabled
    readonly property color hudStatusEmulator: hud.statusEmulator
    readonly property color hudStatusAdb: hud.statusAdb
    readonly property color hudStatusInput: hud.statusInput
    readonly property color hudStatusDisplay: hud.statusDisplay
    readonly property color hudStatusScript: hud.statusScript
    readonly property color hudStatusAi: hud.statusAi
    readonly property color hudLabelColor: hud.labelColor
    readonly property color hudValueColor: hud.valueColor

    // ===== Fonts =====
    readonly property string fontFamily: "Segoe UI"
    readonly property font headerFont: Qt.font({
        family: fontFamily,
        pixelSize: 24,
        weight: Font.Bold
    })
    readonly property font bodyFont: Qt.font({
        family: fontFamily,
        pixelSize: 14,
        weight: Font.Normal
    })

    // ===== NeoZ Theme System Additions =====

    // Animation speed multiplier (1.0 = normal, 0.5 = fast, 2.0 = slow)
    readonly property real animationSpeed: 1.0

    // Corner radius for components
    readonly property int cornerRadius: borderRadius

    // Shadow color
    readonly property color shadowColor: Qt.rgba(0, 0, 0, 0.5)

    // Default font alias
    readonly property font defaultFont: bodyFont

    // Accent color alias
    readonly property color accentColor: primary

    // ===== Helper Functions =====

    // Darken a color by a percentage (0-100)
    function darkenColor(baseColor, amount) {
        return Qt.darker(baseColor, 1 + amount / 100);
    }

    // Lighten a color by a percentage (0-100)
    function lightenColor(baseColor, amount) {
        return Qt.lighter(baseColor, 1 + amount / 100);
    }
}
