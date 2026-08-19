// CamerasPage.qml — the camera database, attached right of Settings. GPLv3+, © JimKnopfIoT.
import QtQuick 2.6
import Sailfish.Silica 1.0

import "../components"

Page {
    allowedOrientations: Orientation.All
    CameraDbView { anchors.fill: parent }
}
