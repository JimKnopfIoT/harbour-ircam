/*
 * harbour-ircam.qml — application root.
 *
 * Owns the backend objects and hands them to the pages. The camera must be a
 * singleton: it holds the V4L2 node, and a second instance would fail to open
 * it. The measurement points live here too, so the cover's snapshot action
 * burns in exactly what the viewfinder shows.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0
import Nemo.Configuration 1.0
import Nemo.KeepAlive 1.2
import QtPositioning 5.0
import harbour.ircam 1.0

import "pages"
import "cover"

ApplicationWindow {
    id: app

    /* ---- backend --------------------------------------------------------- */
    property alias camera: thermalCamera
    property alias captures: captureStore
    property alias settings: settingsGroup
    /* User measurement points, normalised sensor coordinates {nx, ny}. */
    property alias points: measurePoints

    ThermalCamera {
        id: thermalCamera
        sharpen: settingsGroup.sharpen
        tempOffset: settingsGroup.tempOffset
        palette: settingsGroup.palette
    }

    CaptureStore {
        id: captureStore
    }

    /* Records the composed view (overlay burnt in) to a compressed VP8/WebM
     * file. Frames are pulled from the camera in C++; QML only starts/stops. */
    VideoRecorder {
        id: videoRecorder
        source: thermalCamera
        bitrate: settingsGroup.videoBitrate
        onRecordingFinished: captureStore.refresh()
        onLastErrorChanged: if (lastError !== "") console.warn("ircam video:", lastError)
    }
    property alias recorder: videoRecorder
    /* false = photo mode (white shutter), true = video mode (red record). */
    property bool videoMode: false

    ListModel {
        id: measurePoints
    }

    /* GPS for the optional geodata overlay. Only active while the toggle is on,
     * so nothing powers up the receiver otherwise. */
    PositionSource {
        id: positionSource
        active: settingsGroup.geoFormat > 0
        updateInterval: 2000
    }
    /* Formatted "lat, lon" (decimal degrees) or "" when off / no fix. Shown on
     * the picture and burnt into snapshots. */
    readonly property bool geoOn: settingsGroup.geoFormat > 0
    property string geoText: {
        if (settingsGroup.geoFormat === 0)
            return ""
        if (!positionSource.position.latitudeValid || !positionSource.position.longitudeValid)
            return ""
        var c = positionSource.position.coordinate
        return app.fmtGeo(c.latitude, c.longitude, settingsGroup.geoFormat)
    }

    /* Coordinate formatting, one place used by both the live overlay and the
     * burnt-in snapshot. 1 = decimal degrees, 2 = degrees+decimal minutes
     * (nautical), 3 = degrees/minutes/seconds (navigation). */
    function fmtGeo(lat, lon, mode) {
        function one(v, pos, neg) {
            var h = v >= 0 ? pos : neg
            v = Math.abs(v)
            if (mode === 2) {
                var d = Math.floor(v)
                var m = (v - d) * 60
                return d + "°" + (m < 10 ? "0" : "") + m.toFixed(3) + "'" + h
            }
            if (mode === 3) {
                var d3 = Math.floor(v)
                var mf = (v - d3) * 60
                var m3 = Math.floor(mf)
                var s3 = (mf - m3) * 60
                return d3 + "°" + (m3 < 10 ? "0" : "") + m3 + "'"
                     + (s3 < 10 ? "0" : "") + s3.toFixed(1) + "\"" + h
            }
            return v.toFixed(5) + "°" + h
        }
        return one(lat, "N", "S") + "  " + one(lon, "E", "W")
    }

    function geoFormatName(mode) {
        return mode === 1 ? qsTr("Decimal")
             : mode === 2 ? qsTr("Nautical")
             : mode === 3 ? qsTr("Navigation")
             : qsTr("Off")
    }

    function cycleGeo() {
        settingsGroup.geoFormat = (settingsGroup.geoFormat + 1) % 4
    }

    /* ---- persisted settings (dconf /apps/harbour-ircam) ------------------- */
    ConfigurationGroup {
        id: settingsGroup
        path: "/apps/harbour-ircam"

        property int palette: 0              /* palettes::Lava */
        property real sharpen: 1.2
        property real tempOffset: 0.0
        property bool fahrenheit: false
        property bool showMarkers: true       /* hottest / coldest + tap points */
        property bool showCenter: false       /* centre spot */
        property bool showBar: true           /* colour bar */
        property bool showReadout: true       /* min/max readout on the picture + burnt in */
        property int geoFormat: 0             /* 0 off, 1 decimal, 2 nautical (DDM), 3 navigation (DMS) */
        property int fillMode: 0              /* ThermalView.PreserveAspectFit */
        /* Camera points back at the user ("selfie"). A user-facing camera
         * reverses the sense of rotation, so the fixed-picture trick turns the
         * wrong way in landscape; this corrects only that (NOT a left-right
         * flip). Cannot be auto-detected: UVC/V4L2 does not report the
         * plug/lens direction. */
        property bool facesMe: false
        /* The P2 Pro can be plugged in facing either way; this turns the
         * picture round without the user having to replug. */
        property bool rotate180: false
        property bool keepDisplayOn: true
        property bool showTimestamp: false
        property bool saveCsv: false
        /* VP8 target bitrate in bits/s for video recording. Lower = smaller
         * file, more compression. Driven by the slider in Settings. */
        property int videoBitrate: 1200000
    }

    /* The viewfinder publishes its current picture rotation (orientation
     * plus the 180° setting) so snapshots taken from the cover are upright
     * the same way. */
    property int viewRotation: 90
    property int viewElementRotation: 0

    /* Readings for the labels on screen, refreshed at a few Hz rather than
     * per frame — see MeasurementOverlay for why. */
    property real tMaxShown: 0
    property real tMinShown: 0
    Timer {
        interval: 150
        running: thermalCamera.streaming
        repeat: true
        triggeredOnStart: true
        onTriggered: {
            app.tMaxShown = thermalCamera.tMax
            app.tMinShown = thermalCamera.tMin
        }
    }

    /* One clock for the label on screen and the text burnt into a snapshot. */
    property string currentTimestamp: ""
    Timer {
        interval: 1000
        running: true
        repeat: true
        triggeredOnStart: true
        onTriggered: app.currentTimestamp =
            Qt.formatDateTime(new Date(), "yyyy-MM-dd  hh:mm:ss")
    }

    /* Looking at a thermal picture means not touching the screen. Hold the
     * blanking off while frames are actually arriving. */
    DisplayBlanking {
        preventBlanking: settingsGroup.keepDisplayOn && thermalCamera.streaming
    }

    /* ---- shared helpers -------------------------------------------------- */

    /* Format a °C value in the chosen unit, e.g. "23.4°C" / "74.1°F". */
    function fmtTemp(celsius, decimals) {
        if (celsius === undefined || celsius !== celsius)   /* NaN */
            return "–"
        var v = settingsGroup.fahrenheit ? celsius * 9 / 5 + 32 : celsius
        return v.toFixed(decimals) + "°" + (settingsGroup.fahrenheit ? "F" : "C")
    }

    function pointList() {
        var list = []
        for (var i = 0; i < measurePoints.count; ++i) {
            var p = measurePoints.get(i)
            list.push({ nx: p.nx, ny: p.ny })
        }
        return list
    }

    function takeSnapshot() {
        if (!thermalCamera.streaming)
            return false
        var path = captureStore.saveSnapshot(thermalCamera, {
            rotation:    app.viewRotation,
            showMarkers: settingsGroup.showMarkers,
            showCenter:  settingsGroup.showCenter,
            showBar:     settingsGroup.showBar,
            showReadout: settingsGroup.showReadout,
            geoText:     app.geoText,
            elementRotation: app.viewElementRotation,
            fahrenheit:  settingsGroup.fahrenheit,
            stampText:   settingsGroup.showTimestamp ? app.currentTimestamp : "",
            saveCsv:     settingsGroup.saveCsv,
            points:      app.pointList()
        })
        return path !== ""
    }

    /* Keep the recorder's burnt-in overlay in step with the live view while it
     * records. VideoRecorder exposes only invokable setters (no writable
     * properties), so push changes through them instead of Bindings. */
    onViewElementRotationChanged: if (videoRecorder.recording) videoRecorder.setElementRotation(viewElementRotation)
    onGeoTextChanged: if (videoRecorder.recording) videoRecorder.setGeoText(geoText)
    onCurrentTimestampChanged: if (videoRecorder.recording)
                                   videoRecorder.setStampText(settingsGroup.showTimestamp ? currentTimestamp : "")

    function startVideo() {
        if (!thermalCamera.streaming || videoRecorder.recording)
            return false
        var path = captureStore.newVideoPath()
        if (path === "")
            return false
        return videoRecorder.start(path, {
            rotation:    app.viewRotation,
            showMarkers: settingsGroup.showMarkers,
            showCenter:  settingsGroup.showCenter,
            showBar:     settingsGroup.showBar,
            showReadout: settingsGroup.showReadout,
            geoText:     app.geoText,
            elementRotation: app.viewElementRotation,
            fahrenheit:  settingsGroup.fahrenheit,
            stampText:   settingsGroup.showTimestamp ? app.currentTimestamp : ""
        })
    }
    function stopVideo() {
        if (videoRecorder.recording)
            videoRecorder.stop()
    }

    function cyclePalette() {
        settingsGroup.palette = (settingsGroup.palette + 1) % thermalCamera.paletteNames.length
    }

    // Sharpen presets cycled by the viewfinder button.
    readonly property var sharpenLevels: [0.0, 1.2, 2.5]
    function sharpenName(v) {
        return v < 0.6 ? qsTr("Off") : v < 1.9 ? qsTr("Normal") : qsTr("Superfine")
    }
    function cycleSharpen() {
        var cur = settingsGroup.sharpen, i = 0
        for (var k = 0; k < sharpenLevels.length; ++k)
            if (Math.abs(sharpenLevels[k] - cur) < Math.abs(sharpenLevels[i] - cur)) i = k
        settingsGroup.sharpen = sharpenLevels[(i + 1) % sharpenLevels.length]
    }

    Component.onDestruction: thermalCamera.stop()

    Component.onCompleted: {
        /* Always on: the app is the camera. If no camera is plugged in the
         * backend simply keeps looking (see thermalcamera.h, HOT-PLUG). */
        thermalCamera.start()
    }

    initialPage: Component { ViewfinderPage { } }
    cover: Qt.resolvedUrl("cover/CoverPage.qml")

    /* All orientations: the camera sits in the USB-C port, so the phone is
     * held however the scene demands, and the picture is turned to stay
     * upright (see ViewfinderPage). */
    allowedOrientations: Orientation.Portrait
}
