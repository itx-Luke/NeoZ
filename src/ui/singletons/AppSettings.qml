pragma Singleton
import QtQuick
import Qt.labs.settings 1.0 as LabsSettings

/**
 * @brief Centralized settings singleton for Neo-Z
 *
 * Usage:
 *   AppSettings.general.lastDevice = "127.0.0.1:5555"
 *   AppSettings.sensitivity.xMultiplier = 1.5
 */
QtObject {
    id: root

    // General Settings
    property LabsSettings.Settings general: LabsSettings.Settings {
        category: "General"
        property string lastDevice: ""
        property bool rememberWindow: true
        property int windowWidth: 1280
        property int windowHeight: 720
    }

    // Theme Settings
    property LabsSettings.Settings theme: LabsSettings.Settings {
        category: "Theme"
        property int activeTheme: 1  // 0=Dark, 1=GlassGradient
        property string customWallpaper: ""
        property string customContainerColor: ""
        property string customTextColor: ""
    }

    // Sensitivity Settings
    property LabsSettings.Settings sensitivity: LabsSettings.Settings {
        category: "Sensitivity"
        property real xMultiplier: 1.0
        property real yMultiplier: 1.0
        property string curve: "Linear"
        property int slowZone: 50
        property int smoothing: 50
        property int mouseDpi: 800
    }

    // AI Settings
    property LabsSettings.Settings ai: LabsSettings.Settings {
        category: "AI"
        property bool enabled: false
        property real confidenceThreshold: 0.7
        property string geminiApiKey: ""
    }

    // DRCS Settings
    property LabsSettings.Settings drcs: LabsSettings.Settings {
        category: "DRCS"
        property bool enabled: false
        property real repetitionTolerance: 4.0
        property real directionThreshold: 0.95
    }

    // Convenience methods
    function resetToDefaults() {
        sensitivity.xMultiplier = 1.0;
        sensitivity.yMultiplier = 1.0;
        sensitivity.curve = "Linear";
        sensitivity.slowZone = 50;
        sensitivity.smoothing = 50;
        ai.enabled = false;
        ai.confidenceThreshold = 0.7;
        drcs.enabled = false;
    }
}
