import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "."
import "components"
Item {
    id:page
    property string searchText:""
    property string sortMode:"CPU"
    property string filterMode:"All"
    property int selectedPid:-1
    property var lastReview: ({})
    function doReview(pid, force) {
        // Explicitly a PREVIEW. C++ always refuses to signal in v0.6.
        page.lastReview = actions.reviewTermination(pid, force)
        reviewResultDialog.open()
    }

    function filteredProcesses() {
        var items=workloadTree.processes.slice()
        var q=searchText.trim().toLowerCase()
        if(q.length>0) items=items.filter(function(p) {
            return String(p.name).toLowerCase().indexOf(q)>=0 || String(p.pid).indexOf(q)>=0 ||
                   String(p.workloadName || "").toLowerCase().indexOf(q)>=0
        })
        if(filterMode==="Managed") items=items.filter(function(p){return Number(p.workloadId)!==0})
        if(filterMode==="Unmatched") items=items.filter(function(p){return Number(p.workloadId)===0})
        if(filterMode==="Protected") items=items.filter(function(p){return p.isProtected === true})
        if(sortMode==="Memory") items.sort(function(a,b){return Number(b.memoryMiB)-Number(a.memoryMiB)})
        else if(sortMode==="PID") items.sort(function(a,b){return Number(a.pid)-Number(b.pid)})
        else if(sortMode==="Name") items.sort(function(a,b){return String(a.name).localeCompare(String(b.name))})
        else items.sort(function(a,b){return Number(b.cpu)-Number(a.cpu)})
        return items
    }
    function selectedProcess() {
        var items=workloadTree.processes
        for(var i=0;i<items.length;i++) if(Number(items[i].pid)===selectedPid) return items[i]
        return null
    }
    ColumnLayout {
        anchors.fill:parent
        spacing:13
        RowLayout {
            Layout.fillWidth:true
            ColumnLayout {
                spacing:4
                Text { text:"Processes"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:23; font.weight:Font.Bold }
                Text { text:"Live Linux processes · THM ownership and action safety preview"; color:Theme.textSecondary; font.pixelSize:12 }
            }
            Item { Layout.fillWidth:true }
            StatusChip { label:String(proc.count)+" running"; tone:Theme.accent }
            StatusChip { label:String(workloadTree.matchedCount)+" linked"; tone:Theme.success }
        }
        RowLayout {
            spacing:7
            Repeater {
                model:["All","Managed","Unmatched","Protected"]
                delegate:Rectangle {
                    required property string modelData
                    width:filterText.implicitWidth+22; height:32; radius:9
                    color:page.filterMode===modelData ? Theme.accentSoft : Theme.surface
                    border.color:page.filterMode===modelData ? Theme.accent : Theme.border
                    Text { id:filterText; anchors.centerIn:parent; text:modelData; color:Theme.textSecondary; font.pixelSize:11 }
                    MouseArea { anchors.fill:parent; cursorShape:Qt.PointingHandCursor; onClicked:page.filterMode=modelData }
                }
            }
            Item { Layout.fillWidth:true }
            Text { text:"SORT"; color:Theme.textLow; font.pixelSize:10 }
            Repeater {
                model:["CPU","Memory","Name","PID"]
                delegate:Rectangle {
                    required property string modelData
                    width:sortText.implicitWidth+19; height:32; radius:9
                    color:page.sortMode===modelData ? Theme.accentSoft : Theme.surface
                    border.color:page.sortMode===modelData ? Theme.accent : Theme.border
                    Text { id:sortText; anchors.centerIn:parent; text:modelData; color:Theme.textSecondary; font.pixelSize:11 }
                    MouseArea { anchors.fill:parent; cursorShape:Qt.PointingHandCursor; onClicked:page.sortMode=modelData }
                }
            }
            Text { text:"  READ ONLY"; color:Theme.warning; font.pixelSize:11; font.weight:Font.DemiBold }
        }
        Panel {
            Layout.fillWidth:true
            Layout.fillHeight:true
            ColumnLayout {
                anchors.fill:parent
                spacing:0
                Rectangle {
                    Layout.fillWidth:true
                    Layout.preferredHeight:41
                    color:Theme.surfaceRaised; radius:Theme.radius
                    RowLayout {
                        anchors.fill:parent; anchors.leftMargin:15; anchors.rightMargin:15
                        Text { text:"PROCESS"; Layout.fillWidth:true; color:Theme.textLow; font.pixelSize:10 }
                        Text { text:"PID"; Layout.preferredWidth:63; color:Theme.textLow; font.pixelSize:10 }
                        Text { text:"CPU %"; Layout.preferredWidth:65; color:Theme.textLow; font.pixelSize:10 }
                        Text { text:"RAM MiB"; Layout.preferredWidth:74; color:Theme.textLow; font.pixelSize:10 }
                        Text { text:"WORKLOAD"; Layout.preferredWidth:148; color:Theme.textLow; font.pixelSize:10 }
                        Text { text:"STATE"; Layout.preferredWidth:43; color:Theme.textLow; font.pixelSize:10 }
                        Text { text:"ACCESS"; Layout.preferredWidth:85; color:Theme.textLow; font.pixelSize:10 }
                    }
                }
                Item {
                    Layout.fillWidth:true; Layout.fillHeight:true
                    EmptyView { anchors.fill:parent; visible:page.filteredProcesses().length===0; symbol:"≡"; heading:"No matching processes"; detail:"Try another search or filter." }
                    ListView {
                        id:list; anchors.fill:parent; clip:true
                        visible:page.filteredProcesses().length>0
                        model:page.filteredProcesses()
                        delegate:Rectangle {
                            id:processRow
                            required property int index
                            required property var modelData
                            width:list.width; height:43
                            color:Number(modelData.pid)===page.selectedPid ? Theme.accentSoft : (index%2===0 ? Theme.surface : Theme.sidebar)
                            RowLayout {
                                anchors.fill:parent; anchors.leftMargin:15; anchors.rightMargin:15
                                RowLayout {
                                    Layout.fillWidth:true; spacing:7
                                    Rectangle { width:7; height:7; radius:4; color:processRow.modelData.workloadId ? Theme.success : Theme.warning }
                                    Text { text:processRow.modelData.name || "(unknown)"; Layout.fillWidth:true; elide:Text.ElideRight; color:Theme.textPrimary; font.pixelSize:12 }
                                }
                                Text { text:String(processRow.modelData.pid); Layout.preferredWidth:63; color:Theme.textSecondary; font.pixelSize:12 }
                                Text { text:Number(processRow.modelData.cpu).toFixed(1); Layout.preferredWidth:65; color:Theme.textPrimary; font.pixelSize:12 }
                                Text { text:Number(processRow.modelData.memoryMiB).toFixed(1); Layout.preferredWidth:74; color:Theme.textSecondary; font.pixelSize:12 }
                                Text { text:processRow.modelData.workloadName || processRow.modelData.assignment; Layout.preferredWidth:148; elide:Text.ElideRight; color:Theme.textSecondary; font.pixelSize:11 }
                                Text { text:processRow.modelData.state || "?"; Layout.preferredWidth:43; color:Theme.textLow; font.pixelSize:12 }
                                Text { text:processRow.modelData.isProtected ? "LOCKED" : "VERIFY"; Layout.preferredWidth:85; color:processRow.modelData.isProtected ? Theme.danger : Theme.warning; font.pixelSize:10; font.weight:Font.DemiBold }
                            }
                            MouseArea { anchors.fill:parent; cursorShape:Qt.PointingHandCursor; onClicked:page.selectedPid=Number(processRow.modelData.pid) }
                        }
                    }
                }
            }
        }
        Panel {
            id:inspector
            Layout.fillWidth:true
            Layout.preferredHeight:page.selectedPid>0 ? 285 : 45
            ColumnLayout {
                anchors.fill:parent
                anchors.margins:12
                spacing:5
                RowLayout {
                    Text { text:"PROCESS INSPECTOR"; color:Theme.textLow; font.pixelSize:10; font.weight:Font.DemiBold }
                    Item { Layout.fillWidth:true }
                    Text { text:page.selectedPid>0 ? "PID " + page.selectedPid : "Select a row to inspect"; color:Theme.textSecondary; font.pixelSize:11 }
                    Text { text:page.selectedPid>0 ? "✕" : ""; color:Theme.textSecondary; font.pixelSize:15
                           MouseArea { anchors.fill:parent; enabled:page.selectedPid>0; cursorShape:Qt.PointingHandCursor; onClicked:page.selectedPid=-1 } }
                }
                ColumnLayout {
                    visible:page.selectedPid>0
                    Layout.fillWidth:true
                    spacing:4
                    Text { text:"Name: " + (page.selectedProcess() ? page.selectedProcess().name : "Process exited"); color:Theme.textPrimary; font.pixelSize:12 }
                    Text { text:"Parent PID: " + (page.selectedProcess() ? page.selectedProcess().ppid : "—") + "     UID: " + (page.selectedProcess() ? page.selectedProcess().uid : "—"); color:Theme.textSecondary; font.pixelSize:11 }
                    Text { text:"Executable: " + (page.selectedProcess() ? (page.selectedProcess().exe || "unavailable") : "—"); color:Theme.textSecondary; font.pixelSize:11; elide:Text.ElideMiddle; Layout.fillWidth:true }
                    Text { text:"CWD: " + (page.selectedProcess() ? (page.selectedProcess().cwd || "unavailable") : "—"); color:Theme.textSecondary; font.pixelSize:11; elide:Text.ElideMiddle; Layout.fillWidth:true }
                    Text { text:"Command: " + (page.selectedProcess() ? (page.selectedProcess().cmdline || "unavailable") : "—"); color:Theme.textLow; font.pixelSize:11; elide:Text.ElideRight; Layout.fillWidth:true }
                    Text { text:"Cgroup: " + (page.selectedProcess() ? (page.selectedProcess().cgroup || "unavailable") : "—"); color:Theme.textLow; font.pixelSize:11; elide:Text.ElideRight; Layout.fillWidth:true }
                    Text {
                        text:"Protection: " + (page.selectedProcess() ? page.selectedProcess().protectionStatus : "PROCESS EXITED")
                        color:page.selectedProcess() && page.selectedProcess().isProtected ? Theme.danger : Theme.warning
                        font.pixelSize:11; font.weight:Font.DemiBold
                    }
                    Text {
                        text:page.selectedProcess() ? page.selectedProcess().terminateReason : "Process no longer exists in current snapshot"
                        color:Theme.textSecondary; font.pixelSize:11; Layout.fillWidth:true
                        elide:Text.ElideRight
                    }
                    RowLayout {
                        spacing: 9
                        ActionButton {
                            label: "Review Terminate"; symbol: "!"
                            onClicked: {
                                if (page.selectedPid > 1) {
                                    confirmRequest.pid = page.selectedPid
                                    confirmRequest.force = false
                                    confirmRequest.open()
                                }
                            }
                        }
                        ActionButton {
                            label: "Review Force Kill"; symbol: "!!"
                            onClicked: {
                                if (page.selectedPid > 1) {
                                    confirmRequest.pid = page.selectedPid
                                    confirmRequest.force = true
                                    confirmRequest.open()
                                }
                            }
                        }
                        Text { text: "PREVIEW ONLY — no signal sent"; color: Theme.warning; font.pixelSize: 11 }
                    }
                }
            }
        }
        Text { text:"v0.6 safety preview: review requests are logged and ALWAYS denied; actual termination is not enabled."; color:Theme.textLow; font.pixelSize:11 }
    }

    // These dialogs exercise the future two-stage confirmation UX, but the
    // backend contains zero process-signalling APIs and refuses every request.
    Dialog {
        id: confirmRequest
        property int pid: -1
        property bool force: false
        width: 470
        x: Math.max(0, (page.width - width) / 2)
        y: Math.max(0, (page.height - height) / 2)
        modal: true
        focus: true
        title: "Review process action"
        standardButtons: Dialog.NoButton
        background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radius; border.color: Theme.border }
        contentItem: ColumnLayout {
            spacing: 14
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                text: "Review " + (confirmRequest.force ? "SIGKILL" : "SIGTERM") + " request for PID " + confirmRequest.pid + "?"
                color: Theme.textPrimary; font.pixelSize: 15; font.weight: Font.DemiBold
            }
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                text: "This is a read-only preview. No signal will be sent. The requested action and denial reason will be audited."
                color: Theme.textSecondary; font.pixelSize: 12
            }
            RowLayout {
                Item { Layout.fillWidth: true }
                ActionButton { label: "Cancel"; symbol: "×"; onClicked: confirmRequest.close() }
                ActionButton {
                    label: confirmRequest.force ? "Continue review" : "Review & log"
                    symbol: "→"; highlighted: true
                    onClicked: {
                        if (confirmRequest.force) {
                            confirmRequest.close()
                            doubleConfirm.pid = confirmRequest.pid
                            doubleConfirm.open()
                        } else {
                            var requestPid = confirmRequest.pid
                            confirmRequest.close()
                            page.doReview(requestPid, false)
                        }
                    }
                }
            }
        }
    }
    Dialog {
        id: doubleConfirm
        property int pid: -1
        width: 470
        x: Math.max(0, (page.width - width) / 2)
        y: Math.max(0, (page.height - height) / 2)
        modal: true
        focus: true
        title: "Second confirmation — force action review"
        standardButtons: Dialog.NoButton
        background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radius; border.color: Theme.border }
        contentItem: ColumnLayout {
            spacing: 14
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                text: "Review a force-kill request for PID " + doubleConfirm.pid + "? This still sends NO signal."
                color: Theme.warning; font.pixelSize: 14
            }
            RowLayout {
                Item { Layout.fillWidth: true }
                ActionButton { label: "Cancel"; symbol: "×"; onClicked: doubleConfirm.close() }
                ActionButton {
                    label: "Review & log"; symbol: "→"; highlighted: true
                    onClicked: {
                        var requestPid = doubleConfirm.pid
                        doubleConfirm.close()
                        page.doReview(requestPid, true)
                    }
                }
            }
        }
    }
    Dialog {
        id: reviewResultDialog
        width: 480
        x: Math.max(0, (page.width - width) / 2)
        y: Math.max(0, (page.height - height) / 2)
        modal: true
        title: "Process action denied (preview)"
        standardButtons: Dialog.NoButton
        background: Rectangle { color: Theme.surfaceRaised; radius: Theme.radius; border.color: Theme.border }
        contentItem: ColumnLayout {
            spacing: 12
            Text { text: "NO SIGNAL SENT"; color: Theme.warning; font.pixelSize: 15; font.weight: Font.Bold }
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                text: String(page.lastReview.code || "BLOCKED") + ": " + String(page.lastReview.reason || "Denied")
                color: Theme.textPrimary; font.pixelSize: 13
            }
            Text {
                Layout.fillWidth: true; wrapMode: Text.WordWrap
                text: page.lastReview.logged ? "Review written to " + page.lastReview.auditPath :
                     "Warning: Could not write the audit log."
                color: Theme.textSecondary; font.pixelSize: 11
            }
            RowLayout {
                Item { Layout.fillWidth: true }
                ActionButton { label: "Close"; symbol: "✓"; highlighted: true; onClicked: reviewResultDialog.close() }
            }
        }
    }
}
