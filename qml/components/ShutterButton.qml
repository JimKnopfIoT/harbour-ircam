/*
 * ShutterButton.qml — the snapshot control. The classic two-ring shutter:
 * recognisable without a label in any language.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0

MouseArea {
    id: root

    width: Theme.itemSizeLarge
    height: width

    property bool pressedDown: pressed && containsMouse
    /* Photo mode: white disc. Video mode: red disc, and while recording the
     * disc becomes a rounded square (the universal "stop" glyph). */
    property bool videoMode: false
    property bool recording: false

    readonly property color _fill: videoMode ? "#FF3B30"
                                             : Theme.rgba(Theme.lightPrimaryColor, 0.95)

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: "transparent"
        border.width: 4
        border.color: root.enabled ? Theme.rgba(Theme.lightPrimaryColor, 0.95)
                                   : Theme.rgba(Theme.lightSecondaryColor, 0.3)
    }

    Rectangle {
        id: inner
        anchors.centerIn: parent
        /* Fixed fraction of the button. The press feedback is done with `scale`,
         * NOT by animating this width: a `Behavior on width` would also animate
         * the width change that happens when the button is re-laid out on an
         * orientation change, ballooning the disc across the whole screen for a
         * moment. Scale only ever changes on press, so re-layout is instant. */
        width: parent.width * (root.recording ? 0.5 : 0.78)
        height: width
        radius: root.recording ? width * 0.22 : width / 2
        color: root.enabled ? root._fill
                            : Theme.rgba(Theme.lightSecondaryColor, 0.3)
        scale: root.pressedDown ? 0.85 : 1.0
        Behavior on scale { NumberAnimation { duration: 90 } }
        Behavior on radius { NumberAnimation { duration: 140 } }
        /* Softly pulse while recording so it's obvious the clip is running. */
        SequentialAnimation on opacity {
            running: root.recording
            alwaysRunToEnd: false
            loops: Animation.Infinite
            NumberAnimation { to: 0.55; duration: 650 }
            NumberAnimation { to: 1.0;  duration: 650 }
        }
        Connections {
            target: root
            onRecordingChanged: if (!root.recording) inner.opacity = 1.0
        }
    }
}
