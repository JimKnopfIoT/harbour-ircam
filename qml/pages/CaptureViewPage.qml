/*
 * CaptureViewPage.qml — one capture full screen; swipe the centre of the
 * picture left/right to step through the others. The edges are left free so
 * the page's own gestures still work: pull down from the top for
 * delete/rename/share, swipe in from the left edge to go back.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0
import Sailfish.Share 1.0
import QtMultimedia 5.6

Page {
    id: page

    property int index: -1

    allowedOrientations: Orientation.All

    readonly property var item: index >= 0 && index < app.captures.count
                                ? app.captures.itemAt(index) : ({})
    readonly property bool isVideo: item.isVideo === true

    function step(delta) {
        var n = index + delta
        if (n >= 0 && n < app.captures.count)
            index = n
    }

    /* A RemorsePopup triggers on destroy, so cancel a pending delete on anything
     * but the timer or a left-swipe: leaving the page, stepping, backgrounding. */
    onStatusChanged: if (status !== PageStatus.Active) remorse.cancel()
    onIndexChanged: remorse.cancel()
    Component.onDestruction: remorse.cancel()
    Connections {
        target: Qt.application
        onStateChanged: if (Qt.application.state !== Qt.ApplicationActive) remorse.cancel()
    }

    /* Page-level so it exists before onIndexChanged can fire during creation
     * and survives whichever child triggered the delete. */
    RemorsePopup { id: remorse }

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: height

        PullDownMenu {
            MenuItem {
                text: qsTr("Delete")
                onClicked: {
                    var row = page.index
                    remorse.execute(qsTr("Deleting"), function() {
                        app.captures.remove(row)
                        if (app.captures.count === 0)
                            pageStack.pop()
                        else if (page.index >= app.captures.count)
                            page.index = app.captures.count - 1
                    })
                }
            }
            MenuItem {
                text: qsTr("Rename")
                onClicked: {
                    var dialog = pageStack.push(
                        Qt.resolvedUrl("RenameDialog.qml"),
                        { index: page.index,
                          baseName: app.captures.baseName(page.index),
                          suffix: page.item.isVideo ? ".webm" : ".jpg" })
                    dialog.accepted.connect(function() {
                        app.captures.rename(dialog.index, dialog.baseName)
                    })
                }
            }
            MenuItem {
                text: qsTr("Share")
                onClicked: shareAction.trigger()
            }
        }

        Image {
            id: photo
            visible: !page.isVideo
            anchors.fill: parent
            anchors.bottomMargin: caption.height
            fillMode: Image.PreserveAspectFit
            asynchronous: true
            source: (!page.isVideo && page.item.url) ? page.item.url : ""
            smooth: true
        }

        /* Video playback. autoPlay starts as soon as the media loads; the tap
         * toggles play/pause. Errors are logged so a codec/sink problem shows up
         * in the journal instead of just a black frame. */
        Video {
            id: player
            visible: page.isVideo
            anchors.fill: parent
            anchors.bottomMargin: caption.height
            fillMode: VideoOutput.PreserveAspectFit
            autoPlay: true
            source: page.isVideo && page.item.url ? page.item.url : ""
            onErrorChanged: if (error !== MediaPlayer.NoError)
                                console.warn("ircam: video error", error, errorString)
            onStatusChanged: {
                // Rewind and pause at the end so a tap replays from the start.
                if (status === MediaPlayer.EndOfMedia) { seek(0); pause() }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (player.playbackState === MediaPlayer.PlayingState)
                        player.pause()
                    else {
                        if (player.status === MediaPlayer.EndOfMedia)
                            player.seek(0)
                        player.play()
                    }
                }
            }
            // Big play glyph while paused/stopped.
            Rectangle {
                anchors.centerIn: parent
                width: Theme.itemSizeLarge; height: width; radius: width / 2
                color: Qt.rgba(0, 0, 0, 0.5)
                visible: player.playbackState !== MediaPlayer.PlayingState
                Image {
                    anchors.centerIn: parent
                    source: "image://theme/icon-l-play?" + Theme.lightPrimaryColor
                }
            }
        }
        Component.onDestruction: player.stop()

        /* Swipe only the centre of the picture: leaves the left edge for the
         * back gesture and the top for the pulley. */
        MouseArea {
            id: swipe
            anchors.centerIn: parent
            /* Keep the gesture from the enclosing SilicaFlickable so a
             * horizontal drag actually reaches us instead of being swallowed
             * as a flick. The pulley still works from the top strip above. */
            preventStealing: true
            width: Math.min(photo.paintedWidth, parent.width) * 0.7
            height: Math.min(photo.paintedHeight, parent.height) * 0.7
            property real startX: 0
            property real startY: 0
            enabled: app.captures.count > 1 && !page.isVideo
            onPressed: { startX = mouse.x; startY = mouse.y }
            onReleased: {
                var dx = mouse.x - startX
                var dy = mouse.y - startY
                if (Math.abs(dx) > Theme.itemSizeSmall && Math.abs(dx) > Math.abs(dy) * 1.5)
                    page.step(dx < 0 ? 1 : -1)
            }
        }

        Rectangle {
            id: caption
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom }
            height: capCol.height + 2 * Theme.paddingMedium
            color: Qt.rgba(0, 0, 0, 0.55)
            Column {
                id: capCol
                anchors {
                    verticalCenter: parent.verticalCenter
                    left: parent.left; right: parent.right
                    leftMargin: Theme.horizontalPageMargin
                    rightMargin: Theme.horizontalPageMargin
                }
                Label {
                    width: parent.width
                    text: (page.item.fileName ? page.item.fileName : "")
                          + (app.captures.count > 1
                             ? "   (" + (page.index + 1) + "/" + app.captures.count + ")" : "")
                    truncationMode: TruncationMode.Fade
                    color: Theme.lightPrimaryColor
                    font.pixelSize: Theme.fontSizeSmall
                }
                Label {
                    text: (page.item.sizeText ? page.item.sizeText : "")
                          + (page.item.hasCsv ? "  ·  " + qsTr("temperature grid saved as CSV") : "")
                    color: Theme.lightSecondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                }
            }
        }
    }

    ShareAction {
        id: shareAction
        resources: page.item.path ? [ page.item.path ] : []
        mimeType: page.item.isVideo ? "video/webm" : "image/jpeg"
    }
}
