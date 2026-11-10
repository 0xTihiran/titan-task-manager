import QtQuick
import QtQuick.Layouts
import "."

Item {
    id: root
    property var process: ({})
    implicitHeight: 32
    height: 32
    Layout.fillWidth: true
    clip: true

    Rectangle { anchors.fill: parent; radius: 7; color: Theme.surfaceRaised; opacity: 0.55 }
    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10 + Math.min(Number(root.process.depth || 0), 12) * 16
        anchors.rightMargin: 12
        spacing: 8
        Text { text: root.process.isRoot ? "◆" : "↳"; color:root.process.isRoot ? Theme.accent : Theme.textLow; font.pixelSize:12 }
        Text { text: root.process.name || "(unknown)"; color:Theme.textPrimary; font.pixelSize:12; elide:Text.ElideRight; Layout.fillWidth:true }
        Text { text: "PID " + String(root.process.pid); color:Theme.textLow; font.pixelSize:11; Layout.preferredWidth:78 }
        Text { text: Number(root.process.cpu || 0).toFixed(1) + "% CPU"; color:Theme.textSecondary; font.pixelSize:11; Layout.preferredWidth:82 }
        Text { text: Number(root.process.memoryMiB || 0).toFixed(1) + " MiB"; color:Theme.textSecondary; font.pixelSize:11; Layout.preferredWidth:80 }
        Text { text: root.process.state || "?"; color:Theme.textLow; font.pixelSize:11; Layout.preferredWidth:15 }
    }
}
