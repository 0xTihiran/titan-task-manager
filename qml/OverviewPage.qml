import QtQuick
import QtQuick.Layouts
import "."
import "components"
Item {
    id: page
    function topWorkloads() {
        var all=thm.workloads.slice()
        all.sort(function(a,b){return Number(b.cpu_pct||0)-Number(a.cpu_pct||0)})
        return all.slice(0,5)
    }
    Flickable {
        id: scroll
        anchors.fill:parent
        clip:true
        contentWidth:width
        contentHeight:body.implicitHeight+24
        boundsBehavior:Flickable.StopAtBounds
        ColumnLayout {
            id:body
            width:scroll.width-4
            spacing:17
            RowLayout {
                Layout.fillWidth:true
                ColumnLayout {
                    spacing:4
                    Text { text:"System overview"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:23; font.weight:Font.Bold }
                    Text { text:"Live Linux metrics and THM workload intelligence"; color:Theme.textSecondary; font.family:Theme.font; font.pixelSize:12 }
                }
                Item { Layout.fillWidth:true }
                StatusChip { label:thm.available ? "THM online" : "Linux-only mode"; tone:thm.available ? Theme.success : Theme.warning }
            }
            GridLayout {
                Layout.fillWidth:true
                columns:4
                rowSpacing:12
                columnSpacing:12
                MetricTile {
                    Layout.fillWidth:true
                    caption:"CPU UTILIZATION"
                    value:sys.available ? sys.cpuPercent.toFixed(1)+"%" : "—"
                    footnote:"Aggregate CPU · live"
                    symbol:"◴"
                    progress:sys.cpuPercent
                    tone:Theme.accent
                }
                MetricTile {
                    Layout.fillWidth:true
                    caption:"MEMORY USAGE"
                    value:sys.memoryPercent.toFixed(1)+"%"
                    footnote:sys.memoryUsedGiB.toFixed(1)+" / "+sys.memoryTotalGiB.toFixed(1)+" GiB"
                    symbol:"▤"
                    progress:sys.memoryPercent
                    tone:Theme.success
                }
                MetricTile {
                    Layout.fillWidth:true
                    caption:"ACTIVE WORKLOADS"
                    value:thm.available ? String(thm.activeWorkloads) : "—"
                    footnote:thm.available ? String(thm.workloadCount)+" tracked workloads" : "THM telemetry unavailable"
                    symbol:"◈"
                    progress:thm.workloadCount>0 ? 100*thm.activeWorkloads/thm.workloadCount : 0
                    tone:Theme.warning
                }
                MetricTile {
                    Layout.fillWidth:true
                    caption:"PROCESSES"
                    value:String(proc.count)
                    footnote:"Live Linux /proc snapshot"
                    symbol:"≡"
                    progress:Math.min(100,proc.count/4)
                    tone:Theme.accent
                }
            }
            RowLayout {
                Layout.fillWidth:true
                spacing:14
                Panel {
                    Layout.fillWidth:true
                    Layout.preferredHeight:263
                    ColumnLayout {
                        anchors.fill:parent
                        anchors.margins:19
                        spacing:11
                        RowLayout {
                            Text { text:"CPU activity"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:16; font.weight:Font.DemiBold }
                            Item { Layout.fillWidth:true }
                            Text { text:"LAST 48 SAMPLES"; color:Theme.textLow; font.family:Theme.font; font.pixelSize:10; font.letterSpacing:1 }
                        }
                        RowLayout {
                            Text { text:sys.cpuPercent.toFixed(1)+"%"; font.family:Theme.font; font.pixelSize:27; font.weight:Font.Bold; color:Theme.textPrimary }
                            Text { text:"CPU load"; font.family:Theme.font; font.pixelSize:12; color:Theme.textSecondary }
                            Item { Layout.fillWidth:true }
                            StatusChip { label:"LIVE"; tone:Theme.success }
                        }
                        Sparkline { Layout.fillWidth:true; Layout.fillHeight:true; values:sys.cpuHistory; lineColor:Theme.accent; fillColor:Theme.chartFill }
                        RowLayout {
                            Text { text:"0%"; color:Theme.textLow; font.pixelSize:10 }
                            Item { Layout.fillWidth:true }
                            Text { text:"1 second /proc/stat polling"; color:Theme.textLow; font.pixelSize:10 }
                            Item { Layout.fillWidth:true }
                            Text { text:"100%"; color:Theme.textLow; font.pixelSize:10 }
                        }
                    }
                }
                Panel {
                    Layout.fillWidth:true
                    Layout.preferredHeight:263
                    ColumnLayout {
                        anchors.fill:parent
                        anchors.margins:19
                        spacing:13
                        RowLayout {
                            Text { text:"Memory activity"; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:16; font.weight:Font.DemiBold }
                            Item { Layout.fillWidth:true }
                            StatusChip { label:"RAM"; tone:Theme.success }
                        }
                        RowLayout {
                            Text { text:sys.memoryPercent.toFixed(1)+"%"; font.family:Theme.font; font.pixelSize:27; font.weight:Font.Bold; color:Theme.textPrimary }
                            Text { text:"in use"; font.family:Theme.font; font.pixelSize:12; color:Theme.textSecondary }
                            Item { Layout.fillWidth:true }
                        }
                        Sparkline { Layout.fillWidth:true; Layout.fillHeight:true; values:sys.memoryHistory; lineColor:Theme.success; fillColor:Theme.successSoft }
                        RowLayout {
                            Text { text:sys.memoryUsedGiB.toFixed(2)+" GiB used"; color:Theme.textLow; font.pixelSize:11 }
                            Item { Layout.fillWidth:true }
                            Text { text:sys.memoryTotalGiB.toFixed(2)+" GiB total"; color:Theme.textLow; font.pixelSize:11 }
                        }
                    }
                }
            }
            Panel {
                Layout.fillWidth:true
                Layout.preferredHeight:112
                RowLayout {
                    anchors.fill:parent
                    anchors.margins:19
                    spacing:20
                    ColumnLayout {
                        spacing:6
                        Text { text:"SYSTEM PRESSURE"; color:Theme.textLow; font.family:Theme.font; font.pixelSize:11; font.letterSpacing:1 }
                        StatusChip { label:thm.available ? thm.pressureLevel : "THM offline"; tone:!thm.available ? Theme.warning : (thm.pressureLevel==="NORMAL" ? Theme.success : (thm.pressureLevel==="MODERATE" ? Theme.warning : Theme.danger)) }
                    }
                    Rectangle { width:1; Layout.fillHeight:true; color:Theme.border }
                    Repeater {
                        model:[
                            { label:"CPU PSI (10s)", value:sys.cpuPsi.toFixed(2)+"%" },
                            { label:"MEMORY PSI (10s)", value:sys.memoryPsi.toFixed(2)+"%" },
                            { label:"I/O PSI (10s)", value:sys.ioPsi.toFixed(2)+"%" },
                            { label:"LOAD AVG (1m)", value:sys.loadAverage.toFixed(2) }
                        ]
                        delegate:ColumnLayout {
                            required property var modelData
                            Layout.fillWidth:true
                            spacing:7
                            Text { text:modelData.label; font.family:Theme.font; font.pixelSize:10; color:Theme.textLow }
                            Text { text:modelData.value; font.family:Theme.font; font.pixelSize:19; font.weight:Font.DemiBold; color:Theme.textPrimary }
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth:true
                spacing:14
                Panel {
                    Layout.fillWidth:true
                    Layout.preferredHeight:270
                    ColumnLayout {
                        anchors.fill:parent
                        anchors.margins:18
                        spacing:12
                        SectionTitle { Layout.fillWidth:true; title:"Top workloads"; hint:"Sorted by CPU" }
                        Item {
                            Layout.fillWidth:true
                            Layout.fillHeight:true
                            EmptyView { anchors.fill:parent; visible:!thm.available || thm.workloads.length===0; symbol:"◈"; heading:"No THM workloads"; detail:"Workload data appears when the THM daemon publishes telemetry." }
                            ColumnLayout {
                                anchors.fill:parent
                                visible:thm.available && thm.workloads.length>0
                                spacing:7
                                Repeater {
                                    model:page.topWorkloads()
                                    delegate: RowLayout {
                                        required property var modelData
                                        Layout.fillWidth:true
                                        spacing:8
                                        Rectangle { width:8; height:8; radius:4; color:modelData.state==="ACTIVE" ? Theme.success : Theme.warning }
                                        Text { text:modelData.name || "Unnamed workload"; Layout.fillWidth:true; elide:Text.ElideRight; color:Theme.textPrimary; font.family:Theme.font; font.pixelSize:12 }
                                        Text { text:Number(modelData.cpu_pct||0).toFixed(1)+"%"; color:Theme.textSecondary; font.family:Theme.font; font.pixelSize:12; Layout.preferredWidth:55; horizontalAlignment:Text.AlignRight }
                                        Text { text:String(modelData.mem_mb||0)+" MiB"; color:Theme.textLow; font.family:Theme.font; font.pixelSize:11; Layout.preferredWidth:72; horizontalAlignment:Text.AlignRight }
                                    }
                                }
                                Item { Layout.fillHeight:true }
                            }
                        }
                    }
                }
                Panel {
                    Layout.fillWidth:true
                    Layout.preferredHeight:270
                    ColumnLayout {
                        anchors.fill:parent
                        anchors.margins:18
                        spacing:15
                        SectionTitle { Layout.fillWidth:true; title:"THM activity"; hint:"Latest published decision" }
                        StatusChip { label:thm.available ? "Connected to telemetry" : "Unavailable"; tone:thm.available ? Theme.success : Theme.warning }
                        Text {
                            text:thm.lastDecision.length>0 ? thm.lastDecision : "No recent orchestrator action reported."
                            font.family:Theme.font
                            font.pixelSize:14
                            color:Theme.textPrimary
                            wrapMode:Text.WordWrap
                            Layout.fillWidth:true
                        }
                        Text { text: thm.available ? ("Governor: "+(thm.governor || "unknown")) : "Linux metrics remain available without THM."; font.family:Theme.font; font.pixelSize:12; color:Theme.textLow; wrapMode:Text.WordWrap; Layout.fillWidth:true }
                        Item { Layout.fillHeight:true }
                        Text { text:"Read-only monitoring · no cgroup writes"; font.family:Theme.font; font.pixelSize:11; color:Theme.textLow }
                    }
                }
            }
        }
    }
}
