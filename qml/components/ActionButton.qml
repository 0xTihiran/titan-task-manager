import QtQuick
import ArchTitan.TitanUI

// Backwards-compatible wrapper using the *actual* shared TitanButton.
Item {
    id: root
    signal clicked()
    property string label: "Action"
    property string symbol: "↻"
    property bool highlighted: false
    implicitWidth: nativeButton.implicitWidth
    implicitHeight: nativeButton.implicitHeight

    TitanButton {
        id: nativeButton
        anchors.fill: parent
        text: root.label
        glyph: root.symbol
        variant: root.highlighted ? "primary" : "secondary"
        onClicked: root.clicked()
    }
}
