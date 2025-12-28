import QtQuick
import QtMultimedia
import NeoZ
import "../style"

// AnimatedBackground - Apple-like Gradient with Smooth Color Shifting
// Glass → Vibrant looping gradient like Apple UI
// Dark → Static matte background
// Frost Glass → Video background (offset to not cover sidebar)
Item {
    id: root
    anchors.fill: parent

    // Property to receive sidebar width from parent
    property int sidebarWidth: 70

    // Only show video after boot completes (NeoLoader has its own video)
    property bool isBootComplete: false

    // ===== APPLE-LIKE GRADIENT BACKGROUND (Glass Theme) =====
    Rectangle {
        id: gradientBase
        anchors.fill: parent
        visible: Style.animatedBackground && !Style.isLightMode

        // Base gradient with animated color stops
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                id: stop1
                position: 0.0
                color: "#1a0a2e"  // Deep purple
                SequentialAnimation on color {
                    running: Style.animatedBackground && !Style.isLightMode
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: "#0d1b2a"
                        duration: 8000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#1b263b"
                        duration: 8000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#1a0a2e"
                        duration: 8000
                        easing.type: Easing.InOutSine
                    }
                }
            }
            GradientStop {
                id: stop2
                position: 0.4
                color: "#0d1b2a"  // Dark blue
                SequentialAnimation on color {
                    running: Style.animatedBackground && !Style.isLightMode
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: "#1b263b"
                        duration: 6000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#0f4c75"
                        duration: 6000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#0d1b2a"
                        duration: 6000
                        easing.type: Easing.InOutSine
                    }
                }
            }
            GradientStop {
                id: stop3
                position: 0.7
                color: "#1b263b"  // Navy
                SequentialAnimation on color {
                    running: Style.animatedBackground && !Style.isLightMode
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: "#3c096c"
                        duration: 7000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#240046"
                        duration: 7000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#1b263b"
                        duration: 7000
                        easing.type: Easing.InOutSine
                    }
                }
            }
            GradientStop {
                id: stop4
                position: 1.0
                color: "#3c096c"  // Vibrant purple
                SequentialAnimation on color {
                    running: Style.animatedBackground && !Style.isLightMode
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: "#5a189a"
                        duration: 5000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#7b2cbf"
                        duration: 5000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#3c096c"
                        duration: 5000
                        easing.type: Easing.InOutSine
                    }
                }
            }
        }

        // Accent color overlay (rotates through colors)
        Rectangle {
            id: accentOverlay
            anchors.fill: parent
            opacity: 0.25
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: "#00e5ff"
                    SequentialAnimation on color {
                        running: Style.animatedBackground
                        loops: Animation.Infinite
                        ColorAnimation {
                            to: "#9945FF"
                            duration: 10000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#FF6B6B"
                            duration: 10000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#00e5ff"
                            duration: 10000
                            easing.type: Easing.InOutSine
                        }
                    }
                }
                GradientStop {
                    position: 0.5
                    color: "transparent"
                }
                GradientStop {
                    position: 1.0
                    color: "#9945FF"
                    SequentialAnimation on color {
                        running: Style.animatedBackground
                        loops: Animation.Infinite
                        ColorAnimation {
                            to: "#00e5ff"
                            duration: 12000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#FF6B6B"
                            duration: 12000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#9945FF"
                            duration: 12000
                            easing.type: Easing.InOutSine
                        }
                    }
                }
            }
        }

        // Subtle noise/shimmer layer
        Rectangle {
            anchors.fill: parent
            opacity: 0.05
            property real shimmerOffset: 0
            NumberAnimation on shimmerOffset {
                running: Style.animatedBackground
                loops: Animation.Infinite
                from: 0
                to: 100
                duration: 30000
            }
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: Qt.rgba(1, 1, 1, 0.1)
                }
                GradientStop {
                    position: 0.3
                    color: "transparent"
                }
                GradientStop {
                    position: 0.7
                    color: Qt.rgba(1, 1, 1, 0.05)
                }
                GradientStop {
                    position: 1.0
                    color: "transparent"
                }
            }
        }
    }

    // ===== FROST GLASS LIGHT MODE BACKGROUND (DISABLED - replaced by video) =====
    Rectangle {
        id: lightGradientBase
        anchors.fill: parent
        visible: false  // Disabled - using video background instead

        // Soft pastel gradient with animated color stops
        gradient: Gradient {
            orientation: Gradient.Vertical
            GradientStop {
                position: 0.0
                color: "#F8F0FC"  // Light lavender
                SequentialAnimation on color {
                    running: Style.animatedBackground && Style.isLightMode
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: "#FDF2F8"
                        duration: 8000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#F5F3FF"
                        duration: 8000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#F8F0FC"
                        duration: 8000
                        easing.type: Easing.InOutSine
                    }
                }
            }
            GradientStop {
                position: 0.5
                color: "#FAFBFC"  // Clean white
                SequentialAnimation on color {
                    running: Style.animatedBackground && Style.isLightMode
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: "#FEF3F2"
                        duration: 6000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#F0FDF4"
                        duration: 6000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#FAFBFC"
                        duration: 6000
                        easing.type: Easing.InOutSine
                    }
                }
            }
            GradientStop {
                position: 1.0
                color: "#FDF2F8"  // Soft pink
                SequentialAnimation on color {
                    running: Style.animatedBackground && Style.isLightMode
                    loops: Animation.Infinite
                    ColorAnimation {
                        to: "#F5F3FF"
                        duration: 7000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#FCE7F3"
                        duration: 7000
                        easing.type: Easing.InOutSine
                    }
                    ColorAnimation {
                        to: "#FDF2F8"
                        duration: 7000
                        easing.type: Easing.InOutSine
                    }
                }
            }
        }

        // Subtle accent overlay for light mode
        Rectangle {
            anchors.fill: parent
            opacity: 0.12
            gradient: Gradient {
                orientation: Gradient.Horizontal
                GradientStop {
                    position: 0.0
                    color: "#EC4899"  // Rose pink
                    SequentialAnimation on color {
                        running: Style.animatedBackground && Style.isLightMode
                        loops: Animation.Infinite
                        ColorAnimation {
                            to: "#8B5CF6"
                            duration: 10000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#7C3AED"
                            duration: 10000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#EC4899"
                            duration: 10000
                            easing.type: Easing.InOutSine
                        }
                    }
                }
                GradientStop {
                    position: 0.5
                    color: "transparent"
                }
                GradientStop {
                    position: 1.0
                    color: "#8B5CF6"  // Soft purple
                    SequentialAnimation on color {
                        running: Style.animatedBackground && Style.isLightMode
                        loops: Animation.Infinite
                        ColorAnimation {
                            to: "#EC4899"
                            duration: 12000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#7C3AED"
                            duration: 12000
                            easing.type: Easing.InOutSine
                        }
                        ColorAnimation {
                            to: "#8B5CF6"
                            duration: 12000
                            easing.type: Easing.InOutSine
                        }
                    }
                }
            }
        }
    }

    // ===== STATIC BACKGROUND (Dark or Light Minimal) =====
    Rectangle {
        anchors.fill: parent
        visible: !Style.animatedBackground
        color: Style.mainBgColor

        // Subtle vignette (only for dark mode)
        Rectangle {
            anchors.fill: parent
            visible: !Style.isLightMode
            gradient: Gradient {
                orientation: Gradient.Vertical
                GradientStop {
                    position: 0.0
                    color: "transparent"
                }
                GradientStop {
                    position: 0.9
                    color: Qt.rgba(0, 0, 0, 0.3)
                }
            }
        }
    }

    // ===== FROST GLASS BACKGROUND (VIDEO DISABLED) =====
    // Static white background for light theme
    Rectangle {
        id: frostBackground
        anchors.fill: parent
        visible: Style.animatedBackground && Style.isLightMode
        color: "#FAFBFC"
        z: 0
    }
}
