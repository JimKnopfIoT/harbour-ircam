/*
 * GalleryPage.qml — every snapshot, newest first, as a grid of thumbnails.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0

Page {
    id: page

    allowedOrientations: Orientation.All

    /* Refresh on show (the cover action can add files while we sit in the stack).
     * A RemorsePopup triggers its action when destroyed, so cancel any pending
     * delete the moment the page leaves the foreground — a panic swipe off the
     * page must not delete. Only the timer or a left-swipe on the popup may. */
    onStatusChanged: {
        if (status === PageStatus.Activating)
            app.captures.refresh()
        else if (status !== PageStatus.Active)
            remorse.cancel()
    }
    Component.onDestruction: remorse.cancel()
    Connections {
        target: Qt.application
        onStateChanged: if (Qt.application.state !== Qt.ApplicationActive) remorse.cancel()
    }

    /* Page-level, so it survives the delegate it was triggered from. */
    RemorsePopup { id: remorse }

    SilicaGridView {
        id: grid
        anchors.fill: parent
        cellWidth: Math.floor(page.width / (page.isPortrait ? 3 : 5))
        cellHeight: cellWidth

        header: PageHeader {
            title: qsTr("Snapshots")
            description: app.captures.count > 0
                         ? qsTr("%n item(s)", "", app.captures.count) : ""
        }

        model: app.captures

        PullDownMenu {
            MenuItem {
                text: qsTr("Refresh")
                onClicked: app.captures.refresh()
            }
        }

        delegate: BackgroundItem {
            id: cell
            width: grid.cellWidth
            height: grid.cellHeight

            onClicked: pageStack.push(Qt.resolvedUrl("CaptureViewPage.qml"),
                                      { index: model.index })

            onPressAndHold: contextMenu.open(cell)

            ContextMenu {
                id: contextMenu
                MenuItem {
                    text: qsTr("Rename")
                    onClicked: {
                        var dialog = pageStack.push(
                            Qt.resolvedUrl("RenameDialog.qml"),
                            { index: model.index,
                              baseName: app.captures.baseName(model.index),
                              suffix: model.isVideo ? ".webm" : ".jpg" })
                        dialog.accepted.connect(function() {
                            app.captures.rename(dialog.index, dialog.baseName)
                        })
                    }
                }
                MenuItem {
                    text: qsTr("Delete")
                    onClicked: {
                        /* Capture the row NOW: model.index shifts as soon as
                         * anything else is removed, and this runs later. */
                        var row = model.index
                        remorse.execute(qsTr("Deleting"), function() {
                            app.captures.remove(row)
                        })
                    }
                }
            }

            Rectangle {
                anchors.fill: parent
                anchors.margins: 2
                color: Theme.rgba(Theme.highlightBackgroundColor, 0.15)
                clip: true
                // Photos thumbnail directly; WebM can't be decoded by Image, so
                // videos get a dark tile with a play glyph instead.
                Image {
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectCrop
                    asynchronous: true
                    source: model.isVideo ? "" : model.url
                    sourceSize.width: grid.cellWidth
                    visible: !model.isVideo
                }
                Image {
                    anchors.centerIn: parent
                    visible: model.isVideo
                    source: "image://theme/icon-l-play?" + Theme.lightPrimaryColor
                }
            }

            /* VIDEO badge, top-right. */
            Rectangle {
                visible: model.isVideo
                anchors { right: parent.right; top: parent.top; margins: Theme.paddingSmall }
                width: vbadge.width + Theme.paddingSmall
                height: vbadge.height
                radius: 2
                color: Qt.rgba(0, 0, 0, 0.6)
                Label {
                    id: vbadge
                    anchors.centerIn: parent
                    text: qsTr("VIDEO")
                    font.pixelSize: Theme.fontSizeTiny
                    color: Theme.lightPrimaryColor
                }
            }

            /* CSV badge: this one has the raw temperatures next to it. */
            Rectangle {
                visible: model.hasCsv
                anchors {
                    left: parent.left; bottom: parent.bottom
                    margins: Theme.paddingSmall
                }
                width: badge.width + Theme.paddingSmall
                height: badge.height
                radius: 2
                color: Qt.rgba(0, 0, 0, 0.6)
                Label {
                    id: badge
                    anchors.centerIn: parent
                    text: "CSV"
                    font.pixelSize: Theme.fontSizeTiny
                    color: Theme.lightPrimaryColor
                }
            }
        }

        ViewPlaceholder {
            enabled: app.captures.count === 0
            text: qsTr("No snapshots yet")
            hintText: qsTr("Snapshots you take will appear here.")
        }

        VerticalScrollDecorator {}
    }
}
