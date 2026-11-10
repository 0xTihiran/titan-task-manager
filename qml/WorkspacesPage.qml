import QtQuick
import QtQuick.Layouts
import "."
import "components"
Item {
    id:page
    function labels() { return [1,2,3,4,5,6,7,8,9,10,-2,-1] }
    function info(id) {
        var count=0,mem=0,cpu=0
        var list=thm.workloads
        for(var i=0;i<list.length;i++) if(Number(list[i].workspace)===id) {
            count++
            mem+=Number(list[i].mem_mb||0)
            cpu+=Number(list[i].cpu_pct||0)
        }
        return {count:count,mem:mem,cpu:cpu}
    }
    Flickable {
        anchors.fill:parent
        contentWidth:width
        contentHeight:body.implicitHeight+24
        clip:true
        ColumnLayout {
            id:body
            width:parent.width-4
            spacing:18
            RowLayout {
                Layout.fillWidth:true
                ColumnLayout {
                    spacing:5
                    Text { text:"Workspace map"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:23; font.weight:Font.Bold }
                    Text { text:"Live workload distribution across Hyprland workspaces"; color:Theme.textSecondary; font.family:Theme.font; font.pixelSize:12 }
                }
                Item { Layout.fillWidth:true }
                StatusChip { label:hyprland.connected ? "Hyprland events connected" : "Telemetry fallback"; tone:hyprland.connected ? Theme.success : Theme.warning }
            }
            Panel {
                Layout.fillWidth:true
                Layout.preferredHeight:56
                RowLayout {
                    anchors.fill:parent
                    anchors.margins:15
                    Rectangle { width:8; height:8; radius:4; color:hyprland.connected ? Theme.success : Theme.warning }
                    Text { text:"Focused workspace: "+(hyprland.connected && hyprland.focusedWorkspace>0 ? String(hyprland.focusedWorkspace) : "waiting for event")+". Workload counts are reported by THM; focus events come from Hyprland socket2."; color:Theme.textSecondary; font.family:Theme.font; font.pixelSize:12; wrapMode:Text.WordWrap; Layout.fillWidth:true }
                }
            }
            GridLayout {
                Layout.fillWidth:true
                columns:3
                rowSpacing:13
                columnSpacing:13
                Repeater {
                    model:page.labels()
                    delegate:Panel {
                        required property var modelData
                        property var stats:page.info(Number(modelData))
                        Layout.fillWidth:true
                        Layout.preferredHeight:145
                        border.color:hyprland.connected && Number(modelData)===hyprland.focusedWorkspace ? Theme.accent : Theme.border
                        border.width:hyprland.connected && Number(modelData)===hyprland.focusedWorkspace ? 2 : 1
                        ColumnLayout {
                            anchors.fill:parent
                            anchors.margins:17
                            spacing:10
                            RowLayout {
                                Text { text:modelData===-2 ? "Headless pool" : (modelData===-1 ? "Unknown" : "Workspace "+modelData); font.family:Theme.font; font.pixelSize:14; font.weight:Font.DemiBold; color:Theme.textPrimary }
                                Item { Layout.fillWidth:true }
                                StatusChip { visible:hyprland.connected && Number(modelData)===hyprland.focusedWorkspace; label:"Focused"; tone:Theme.accent }
                                Rectangle { width:9; height:9; radius:5; color:stats.count>0 ? Theme.accent : Theme.border }
                            }
                            Text { text:String(stats.count)+" workloads"; font.family:Theme.font; font.pixelSize:20; font.weight:Font.Bold; color:Theme.textPrimary }
                            RowLayout {
                                Text { text:stats.cpu.toFixed(1)+"% CPU"; color:Theme.textSecondary; font.pixelSize:11 }
                                Item { Layout.fillWidth:true }
                                Text { text:stats.mem.toFixed(0)+" MiB"; color:Theme.textSecondary; font.pixelSize:11 }
                            }
                        }
                    }
                }
            }
        }
    }
}
