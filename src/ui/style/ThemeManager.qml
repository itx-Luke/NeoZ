pragma Singleton
import QtQuick

QtObject {
    id: themeManager

    // ===== Theme Enum =====
    enum Theme {
        DarkMinimal,    // 0 - Default matte dark
        GlassGradient,  // 1 - Animated liquid glass
        NeonPulse       // 2 - Future: Vibrant neon
    }

    property int currentTheme: Backend.theme  // Load from persisted config

    // ===== Skin Property Bundles =====
    readonly property var skins: ({
            0: {
                // Dark Minimal
                name: "Dark Minimal",
                loaderBgColor: "#0A0A0A",
                mainBgColor: "#0A0A0A",
                containerOpacity: 0.04,
                containerBlur: 0,
                containerBorderOpacity: 0.15,
                glowEnabled: false,
                glowIntensity: 0,
                animatedBackground: false,
                gradientEnabled: false,
                shimmerEnabled: false,
                isLightMode: false,
                // Color palette
                primaryColor: "#00E5FF",
                accentGradientStart: "#00E5FF",
                accentGradientEnd: "#9945FF",
                // Bootloader theme
                loaderVideoEnabled: false,
                loaderVideoPath: "",
                // HUD Theme (for plugin support)
                hud: {
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
                }
            },
            1: {
                // Glass Gradient
                name: "Gradient Variant",
                loaderBgColor: "#050510",
                mainBgColor: "#050510",
                containerOpacity: 0.85,
                containerBlur: 25,
                containerBorderOpacity: 0.3,
                glowEnabled: true,
                glowIntensity: 0.6,
                animatedBackground: true,
                gradientEnabled: true,
                shimmerEnabled: true,
                isLightMode: false,
                // Color palette
                primaryColor: "#00E5FF",
                accentGradientStart: "#00E5FF",
                accentGradientEnd: "#9945FF",
                // Bootloader theme
                loaderVideoEnabled: false,
                loaderVideoPath: "",
                // HUD Theme (for plugin support)
                hud: {
                    tileBackground: "rgba(8, 6, 15, 0.65)",
                    tileBorder: "rgba(255, 255, 255, 0.1)",
                    tileBorderActive: "rgba(0, 229, 255, 0.5)",
                    tileRadius: 16,
                    tileGlowEnabled: true,
                    statusEmulator: "#00e676",
                    statusAdb: "#00e5ff",
                    statusInput: "#00e676",
                    statusDisplay: "#64b5f6",
                    statusScript: "#ba68c8",
                    statusAi: "#ba68c8",
                    labelColor: "#9AA4B2",
                    valueColor: "#FFFFFF"
                }
            },
            2: {
                // Frost Glass - Premium Light Theme
                name: "Frost Glass",
                loaderBgColor: "#F5F7FA",
                mainBgColor: "#FAFBFC",
                containerOpacity: 0.75,
                containerBlur: 20,
                containerBorderOpacity: 0.25,
                glowEnabled: true,
                glowIntensity: 0.4,
                animatedBackground: true,
                gradientEnabled: true,
                shimmerEnabled: true,
                isLightMode: true,
                // Premium pastel palette - bubbly 3D liquid glass
                primaryColor: "#7C3AED"         // Rich violet
                ,
                accentGradientStart: "#EC4899"  // Rose pink
                ,
                accentGradientEnd: "#8B5CF6"    // Soft purple
                ,
                // Bootloader theme
                loaderVideoEnabled: true,
                loaderVideoPath: "qrc:/qt/qml/NeoZ/assets/frost_background.mp4",
                // HUD Theme (for plugin support) - Light mode
                hud: {
                    tileBackground: "rgba(255, 255, 255, 0.85)",
                    tileBorder: "rgba(0, 0, 0, 0.08)",
                    tileBorderActive: "rgba(124, 58, 237, 0.5)",
                    tileRadius: 16,
                    tileGlowEnabled: true,
                    statusEmulator: "#10b981"  // Soft green
                    ,
                    statusAdb: "#7C3AED"       // Violet
                    ,
                    statusInput: "#10b981"     // Soft green
                    ,
                    statusDisplay: "#3b82f6"   // Blue
                    ,
                    statusScript: "#EC4899"    // Rose pink
                    ,
                    statusAi: "#8B5CF6"        // Soft purple
                    ,
                    labelColor: "#334155"      // Darker slate
                    ,
                    valueColor: "#000000"       // Pure black
                }
            }
        })

    // ===== Current Skin Accessor =====
    readonly property var currentSkin: skins[currentTheme] ?? skins[0]

    // ===== Convenience Accessors =====
    readonly property bool isGlass: currentSkin.gradientEnabled
    readonly property bool animatedBackground: currentSkin.animatedBackground
    readonly property bool glowEnabled: currentSkin.glowEnabled
    readonly property bool shimmerEnabled: currentSkin.shimmerEnabled
    readonly property bool isLightMode: currentSkin.isLightMode || false

    // ===== Theme Transition Signal =====
    signal themeTransitionRequested(int newTheme)
    property int pendingTheme: -1
    property bool isTransitioning: false

    // ===== Request Theme Change (triggers animation) =====
    function setTheme(theme) {
        console.log("[ThemeManager] setTheme called with:", theme);

        if (currentTheme === theme) {
            console.log("[ThemeManager] Already on theme", theme, "- no change");
            return;
        }

        if (theme < 0 || theme > 2) {
            console.error("[ThemeManager] ERROR: Invalid theme index:", theme);
            return;
        }

        if (isTransitioning) {
            console.log("[ThemeManager] Transition in progress, ignoring click");
            return;
        }

        // Store pending theme and request transition
        pendingTheme = theme;
        isTransitioning = true;
        console.log("[ThemeManager] Requesting transition to theme:", theme);
        themeTransitionRequested(theme);
    }

    // ===== Apply Theme (called after fade animation) =====
    function applyPendingTheme() {
        if (pendingTheme < 0)
            return;

        var oldTheme = currentTheme;
        currentTheme = pendingTheme;
        pendingTheme = -1;

        console.log("[ThemeManager] Theme switched:", oldTheme, "->", currentTheme);
        console.log("[ThemeManager] Current skin:", currentSkin.name);
        console.log("[ThemeManager] isGlass:", isGlass, "animatedBackground:", animatedBackground);
    }

    // ===== Complete Transition =====
    function completeTransition() {
        isTransitioning = false;
        console.log("[ThemeManager] Transition complete");
    }
}
