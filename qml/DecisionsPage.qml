import QtQuick
import QtQuick.Layouts
import "."
import "components"
Item {
    ColumnLayout {
        anchors.fill:parent
        spacing:18
        RowLayout {
            Layout.fillWidth:true
            ColumnLayout {
                spacing:5
                Text { text:"THM decisions"; font.family:Theme.font; font.pixelSize:23; font.weight:Font.Bold; color:Theme.textPrimary }
                Text { text:"Read-only events reported by the Titan Hardware Manager"; font.family:Theme.font; font.pixelSize:12; color:Theme.textSecondary }
            }
            Item { Layout.fillWidth:true }
            StatusChip { label:thm.available ? "Telemetry connected" : "Offline"; tone:thm.available ? Theme.success : Theme.warning }
        }
        Panel {
            Layout.fillWidth:true
            Layout.preferredHeight:160
            ColumnLayout {
                anchors.fill:parent
                anchors.margins:20
                spacing:15
                SectionTitle { Layout.fillWidth:true; title:"Latest published action"; hint:"THM last_action field" }
                Text { text:thm.lastDecision || "No recent action was published."; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:15; wrapMode:Text.WordWrap; Layout.fillWidth:true }
                Text { text:"Monitoring only. THM policy cannot be overridden from this UI."; color:Theme.textLow; font.family:Theme.font; font.pixelSize:11 }
            }
        }
        Panel {
            Layout.fillWidth:true
            Layout.fillHeight:true
            EmptyView {
                anchors.fill:parent
                symbol:"◷"
                heading:"Historical decisions coming next"
                detail:"The THM socket's read-only 'decisions [N]' command will populate this list in a later backend milestone."
            }
        }
    }
}
