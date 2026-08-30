/*
 * ViewfinderPage.qml — the live thermal picture and the capture controls.
 *
 * ORIENTATION
 * The P2 Pro sits in the USB-C port with its long sensor axis along the
 * phone, so the sensor picture (256 wide, 192 high) is turned by 90° in
 * portrait and shown as is in landscape — and turned again when the phone is
 * held upside down, so the picture is upright in every one of the four
 * orientations. A 180° setting on top of that covers the camera being
 * plugged in the other way round.
 *
 * LAYOUT
 * The picture is the app; everything floats over it. Portrait: a row of
 * toggles above a bottom bar (gallery · shutter · settings). Landscape: the
 * toggles in a column on the right above the shutter, gallery and settings
 * on the left — the thumbs' places. The top row keeps clear of a camera
 * cutout (`topInset`); the picture underneath does not, on purpose.
 *
 * TAP TO MEASURE
 * With markers on, a tap on the picture adds a measurement point (up to six);
 * a tap near an existing one removes it. A drag pans the zoomed picture, a
 * pinch zooms, a double tap cycles 1× → 2× → 4× → 1×.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0
import QtGraphicalEffects 1.0
import QtSensors 5.0
import harbour.ircam 1.0

import "../components"

Page {
    id: page

    // Screen stays portrait; we rotate the picture and the button icons ourselves
    // from the physical device orientation, so controls never change position.
    allowedOrientations: Orientation.Portrait

    property int _orient: OrientationReading.TopUp
    OrientationSensor {
        active: Qt.application.active
        onReadingChanged: {
            if (reading.orientation >= OrientationReading.TopUp
                    && reading.orientation <= OrientationReading.RightUp)
                page._orient = reading.orientation
        }
    }

    // Icon/label rotation so controls stay upright however the phone is held.
    readonly property int controlsRotation: {
        switch (_orient) {
        case OrientationReading.LeftUp:  return 270
        case OrientationReading.TopDown: return 180
        case OrientationReading.RightUp: return 90
        default:                         return 0
        }
    }
    // Sensor is fixed to the phone, so for an outward camera the picture always
    // shows the same way (90°); only icons and overlay rotate. Plus the 180° flip.
    // A user-facing camera ("facesMe") reverses rotation handedness, so the trick
    // turns the wrong way in landscape while portrait stays right; +2×controlsRotation
    // corrects it (+0° portrait, +180° landscape; sign-independent mod 360). Kept a
    // pure rotation, not a flip: a horizontal flip in the 90° view reads as vertical
    // and stands portrait on its head.
    readonly property int pictureRotation: (90
        + (app.settings.rotate180 ? 180 : 0)
        + (app.settings.facesMe ? 2 * controlsRotation : 0)) % 360
    onPictureRotationChanged: app.viewRotation = pictureRotation
    onControlsRotationChanged: app.viewElementRotation = controlsRotation
    Component.onCompleted: { app.viewRotation = pictureRotation; app.viewElementRotation = controlsRotation }

    // Portrait-locked, so this is always false: existing bindings then use the
    // fixed portrait layout.
    readonly property bool landscape: false
    readonly property real barSize: Theme.itemSizeLarge + 2 * Theme.paddingLarge

    // CAMERA CUTOUT
    // The picture may run under the notch — it is a picture, and cropping it
    // would cost more than the corner it hides. The controls may not: on a
    // phone with a wide cutout (Jolla J2) the top row of round buttons and the
    // status line were cut through by the notch edge. Silica gives its own
    // header exactly this margin (`_minimumTopMargin` in PageHeader.qml); this
    // page draws its own controls over a full-screen picture, so it has to.
    //
    // No orientation condition here, unlike Silica: the page is locked to
    // portrait and only the icons rotate, so the cutout is at the page's top
    // however the phone is held. On a phone without one the rectangle is empty
    // and this is zero, so it costs the other devices nothing.
    readonly property real topInset: Screen.topCutout.height

    /* ------------------------------------------------------------------ */
    /* Live picture                                                        */
    /* ------------------------------------------------------------------ */
    Rectangle {
        anchors.fill: parent
        color: "black"
    }

    ThermalView {
        id: view
        anchors.fill: parent
        camera: app.camera
        fillMode: app.settings.fillMode
        frameRotation: page.pictureRotation
    }

    MeasurementOverlay {
        id: overlay
        anchors.fill: view
        visible: view.hasFrame
        view: view
        camera: app.camera
        showMarkers: app.settings.showMarkers
        showCenter: app.settings.showCenter
        showBar: app.settings.showBar
        showReadout: app.settings.showReadout
        geoText: app.geoText
        bottomReserved: app.settings.showTimestamp
        fahrenheit: app.settings.fahrenheit
        points: app.points
        barColors: app.camera.paletteStops(9)
        elementRotation: page.controlsRotation
        /* Keep the colour bar and labels clear of the control bars that float
         * over the picture: the top status strip, and the button bar (bottom
         * in portrait, left+right columns in landscape). */
        safeTop: page.topInset + Theme.paddingLarge + Theme.itemSizeSmall
        safeBottom: page.barSize
        safeLeft: 0
        safeRight: 0
        Connections {
            target: app.camera
            onPaletteChanged: overlay.barColors = app.camera.paletteStops(9)
        }
    }

    /* ------------------------------------------------------------------ */
    /* Gestures: pinch zoom, drag pan, tap to measure, double tap zoom     */
    /* ------------------------------------------------------------------ */
    PinchArea {
        id: pinchArea
        anchors.fill: parent
        /* Leave the control bars to the buttons. */
        anchors.bottomMargin: page.landscape ? 0 : page.barSize
        anchors.rightMargin: page.landscape ? page.barSize : 0
        anchors.leftMargin: page.landscape ? page.barSize : 0
        pinch.minimumScale: 1.0
        pinch.maximumScale: view.maxZoom

        property real startZoom: 1.0
        onPinchStarted: startZoom = view.zoom
        onPinchUpdated: view.zoom = startZoom * pinch.scale

        MouseArea {
            anchors.fill: parent

            property real lastX: 0
            property real lastY: 0
            property bool dragging: false

            onPressed: {
                lastX = mouse.x
                lastY = mouse.y
                dragging = false
            }
            onPositionChanged: {
                if (!view.canPan())
                    return
                var dx = mouse.x - lastX
                var dy = mouse.y - lastY
                if (!dragging && Math.abs(dx) + Math.abs(dy) < 8)
                    return
                dragging = true
                view.panBy(dx, dy)
                lastX = mouse.x
                lastY = mouse.y
            }
            onClicked: {
                if (dragging || !app.settings.showMarkers || !app.camera.tempValid)
                    return
                /* Our coordinates are relative to this MouseArea, which is
                 * inset from the view by the bar margins. */
                var vx = mouse.x + pinchArea.x
                var vy = mouse.y + pinchArea.y
                var n = view.toFramePoint(vx, vy)
                if (n.x < 0 || n.x > 1 || n.y < 0 || n.y > 1)
                    return
                var best = -1, bd = 1e9
                for (var i = 0; i < app.points.count; ++i) {
                    var p = app.points.get(i)
                    var d = (p.nx - n.x) * (p.nx - n.x) + (p.ny - n.y) * (p.ny - n.y)
                    if (d < bd) { bd = d; best = i }
                }
                /* Within ~5 % of an existing point: remove it, else add. */
                if (best >= 0 && bd < 0.0025)
                    app.points.remove(best)
                else if (app.points.count < 6)
                    app.points.append({ nx: n.x, ny: n.y })
                overlay.requestPaint()
            }
            onDoubleClicked: {
                if (view.zoom >= view.maxZoom) {
                    view.zoom = 1.0
                    view.resetPan()
                } else if (view.zoom >= 2.0) {
                    view.zoom = view.maxZoom
                } else {
                    view.zoom = 2.0
                }
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /* Placeholder while there is no picture                                */
    /* ------------------------------------------------------------------ */
    StatusOverlay {
        anchors.centerIn: pinchArea
        width: pinchArea.width - 2 * Theme.paddingLarge
        camera: app.camera
        visible: !view.hasFrame || !app.camera.streaming
    }

    /* Burnt-in timestamp preview, bottom-left of the picture. */
    Label {
        visible: app.settings.showTimestamp && view.hasFrame
        anchors {
            left: parent.left
            bottom: parent.bottom
            leftMargin: (page.landscape ? page.barSize : 0) + Theme.paddingMedium
            bottomMargin: (page.landscape ? 0 : page.barSize) + Theme.paddingMedium
        }
        text: app.currentTimestamp
        font.pixelSize: Theme.fontSizeSmall
        font.family: Theme.fontFamilyHeading
        color: "#FFD25A"
        style: Text.Outline
        styleColor: Qt.rgba(0, 0, 0, 0.85)
    }

    /* Palette name, shown briefly after cycling. */
    Rectangle {
        id: paletteToast
        anchors.centerIn: pinchArea
        width: toastLabel.width + 2 * Theme.paddingLarge
        height: toastLabel.height + Theme.paddingMedium
        radius: height / 2
        color: Qt.rgba(0, 0, 0, 0.65)
        opacity: 0
        visible: opacity > 0
        function show(text) { toastLabel.text = text; toastAnim.restart() }
        Label {
            id: toastLabel
            anchors.centerIn: parent
            font.pixelSize: Theme.fontSizeMedium
            color: Theme.lightPrimaryColor
        }
        SequentialAnimation {
            id: toastAnim
            NumberAnimation { target: paletteToast; property: "opacity"; to: 1; duration: 100 }
            PauseAnimation { duration: 900 }
            NumberAnimation { target: paletteToast; property: "opacity"; to: 0; duration: 300 }
        }
    }

    /* ------------------------------------------------------------------ */
    /* Controls. x/y positioned only (conditional anchors stretch a fixed    */
    /* control full-width on rotation). Top-left cluster = settings, sharpen, */
    /* gallery (a row in portrait, a bottom-up column in landscape) with the  */
    /* live status beside it; the capture strip (toggles + shutter) stays at  */
    /* the bottom in both orientations.                                       */
    /* ------------------------------------------------------------------ */
    Item {
        id: controls
        anchors.fill: parent

        readonly property real pad: Theme.paddingLarge
        readonly property real sp: Theme.paddingMedium
        readonly property real ssz: Theme.itemSizeSmall
        // Same margin, plus whatever the camera cutout takes off the top.
        readonly property real padTop: pad + page.topInset

        // fades for legibility
        Rectangle {
            visible: !page.landscape
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: page.barSize
            gradient: Gradient {
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.7) }
            }
        }
        Rectangle {
            anchors { left: parent.left; right: parent.right; top: parent.top }
            height: controls.padTop + controls.ssz
            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(0, 0, 0, 0.5) }
                GradientStop { position: 1.0; color: "transparent" }
            }
        }

        // top-left cluster: settings, sharpen, gallery (all round, same size)
        ToggleButton {
            id: settingsButton
            rotation: page.controlsRotation
            width: controls.ssz; height: width
            x: controls.pad
            y: page.landscape ? (page.height - height - controls.pad) : controls.padTop
            iconSource: "image://theme/icon-m-developer-mode"
            active: true
            onClicked: pageStack.push(Qt.resolvedUrl("SettingsPage.qml"))
        }
        ToggleButton {
            id: sharpenButton
            rotation: page.controlsRotation
            width: controls.ssz; height: width
            x: page.landscape ? controls.pad : (settingsButton.x + width + controls.sp)
            y: page.landscape ? (settingsButton.y - height - controls.sp) : controls.padTop
            iconSource: Qt.resolvedUrl("../images/icon-sharpen.png")
            active: app.settings.sharpen > 0.6
            onClicked: {
                app.cycleSharpen()
                paletteToast.show(qsTr("Sharpness") + ": " + app.sharpenName(app.settings.sharpen))
            }
        }
        MouseArea {
            id: galleryButton
            rotation: page.controlsRotation
            width: controls.ssz; height: width
            // Outermost of the cluster: the colourful thumbnail sits on the edge.
            x: page.landscape ? controls.pad : (modeButton.x + width + controls.sp)
            y: page.landscape ? (modeButton.y - height - controls.sp) : controls.padTop
            onClicked: pageStack.push(Qt.resolvedUrl("GalleryPage.qml"))

            Rectangle {
                id: galleryDisc
                anchors.fill: parent
                radius: width / 2
                color: Theme.rgba("black", 0.4)
                border.width: 1
                border.color: Theme.rgba(Theme.lightPrimaryColor, 0.4)
                Image {
                    id: lastThumb
                    anchors.fill: parent
                    anchors.margins: 2
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    sourceSize.width: controls.ssz
                    source: app.captures.lastCapturePath !== "" ? "file://" + app.captures.lastCapturePath : ""
                    visible: false
                }
                OpacityMask {
                    anchors.fill: lastThumb
                    source: lastThumb
                    visible: lastThumb.status === Image.Ready
                    maskSource: Rectangle {
                        width: lastThumb.width; height: lastThumb.height
                        radius: width / 2; visible: false
                    }
                }
                Image {
                    anchors.centerIn: parent
                    visible: lastThumb.status !== Image.Ready
                    source: "image://theme/icon-m-image"
                    opacity: app.captures.count > 0 ? 0.9 : 0.35
                }
            }
            Rectangle {
                visible: app.captures.count > 0
                anchors { right: parent.right; top: parent.top }
                width: countLabel.width + Theme.paddingSmall
                height: countLabel.height
                radius: height / 2
                color: Theme.highlightBackgroundColor
                Label {
                    id: countLabel
                    anchors.centerIn: parent
                    text: app.captures.count
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.primaryColor
                }
            }
        }

        // photo/video mode toggle, next to the gallery. Locked while recording
        // (you can't switch a clip to stills half-way through).
        ToggleButton {
            id: modeButton
            rotation: page.controlsRotation
            width: controls.ssz; height: width
            // Sits between sharpen and the gallery (gallery is the outer one).
            x: page.landscape ? controls.pad : (sharpenButton.x + width + controls.sp)
            y: page.landscape ? (sharpenButton.y - height - controls.sp) : controls.padTop
            iconSource: Qt.resolvedUrl("../images/icon-photovideo.png")
            active: app.videoMode
            enabled: !app.recorder.recording
            opacity: enabled ? 1.0 : 0.4
            onClicked: {
                app.videoMode = !app.videoMode
                paletteToast.show(app.videoMode ? qsTr("Video") : qsTr("Photo"))
            }
        }

        // live status: portrait beside the buttons; landscape top-left of the
        // black bar, on the same top line.
        // Live status, right-aligned to the standard margin, top-right in both
        // orientations so it stays in the same region on rotation.
        Row {
            id: liveRow
            rotation: page.controlsRotation
            spacing: Theme.paddingSmall
            // portrait: right-aligned to the margin; landscape: top-left corner.
            x: page.landscape ? controls.pad : (page.width - controls.pad - width)
            y: page.landscape ? controls.padTop : (controls.padTop + (controls.ssz - height) / 2)
            Rectangle {
                width: Theme.paddingSmall; height: width; radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                color: app.camera.streaming ? "#5FD35F"
                     : app.camera.status === ThermalCamera.Error ? Theme.errorColor : Theme.highlightColor
                SequentialAnimation on opacity {
                    running: !app.camera.streaming && app.camera.running
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.25; duration: 600 }
                    NumberAnimation { to: 1.0;  duration: 600 }
                }
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: app.camera.statusText
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.lightPrimaryColor
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                visible: app.camera.streaming
                text: app.camera.fps.toFixed(0) + " fps"
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.lightSecondaryColor
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                visible: view.zoom > 1.01
                text: view.zoom.toFixed(1) + "×"
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.lightSecondaryColor
            }
        }

        // bottom capture strip: toggles row + shutter, same place both orientations
        Grid {
            id: toggles
            columns: 6
            spacing: Theme.paddingLarge
            x: (page.width - width) / 2
            y: shutter.y - height - controls.pad
            ToggleButton {
                rotation: page.controlsRotation
                iconSource: "image://theme/icon-m-location"
                active: app.settings.showMarkers
                onClicked: app.settings.showMarkers = !app.settings.showMarkers
            }
            ToggleButton {
                rotation: page.controlsRotation
                iconSource: "image://theme/icon-m-dot"
                active: app.settings.showCenter
                onClicked: app.settings.showCenter = !app.settings.showCenter
            }
            ToggleButton {
                rotation: page.controlsRotation
                iconSource: "image://theme/icon-m-levels"
                active: app.settings.showBar
                onClicked: app.settings.showBar = !app.settings.showBar
            }
            ToggleButton {
                rotation: page.controlsRotation
                iconSource: "image://theme/icon-m-annotation"
                active: app.settings.showReadout
                onClicked: app.settings.showReadout = !app.settings.showReadout
            }
            ToggleButton {
                rotation: page.controlsRotation
                iconSource: "image://theme/icon-m-gps"
                active: app.geoOn
                onClicked: {
                    app.cycleGeo()
                    paletteToast.show(qsTr("Location") + ": " + app.geoFormatName(app.settings.geoFormat))
                }
            }
            ToggleButton {
                rotation: page.controlsRotation
                iconSource: "image://theme/icon-m-ambience"
                active: true
                onClicked: {
                    app.cyclePalette()
                    paletteToast.show(app.camera.paletteNames[app.settings.palette])
                }
            }
        }

        ShutterButton {
            id: shutter
            rotation: page.controlsRotation
            x: (page.width - width) / 2
            y: page.height - height - controls.pad
            enabled: app.camera.streaming
            videoMode: app.videoMode
            recording: app.recorder.recording
            onClicked: {
                if (app.videoMode) {
                    if (app.recorder.recording)
                        app.stopVideo()
                    else
                        app.startVideo()
                } else if (app.takeSnapshot()) {
                    flash.flash()
                }
            }
        }

        // Recording indicator: blinking red dot + elapsed time, top-centre.
        Row {
            id: recIndicator
            rotation: page.controlsRotation
            visible: app.recorder.recording
            spacing: Theme.paddingSmall
            x: (page.width - width) / 2
            y: controls.padTop + (controls.ssz - height) / 2
            Rectangle {
                width: Theme.paddingMedium; height: width; radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                color: "#FF3B30"
                SequentialAnimation on opacity {
                    running: app.recorder.recording
                    loops: Animation.Infinite
                    NumberAnimation { to: 0.2; duration: 550 }
                    NumberAnimation { to: 1.0; duration: 550 }
                }
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                function fmt(ms) {
                    var s = Math.floor(ms / 1000)
                    var m = Math.floor(s / 60)
                    var ss = s % 60
                    return m + ":" + (ss < 10 ? "0" : "") + ss
                }
                text: "REC " + fmt(app.recorder.durationMs)
                font.pixelSize: Theme.fontSizeSmall
                font.family: Theme.fontFamilyHeading
                color: Theme.lightPrimaryColor
                style: Text.Outline
                styleColor: Qt.rgba(0, 0, 0, 0.85)
            }
        }
    }

    /* ------------------------------------------------------------------ */
    /* Shutter flash + error feedback                                      */
    /* ------------------------------------------------------------------ */
    Rectangle {
        id: flash
        anchors.fill: parent
        color: "white"
        opacity: 0
        function flash() { flashAnim.restart() }
        SequentialAnimation {
            id: flashAnim
            NumberAnimation { target: flash; property: "opacity"; to: 0.75; duration: 60 }
            NumberAnimation { target: flash; property: "opacity"; to: 0.0;  duration: 220 }
        }
    }

    Connections {
        target: app.captures
        onError: errorBanner.show(message)
    }

    Rectangle {
        id: errorBanner
        anchors { bottom: parent.bottom; left: parent.left; right: parent.right }
        height: errorLabel.height + 2 * Theme.paddingMedium
        color: Theme.rgba(Theme.errorColor, 0.9)
        opacity: 0
        visible: opacity > 0
        function show(message) {
            errorLabel.text = message
            errorAnim.restart()
        }
        Label {
            id: errorLabel
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left; right: parent.right
                leftMargin: Theme.horizontalPageMargin
                rightMargin: Theme.horizontalPageMargin
            }
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeSmall
            color: Theme.lightPrimaryColor
        }
        SequentialAnimation {
            id: errorAnim
            NumberAnimation { target: errorBanner; property: "opacity"; to: 1.0; duration: 150 }
            PauseAnimation { duration: 4000 }
            NumberAnimation { target: errorBanner; property: "opacity"; to: 0.0; duration: 400 }
        }
    }
}
