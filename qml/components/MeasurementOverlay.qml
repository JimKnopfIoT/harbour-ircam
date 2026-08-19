/*
 * MeasurementOverlay.qml — hottest/coldest spot, centre spot, the user's tap
 * points and the colour bar, drawn over the live picture.
 *
 * It is a Canvas sized like the ThermalView underneath and asks the view
 * where each sensor position is on screen (toItemPoint), so markers follow
 * the picture through rotation, zoom and pan without any geometry of their
 * own. Text is always upright: the canvas does not rotate with the picture.
 *
 * The snapshot draws the same things in C++ (snapshotcomposer.cpp) — keep the
 * colours and conventions in step when changing either.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0

Canvas {
    id: overlay

    /* The ThermalView this overlay sits on. */
    property var view
    property var camera
    property bool showMarkers: true
    property bool showCenter: false
    property bool showBar: true
    property bool showReadout: true
    property bool fahrenheit: false
    /* Location line drawn bottom-left of the picture (empty = off). */
    property string geoText: ""
    /* Leave room for the timestamp label below the location line. */
    property bool bottomReserved: false
    // Rotate the drawn elements (labels, bar, markers) by this angle so they
    // stay upright for the viewer while the picture is fixed. Marker crosses are
    // inverse-mapped so they remain on their sensor spots.
    property real elementRotation: 0
    onElementRotationChanged: requestPaint()
    /* ListModel of {nx, ny}. */
    property var points
    /* 0..1 of the palette, top to bottom = hot to cold, as an array of colour
     * strings for the bar gradient. */
    property var barColors: []
    /* Keep the colour bar and any labels clear of the control bars, which sit
     * over the picture. Insets in pixels from each edge of this item. */
    property real safeTop: 0
    property real safeBottom: 0
    property real safeLeft: 0
    property real safeRight: 0

    antialiasing: true
    renderStrategy: Canvas.Cooperative

    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onVisibleChanged: requestPaint()
    onShowMarkersChanged: requestPaint()
    onShowCenterChanged: requestPaint()
    onShowBarChanged: requestPaint()
    onShowReadoutChanged: requestPaint()
    onGeoTextChanged: requestPaint()
    onFahrenheitChanged: requestPaint()
    onBarColorsChanged: requestPaint()

    /* The camera updates 25 times a second; repainting markers and labels at
     * that rate is wasted work on the GUI thread. Collect and repaint at
     * ~8 Hz instead — plenty for numbers you read with your eyes. */
    property bool _dirty: false
    Connections {
        target: overlay.camera
        onTempUpdated: overlay._dirty = true
    }
    Timer {
        interval: 120
        repeat: true
        running: overlay.visible
        onTriggered: if (overlay._dirty) { overlay._dirty = false; overlay.requestPaint() }
    }
    Connections {
        target: overlay.view
        onFrameRectChanged: if (overlay.visible) overlay.requestPaint()
        onMirroredChanged: if (overlay.visible) overlay.requestPaint()
    }
    Connections {
        target: overlay.points
        onCountChanged: overlay.requestPaint()
    }

    function fmt(c, decimals) {
        if (c !== c) return "–"
        var v = fahrenheit ? c * 9 / 5 + 32 : c
        return v.toFixed(decimals) + "°" + (fahrenheit ? "F" : "C")
    }

    function outlinedText(ctx, text, x, y) {
        ctx.lineWidth = 3
        ctx.strokeStyle = "rgba(0,0,0,0.8)"
        ctx.strokeText(text, x, y)
        ctx.fillText(text, x, y)
    }

    /* Clamp a label's baseline y so it stays inside the safe area. */
    function clampY(y, fs) {
        var top = safeTop + fs
        var bot = height - safeBottom - 2
        return Math.max(top, Math.min(bot, y))
    }

    function cross(ctx, p, color, label, fs) {
        var r = Math.round(Math.max(8, Math.min(width, height) * 0.022))
        ctx.strokeStyle = color
        ctx.lineWidth = 2.5
        ctx.lineCap = "round"
        ctx.beginPath()
        ctx.moveTo(p.x - r, p.y); ctx.lineTo(p.x + r, p.y)
        ctx.moveTo(p.x, p.y - r); ctx.lineTo(p.x, p.y + r)
        ctx.stroke()
        ctx.fillStyle = color
        ctx.textAlign = "left"
        var tw = ctx.measureText(label).width
        var tx = p.x + r + 4
        /* Flip the label to the other side if it would leave the safe area. */
        if (tx + tw > width - safeRight) { ctx.textAlign = "right"; tx = p.x - r - 4 }
        outlinedText(ctx, label, tx, clampY(p.y - r, fs))
        ctx.textAlign = "left"
    }

    // Marker cross in the rotated element frame; the label is kept inside the
    // picture bounds (picL..picR, picT..picB), like the burnt-in snapshot.
    function crossL(ctx, p, color, label, fs, picL, picR, picT, picB) {
        var r = Math.round(Math.max(8, Math.min(width, height) * 0.022))
        ctx.strokeStyle = color
        ctx.lineWidth = 2.5
        ctx.lineCap = "round"
        ctx.beginPath()
        ctx.moveTo(p.x - r, p.y); ctx.lineTo(p.x + r, p.y)
        ctx.moveTo(p.x, p.y - r); ctx.lineTo(p.x, p.y + r)
        ctx.stroke()
        ctx.fillStyle = color
        ctx.textAlign = "left"
        var tw = ctx.measureText(label).width
        var tx = p.x + r + 4
        if (tx + tw > picR - 2) tx = p.x - r - 4 - tw
        tx = Math.max(picL + 2, Math.min(tx, picR - tw - 2))
        var ty = Math.max(picT + fs, Math.min(p.y - r, picB - 2))
        outlinedText(ctx, label, tx, ty)
        ctx.textAlign = "left"
    }

    onPaint: {
        var ctx = getContext("2d")
        ctx.clearRect(0, 0, width, height)
        if (!camera || !view || !camera.tempValid)
            return

        var er = ((elementRotation % 360) + 360) % 360
        var swap = (er === 90 || er === 270)
        var uw = swap ? height : width        // user-view width
        var uh = swap ? width : height        // user-view height
        var rad = er * Math.PI / 180
        var cw = width / 2, ch = height / 2

        // native screen point -> current (rotated, centre-origin) frame
        function toLocal(p) {
            var dx = p.x - cw, dy = p.y - ch
            var a = -rad, c = Math.cos(a), s = Math.sin(a)
            return Qt.point(dx * c - dy * s, dx * s + dy * c)
        }

        ctx.save()
        ctx.translate(cw, ch)
        ctx.rotate(rad)

        ctx.textBaseline = "top"
        var fs = Math.round(Math.max(14, Math.min(width, height) * 0.036))
        ctx.font = "bold " + fs + "px sans-serif"
        var m = Math.round(Math.min(uw, uh) * 0.03)

        // Picture rectangle mapped into this rotated, centre-origin frame, so
        // the elements hug the picture (not the black screen edges).
        var fr = view.frameRect
        var q1 = toLocal(Qt.point(fr.x, fr.y)),               q2 = toLocal(Qt.point(fr.x + fr.width, fr.y))
        var q3 = toLocal(Qt.point(fr.x, fr.y + fr.height)),   q4 = toLocal(Qt.point(fr.x + fr.width, fr.y + fr.height))
        var picL = Math.min(q1.x, q2.x, q3.x, q4.x), picR = Math.max(q1.x, q2.x, q3.x, q4.x)
        var picT = Math.min(q1.y, q2.y, q3.y, q4.y), picB = Math.max(q1.y, q2.y, q3.y, q4.y)
        // The picture is letterboxed clear of the buttons, so the elements hug
        // its edges directly.
        var elemT = picT
        var elemB = picB

        if (showReadout) {
            ctx.textAlign = "left"
            var rx = picL + m
            var ry = elemT + m
            var hot = "\u25B2 " + fmt(camera.tMax, 1)
            ctx.fillStyle = "#ff6b60"
            outlinedText(ctx, hot, rx, ry)
            var cold = "\u25BC " + fmt(camera.tMin, 1)
            ctx.fillStyle = "#7fd4ff"
            outlinedText(ctx, cold, rx + ctx.measureText(hot).width + m * 2, ry)
        }

        if (showBar && barColors.length > 1) {
            var bw = Math.round(Math.max(10, Math.min(uw, uh) * 0.03))
            var by = elemT + fs
            var bh = elemB - elemT - 2 * fs
            var bx = picR - bw - m
          if (bh >= Theme.itemSizeSmall) {
            var g = ctx.createLinearGradient(0, by, 0, by + bh)
            for (var i = 0; i < barColors.length; ++i)
                g.addColorStop(i / (barColors.length - 1), barColors[i])
            ctx.fillStyle = g
            ctx.fillRect(bx, by, bw, bh)
            ctx.strokeStyle = "rgba(255,255,255,0.6)"
            ctx.lineWidth = 1
            ctx.strokeRect(bx, by, bw, bh)
            ctx.fillStyle = "#ffffff"
            ctx.textAlign = "right"
            var lx = bx - 5
            outlinedText(ctx, fmt(camera.tWinHi, 0), lx, by - 2)
            outlinedText(ctx, fmt((camera.tWinHi + camera.tWinLo) / 2, 0), lx, by + bh / 2 - fs / 2)
            outlinedText(ctx, fmt(camera.tWinLo, 0), lx, by + bh - fs)
            ctx.textAlign = "left"
          }
        }

        if (showMarkers) {
            crossL(ctx, toLocal(view.toItemPoint(camera.tMaxX, camera.tMaxY)), "#ff3b30", fmt(camera.tMax, 1), fs, picL, picR, picT, picB)
            crossL(ctx, toLocal(view.toItemPoint(camera.tMinX, camera.tMinY)), "#5ac8fa", fmt(camera.tMin, 1), fs, picL, picR, picT, picB)
            if (points) {
                for (var k = 0; k < points.count; ++k) {
                    var pt = points.get(k)
                    crossL(ctx, toLocal(view.toItemPoint(pt.nx, pt.ny)), "#5aff5a",
                           (k + 1) + ": " + fmt(camera.tempAt(pt.nx, pt.ny), 1), fs, picL, picR, picT, picB)
                }
            }
        }

        if (geoText !== "") {
            var gfs = Math.round(Math.max(13, Math.min(uw, uh) * 0.032))
            ctx.font = gfs + "px sans-serif"
            ctx.textAlign = "left"
            ctx.fillStyle = "#ffffff"
            ctx.textBaseline = "bottom"
            outlinedText(ctx, geoText, picL + m, elemB - m)
            ctx.textBaseline = "top"
            ctx.font = "bold " + fs + "px sans-serif"
        }

        if (showCenter) {
            var c = toLocal(view.toItemPoint(0.5, 0.5))
            var cr = Math.round(Math.max(10, Math.min(width, height) * 0.03))
            ctx.strokeStyle = "#ffffff"
            ctx.lineWidth = 2.5
            ctx.beginPath()
            ctx.arc(c.x, c.y, cr, 0, 2 * Math.PI)
            ctx.moveTo(c.x - cr * 1.6, c.y); ctx.lineTo(c.x - cr * 0.6, c.y)
            ctx.moveTo(c.x + cr * 0.6, c.y); ctx.lineTo(c.x + cr * 1.6, c.y)
            ctx.moveTo(c.x, c.y - cr * 1.6); ctx.lineTo(c.x, c.y - cr * 0.6)
            ctx.moveTo(c.x, c.y + cr * 0.6); ctx.lineTo(c.x, c.y + cr * 1.6)
            ctx.stroke()
            ctx.fillStyle = "#ffffff"
            outlinedText(ctx, fmt(camera.tCenter, 1), c.x + cr * 1.8, c.y - cr * 1.2)
        }

        ctx.restore()
    }
}
