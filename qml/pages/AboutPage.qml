/*
 * AboutPage.qml — what the app is, which hardware it speaks to, the licence.
 *
 * Copyright (C) 2026  JimKnopfIoT — GPLv3 or later.
 */
import QtQuick 2.6
import Sailfish.Silica 1.0

Page {
    id: page
    allowedOrientations: Orientation.All

    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column
            width: parent.width
            spacing: Theme.paddingMedium

            PageHeader { title: qsTr("About IrCam") }

            Image {
                anchors.horizontalCenter: parent.horizontalCenter
                source: "image://theme/harbour-ircam"
                width: Theme.iconSizeLarge
                height: width
                fillMode: Image.PreserveAspectFit
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Live view, temperature measurement and snapshots for "
                           + "the InfiRay P2 Pro thermal camera.")
                wrapMode: Text.Wrap
                color: Theme.highlightColor
            }

            SectionHeader { text: qsTr("Supported hardware") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: qsTr("InfiRay P2 Pro (USB 0bda:5830), a 256×192 USB-C thermal "
                           + "camera. It is a standard UVC device: the kernel "
                           + "driver presents it as /dev/videoN and IrCam reads it "
                           + "over V4L2. The stream is 256×384 — the picture on "
                           + "top, a 16-bit temperature value for every pixel "
                           + "underneath, which is where all readings come from.\n\n"
                           + "Other cameras of the same family that deliver the same "
                           + "stream layout (e.g. the Topdon TC001) should work too, "
                           + "but have not been tested.")
            }

            SectionHeader { text: qsTr("Measuring") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: qsTr("Temperatures are what the sensor reports, corrected "
                           + "only by the offset you set. Readings depend on the "
                           + "emissivity of the surface: matte paint, skin, wood "
                           + "and most plastics read well; bare or shiny metal "
                           + "reflects its surroundings and reads far too low. "
                           + "Use the readings as a guide, not as a calibrated "
                           + "measurement.")
            }

            SectionHeader { text: qsTr("Snapshots") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: qsTr("Snapshots are saved as JPEG in ~/Pictures/IrCam with the "
                           + "markers and colour bar drawn in, at three times the "
                           + "sensor resolution. Optionally the raw temperature grid "
                           + "is written next to each picture as CSV. No location or "
                           + "device information is added to any file.")
            }

            SectionHeader { text: qsTr("Privacy") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: qsTr("Everything runs on the device: no telemetry, no cloud "
                           + "component. Snapshots carry no location or device data "
                           + "unless you switch the location overlay on yourself — "
                           + "then the GPS coordinates you see are drawn into the "
                           + "picture.")
            }

            SectionHeader { text: qsTr("Licence") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeSmall
                color: Theme.secondaryColor
                text: qsTr("GNU General Public License v3.0 or later. "
                           + "Provided as is, with no warranty.")
            }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                wrapMode: Text.Wrap
                font.pixelSize: Theme.fontSizeExtraSmall
                color: Theme.secondaryColor
                text: "https://github.com/JimKnopfIoT/harbour-ircam"
            }
        }

        VerticalScrollDecorator {}
    }
}
