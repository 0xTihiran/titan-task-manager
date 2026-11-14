pragma Singleton
import QtQuick

// Dark design-token PREVIEW. Not the official ArchTitan.TitanUI TitanTheme.
// No page/component hardcodes a color: replace these values by aliases to the
// repository's real TitanTheme before merging into ArchTitan OS.
QtObject {
    readonly property color canvas: Qt.rgba(0.043,0.057,0.080,1)
    readonly property color sidebar: Qt.rgba(0.058,0.075,0.106,1)
    readonly property color surface: Qt.rgba(0.078,0.099,0.135,1)
    readonly property color surfaceRaised: Qt.rgba(0.100,0.124,0.164,1)
    readonly property color surfaceHover: Qt.rgba(0.128,0.156,0.198,1)
    readonly property color border: Qt.rgba(0.180,0.217,0.279,1)
    readonly property color textPrimary: Qt.rgba(0.949,0.966,0.992,1)
    readonly property color textSecondary: Qt.rgba(0.616,0.666,0.740,1)
    readonly property color textLow: Qt.rgba(0.465,0.525,0.617,1)
    readonly property color accent: Qt.rgba(0.381,0.568,1,1)
    readonly property color accentSoft: Qt.rgba(0.126,0.184,0.337,1)
    readonly property color success: Qt.rgba(0.317,0.849,0.651,1)
    readonly property color successSoft: Qt.rgba(0.103,0.235,0.204,1)
    readonly property color warning: Qt.rgba(1,0.711,0.338,1)
    readonly property color danger: Qt.rgba(1,0.405,0.470,1)
    readonly property color chartFill: Qt.rgba(0.381,0.568,1,0.12)
    readonly property int radius: 16
    readonly property string font: "Noto Sans"
}
