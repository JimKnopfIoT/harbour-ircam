/*
 * CoverPage.qml — the tile shown while the app runs in the background.
 *
 * While the camera streams, the cover shows the live picture itself: a
 * thermal image is recognisable at any size, and "is it still running, what
 * is the hottest thing in view" is exactly what you want to know at a glance.
 * Without a picture it falls back to the icon crop. One action: snapshot.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0
import harbour.ircam 1.0

CoverBackground {
    id: cover

    Image {
        anchors.fill: parent
        source: Qt.resolvedUrl("../images/cover-logo.png")
        fillMode: Image.PreserveAspectCrop
        opacity: liveView.visible ? 0 : 0.38
        asynchronous: true
    }

    ThermalView {
        id: liveView
        anchors.fill: parent
        /* Only attached while the app is in the background — that is the only
         * time the cover is on screen, and uploading a texture per frame for
         * a tile nobody sees would just burn battery. */
        camera: Qt.application.active ? null : app.camera
        /* The cover is always portrait. */
        frameRotation: (90 + (app.settings.rotate180 ? 180 : 0)) % 360
        fillMode: ThermalView.PreserveAspectCrop
        visible: hasFrame && app.camera.streaming && !Qt.application.active
    }

    Rectangle {
        anchors.fill: parent
        gradient: Gradient {
            GradientStop { position: 0.0; color: "transparent" }
            GradientStop { position: 0.55; color: Qt.rgba(0, 0, 0, 0.35) }
            GradientStop { position: 1.0; color: Qt.rgba(0, 0, 0, 0.7) }
        }
    }

    Column {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom
            bottomMargin: Theme.itemSizeSmall + Theme.paddingLarge
            leftMargin: Theme.paddingMedium
            rightMargin: Theme.paddingMedium
        }
        spacing: Theme.paddingSmall

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: "IrCam"
            font.pixelSize: Theme.fontSizeLarge
            color: Theme.lightPrimaryColor
        }

        Label {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            visible: app.camera.streaming && app.camera.tempValid
            font.pixelSize: Theme.fontSizeSmall
            font.bold: true
            color: Theme.lightPrimaryColor
            text: "▲ " + app.fmtTemp(app.tMaxShown, 1) + "   ▼ " + app.fmtTemp(app.tMinShown, 1)
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Theme.paddingSmall
            Rectangle {
                width: Theme.paddingSmall
                height: width
                radius: width / 2
                anchors.verticalCenter: parent.verticalCenter
                color: app.camera.streaming ? "#5FD35F" : Theme.lightSecondaryColor
            }
            Label {
                anchors.verticalCenter: parent.verticalCenter
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.lightSecondaryColor
                text: app.camera.statusText
            }
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-cover-camera"
            onTriggered: app.takeSnapshot()
        }
    }
}
