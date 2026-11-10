import QtQuick
import QtQuick.Layouts
import "."
import "components"
Item {
    id:page
    property string searchText:""
    property string filter:"All"
    property string sortMode:"CPU"
    property var expandedById: ({})

    function toggleWorkload(key) {
        var newMap={}
        for (var k in expandedById) newMap[k]=expandedById[k]
        newMap[key] = !newMap[key]
        expandedById=newMap
    }
    function selectedWorkloads() {
        var items=workloadTree.workloads.slice()
        var needle=searchText.trim().toLowerCase()
        items=items.filter(function(w) {
            if(filter==="Classified" && w.observed===true) return false
            if(filter==="Observed" && w.observed!==true) return false
            return !needle || String(w.name||"").toLowerCase().indexOf(needle)>=0 ||
                   String(w.type||"").toLowerCase().indexOf(needle)>=0 ||
                   String(w.pid||"").indexOf(needle)>=0
        })
        if(sortMode==="Memory") items.sort(function(a,b){return Number(b.mem_mb||0)-Number(a.mem_mb||0)})
        else if(sortMode==="Workspace") items.sort(function(a,b){return Number(a.workspace||0)-Number(b.workspace||0)})
        else if(sortMode==="State") items.sort(function(a,b){return String(a.state).localeCompare(String(b.state))})
        else items.sort(function(a,b){return Number(b.cpu_pct||0)-Number(a.cpu_pct||0)})
        return items
    }

    ColumnLayout {
        anchors.fill:parent
        spacing:14
        RowLayout {
            Layout.fillWidth:true
            ColumnLayout {
                spacing:5
                Text { text:"THM workloads"; font.family:Theme.font; font.pixelSize:23; font.weight:Font.Bold; color:Theme.textPrimary }
                Text { text:"Expand workloads to inspect their Linux process trees"; font.family:Theme.font; font.pixelSize:12; color:Theme.textSecondary }
            }
            Item { Layout.fillWidth:true }
            StatusChip { label:thm.available ? (String(thm.workloadCount)+" tracked") : "THM offline"; tone:thm.available ? Theme.accent : Theme.warning }
        }
        RowLayout {
            spacing:7
            Repeater {
                model:["All","Classified","Observed"]
                delegate:Rectangle {
                    required property string modelData
                    width:label.implicitWidth+24; height:33; radius:9
                    color:page.filter===modelData ? Theme.accentSoft : Theme.surface
                    border.color:page.filter===modelData ? Theme.accent : Theme.border
                    Text { id:label; text:modelData; anchors.centerIn:parent; color:Theme.textSecondary; font.pixelSize:12 }
                    MouseArea { anchors.fill:parent; cursorShape:Qt.PointingHandCursor; onClicked:page.filter=modelData }
                }
            }
            Item { Layout.fillWidth:true }
            Text { text:"SORT"; color:Theme.textLow; font.pixelSize:10 }
            Repeater {
                model:["CPU","Memory","Workspace","State"]
                delegate:Rectangle {
                    required property string modelData
                    width:sortText.implicitWidth+18; height:33; radius:9
                    color:page.sortMode===modelData ? Theme.accentSoft : Theme.surface
                    border.color:page.sortMode===modelData ? Theme.accent : Theme.border
                    Text { id:sortText; text:modelData; anchors.centerIn:parent; color:Theme.textSecondary; font.pixelSize:11 }
                    MouseArea { anchors.fill:parent; cursorShape:Qt.PointingHandCursor; onClicked:page.sortMode=modelData }
                }
            }
        }
        Item {
            Layout.fillWidth:true
            Layout.fillHeight:true
            EmptyView {
                anchors.fill:parent
                visible:!thm.available || page.selectedWorkloads().length===0
                symbol:"◈"
                heading:thm.available ? "No matching workloads" : "Waiting for THM telemetry"
                detail:thm.available ? "Try a different search or filter." : "Live Linux processes remain available under Processes even without THM."
            }
            ListView {
                id:list
                anchors.fill:parent
                clip:true
                spacing:10
                visible:thm.available && page.selectedWorkloads().length>0
                model:page.selectedWorkloads()
                delegate:WorkloadCard {
                    required property var modelData
                    width:list.width
                    workload:modelData
                    expanded:page.expandedById[String(modelData.id) + ":" + String(modelData.pid)]===true
                    onToggleRequested:page.toggleWorkload(String(modelData.id) + ":" + String(modelData.pid))
                }
            }
        }
        Text { text:"Root PID lineage is reconstructed from /proc; missing roots show STALE. No process termination is enabled."; color:Theme.textLow; font.pixelSize:11 }
    }
}
