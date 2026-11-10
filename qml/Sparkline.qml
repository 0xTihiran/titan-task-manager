import QtQuick
import "."
Canvas {
    id: root
    property var values: []
    property color lineColor: Theme.accent
    property color fillColor: Theme.chartFill
    antialiasing: true
    onValuesChanged: requestPaint()
    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0,0,width,height)
        var data=values || []
        if (data.length < 2 || width < 10 || height < 10) return
        var w=width, h=height, padding=3
        function px(i) { return padding+(w-2*padding)*i/(data.length-1) }
        function py(v) { return padding+(h-2*padding)*(1-Math.max(0,Math.min(100,v))/100) }
        ctx.beginPath()
        ctx.moveTo(px(0),py(data[0]))
        for (var i=1;i<data.length;i++) ctx.lineTo(px(i),py(data[i]))
        ctx.lineTo(px(data.length-1),h)
        ctx.lineTo(px(0),h)
        ctx.closePath()
        ctx.fillStyle=fillColor
        ctx.fill()
        ctx.beginPath()
        ctx.moveTo(px(0),py(data[0]))
        for (var j=1;j<data.length;j++) ctx.lineTo(px(j),py(data[j]))
        ctx.lineWidth=2.2
        ctx.strokeStyle=lineColor
        ctx.lineJoin="round"
        ctx.lineCap="round"
        ctx.stroke()
    }
}
