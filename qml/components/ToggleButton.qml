/*
 * ToggleButton.qml — a round icon button that shows its on/off state.
 *
 * Silica's IconButton has no "checked" look of its own, so this wraps one in
 * a translucent disc that brightens when active. Used for the overlay
 * toggles (markers, centre spot, colour bar) and the palette cycler.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0

MouseArea {
    id: root

    property string iconSource
    property bool active: true
    /* Optional short caption under the icon (e.g. the palette name). */
    property string caption: ""

    width: Theme.itemSizeSmall
    height: width

    Rectangle {
        anchors.fill: parent
        radius: width / 2
        color: root.active ? Theme.rgba(Theme.lightPrimaryColor, root.pressed ? 0.4 : 0.28)
                           : Theme.rgba("black", root.pressed ? 0.6 : 0.4)
        border.width: 1
        border.color: Theme.rgba(Theme.lightPrimaryColor, root.active ? 0.9 : 0.3)
    }

    Image {
        anchors.centerIn: parent
        source: root.iconSource
        width: Theme.iconSizeSmallPlus
        height: width
        sourceSize.width: width
        sourceSize.height: height
        opacity: root.active ? 1.0 : 0.5
    }

    Label {
        visible: root.caption !== ""
        anchors {
            top: parent.bottom
            topMargin: 2
            horizontalCenter: parent.horizontalCenter
        }
        text: root.caption
        font.pixelSize: Theme.fontSizeTiny
        color: Theme.lightPrimaryColor
        style: Text.Outline
        styleColor: Qt.rgba(0, 0, 0, 0.8)
    }
}
