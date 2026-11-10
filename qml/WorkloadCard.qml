import QtQuick
import QtQuick.Layouts
import "."
import "components"

Panel {
    id: card
    property var workload: ({})
    property bool expanded: false
    signal toggleRequested()
    readonly property int visibleRows: Math.min((workload.tree || []).length, 60)
    height: expanded ? 141 + visibleRows * 35 + 18 : 112
    clip: true

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 15
        spacing: 9
        Item {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            RowLayout {
                anchors.fill: parent
                spacing: 13
                Rectangle {
                    width:41; height:41; radius:12
                    color:Theme.accentSoft
                    Text { anchors.centerIn:parent; text:"◈"; font.pixelSize:25; color:Theme.accent }
                }
                ColumnLayout {
                    Layout.fillWidth:true
                    spacing:6
                    RowLayout {
                        spacing:9
                        Text { text:card.workload.name || "Unnamed"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:15; font.weight:Font.DemiBold; elide:Text.ElideRight; Layout.maximumWidth:275 }
                        StatusChip { label:card.workload.type || "NEUTRAL"; tone:Theme.accent }
                        StatusChip { label:card.workload.stale ? "STALE ROOT" : (card.workload.state || "DISCOVERED"); tone: card.workload.stale ? Theme.warning : (card.workload.state==="ACTIVE" ? Theme.success : Theme.textLow) }
                    }
                    RowLayout {
                        spacing:12
                        Text { text:"Root PID " + String(card.workload.pid || "—"); color:Theme.textLow; font.pixelSize:11 }
                        Text { text:"Workspace " + (card.workload.workspace===-2?"Headless":(card.workload.workspace===-1?"Unknown":card.workload.workspace)); color:Theme.textLow; font.pixelSize:11 }
                        Text { text:String(card.workload.matchedCount || 0) + " linked / " + String(card.workload.pids || 0) + " reported"; color:Theme.textLow; font.pixelSize:11 }
                    }
                }
                ColumnLayout {
                    spacing:6
                    Text { text:Number(card.workload.cpu_pct || 0).toFixed(1) + "% CPU"; color:Theme.textPrimary; font.weight:Font.Bold; font.pixelSize:14; Layout.alignment:Qt.AlignRight }
                    Text { text:String(card.workload.mem_mb || 0) + " MiB RAM"; color:Theme.textSecondary; font.pixelSize:11; Layout.alignment:Qt.AlignRight }
                    Text { text:card.expanded ? "▴ Collapse" : "▾ Expand"; color:Theme.accent; font.pixelSize:11; Layout.alignment:Qt.AlignRight }
                }
            }
            MouseArea {
                anchors.fill:parent
                cursorShape:Qt.PointingHandCursor
                onClicked:card.toggleRequested()
            }
        }
        ColumnLayout {
            visible:card.expanded
            Layout.fillWidth:true
            spacing:4
            Rectangle { Layout.fillWidth:true; height:1; color:Theme.border }
            Text {
                text:card.workload.stale ? "Root PID is not present in /proc; no process tree can be confirmed." : "Reconstructed /proc process lineage · THM-managed (read-only)"
                font.family:Theme.font; color:Theme.textSecondary; font.pixelSize:11
                Layout.fillWidth:true; wrapMode:Text.WordWrap
            }
            Repeater {
                model:card.expanded ? (card.workload.tree || []).slice(0,60) : []
                delegate:ProcessTreeRow {
                    required property var modelData
                    process:modelData
                    Layout.fillWidth:true
                }
            }
            Text {
                visible:card.workload.matchedCount > 60
                text:"Displaying first 60 linked processes; use Processes for the full list."
                color:Theme.warning; font.pixelSize:11
            }
        }
        Item { Layout.fillHeight:true }
    }
}
