pragma Singleton
import QtQuick
import ArchTitan.TitanUI

// Native ArchTitan semantic palette bridge. No preview literals.
// Other Task Manager pages keep the same readable Theme.* contract while
// every color, typography and radius comes from the OS's real TitanTheme.
QtObject {
    readonly property color canvas:        TitanTheme.bg0
    readonly property color sidebar:       TitanTheme.bg1
    readonly property color surface:       TitanTheme.bg3
    readonly property color surfaceRaised: TitanTheme.bg4
    readonly property color surfaceHover:  TitanTheme.stateHover
    readonly property color border:        TitanTheme.borderSubtle
    readonly property color textPrimary:   TitanTheme.textHigh
    readonly property color textSecondary: TitanTheme.textMid
    readonly property color textLow:       TitanTheme.textLow
    readonly property color accent:        TitanTheme.accent
    readonly property color accentSoft:    TitanTheme.accentSoft
    readonly property color success:       TitanTheme.success
    readonly property color successSoft:   TitanTheme.alphaColor(TitanTheme.success, 0.16)
    readonly property color warning:       TitanTheme.warning
    readonly property color danger:        TitanTheme.danger
    readonly property color chartFill:     TitanTheme.alphaColor(TitanTheme.accent, 0.12)
    readonly property int radius:          TitanTheme.radiusCard
    readonly property string font:         TitanTheme.fontFamily
}
