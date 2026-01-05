pragma Singleton
import QtQuick

// Cyberpunk Color Palette for Win10 Optimizer
QtObject {
    id: cyberpunk

    // ===== PRIMARY NEON COLORS =====
    readonly property color neonBlue: "#00D4FF"      // Primary actions, main highlights
    readonly property color electricPurple: "#9D00FF" // Secondary actions, accents
    readonly property color matrixGreen: "#00FF41"   // Success, optimizations, AI text
    readonly property color warningOrange: "#FF6B00" // Warnings, medium priority
    readonly property color dangerRed: "#FF0055"     // Errors, critical issues

    // ===== BACKGROUND COLORS =====
    readonly property color darkCharcoal: "#0F0F1A"  // Main background
    readonly property color darkSlate: "#1A1A2E"    // Card backgrounds
    readonly property color deepSpace: "#0A0A14"    // Alternate sections
    readonly property color nebulaBlack: "#0A0A0F"  // Deep black for luxury panels

    // ===== GLASSMORPHISM =====
    readonly property color glassWhite: Qt.rgba(1, 1, 1, 0.08)
    readonly property color glassBorder: Qt.rgba(1, 1, 1, 0.12)
    readonly property color glassHighlight: Qt.rgba(1, 1, 1, 0.05)

    // ===== TEXT COLORS =====
    readonly property color glowingWhite: "#E0E0FF"  // Primary text with subtle glow
    readonly property color neonCyan: "#00FFFF"      // Headers, important labels
    readonly property color dimGray: "#888888"       // Secondary text, disabled
    readonly property color warningYellow: "#FFFF00" // Alerts, notices

    // ===== GRADIENT HELPERS =====
    readonly property color purpleGradientStart: "#7C3AED"
    readonly property color purpleGradientMid: "#9333EA"
    readonly property color purpleGradientEnd: "#C026D3"

    readonly property color blueGradientStart: "#0077FF"
    readonly property color blueGradientEnd: "#00D4FF"

    readonly property color greenGradientStart: "#00C853"
    readonly property color greenGradientEnd: "#00FF41"

    // ===== STATUS COLORS =====
    readonly property color statusOptimized: "#00FF41"
    readonly property color statusWarning: "#FF6B00"
    readonly property color statusCritical: "#FF0055"
    readonly property color statusNeutral: "#888888"

    // ===== ORB COLORS (for monitoring orbs) =====
    readonly property color cpuOrb: "#FF4444"        // Red
    readonly property color ramOrb: "#4488FF"        // Blue
    readonly property color diskOrb: "#44FF44"       // Green
    readonly property color networkOrb: "#FFFF44"    // Yellow
    readonly property color tempOrb: "#FF44FF"       // Magenta
    readonly property color powerOrb: "#44FFFF"      // Cyan

    // ===== GLOW EFFECTS =====
    function glowColor(baseColor, intensity) {
        return Qt.rgba(baseColor.r, baseColor.g, baseColor.b, intensity);
    }

    function neonGlow(baseColor) {
        return Qt.rgba(baseColor.r, baseColor.g, baseColor.b, 0.6);
    }

    function subtleGlow(baseColor) {
        return Qt.rgba(baseColor.r, baseColor.g, baseColor.b, 0.25);
    }
}
