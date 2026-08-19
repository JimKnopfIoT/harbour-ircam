// SettingsPage.qml — adjustable options; pull down for About, swipe left for
// the camera database. GPLv3+, © JimKnopfIoT.
import QtQuick 2.6
import Sailfish.Silica 1.0

import "../components"

Page {
    id: page
    allowedOrientations: Orientation.All

    // Swipe left from here to reach the camera database; swipe right goes back
    // to the camera. Attach once the page is actually active.
    onStatusChanged: {
        if (status === PageStatus.Active && pageStack.nextPage(page) === null)
            pageStack.pushAttached(Qt.resolvedUrl("CamerasPage.qml"))
    }

    SilicaFlickable {
            anchors.fill: parent
            contentHeight: column.height + Theme.paddingLarge

            PullDownMenu {
                MenuItem {
                    text: qsTr("About IrCam")
                    onClicked: pageStack.push(Qt.resolvedUrl("AboutPage.qml"))
                }
            }

            Column {
                id: column
                width: parent.width

                PageHeader { title: qsTr("Settings") }

                SectionHeader { text: qsTr("Camera") }

                TextSwitch {
                    text: qsTr("Camera on")
                    description: app.camera.statusText
                         + (app.camera.statusDetail !== "" ? " — " + app.camera.statusDetail : "")
                         + (app.camera.devicePath !== "" ? "  (" + app.camera.devicePath + ")" : "")
                    checked: app.camera.running
                    onClicked: app.camera.running ? app.camera.stop() : app.camera.start()
                }

                TextSwitch {
                    text: qsTr("Turn picture by 180°")
                    description: qsTr("The camera can be plugged in facing either way. "
                                      + "If the picture is upside down, switch this on "
                                      + "instead of replugging.")
                    checked: app.settings.rotate180
                    onClicked: app.settings.rotate180 = !app.settings.rotate180
                }

                TextSwitch {
                    text: qsTr("Camera points at me")
                    description: qsTr("Turn this on when the lens faces you instead of "
                                      + "away. A user-facing camera turns the wrong way "
                                      + "in landscape otherwise. This only fixes the "
                                      + "rotation — use \"Mirror picture\" as well if you "
                                      + "also want a left-right flip. (The plug direction "
                                      + "cannot be detected automatically.)")
                    checked: app.settings.facesMe
                    onClicked: app.settings.facesMe = !app.settings.facesMe
                }

                ComboBox {
                    label: qsTr("Scaling")
                    description: qsTr("How the 4:3 thermal picture fills the screen.")
                    currentIndex: app.settings.fillMode
                    menu: ContextMenu {
                        MenuItem { text: qsTr("Fit — show the whole picture") }
                        MenuItem { text: qsTr("Fill — crop the edges") }
                    }
                    onCurrentIndexChanged: app.settings.fillMode = currentIndex
                }

                SectionHeader { text: qsTr("Picture") }

                ComboBox {
                    label: qsTr("Palette")
                    currentIndex: app.settings.palette
                    menu: ContextMenu {
                        Repeater {
                            model: app.camera.paletteNames
                            MenuItem { text: modelData }
                        }
                    }
                    onCurrentIndexChanged: app.settings.palette = currentIndex
                }

                Slider {
                    width: parent.width
                    label: qsTr("Sharpening")
                    minimumValue: 0
                    maximumValue: 3
                    stepSize: 0.1
                    value: app.settings.sharpen
                    valueText: value.toFixed(1)
                    onValueChanged: app.settings.sharpen = value
                }

                SectionHeader { text: qsTr("Measurement") }

                TextSwitch {
                    text: qsTr("Fahrenheit")
                    description: qsTr("Show and save temperatures in °F instead of °C.")
                    checked: app.settings.fahrenheit
                    onClicked: app.settings.fahrenheit = !app.settings.fahrenheit
                }

                Slider {
                    width: parent.width
                    label: qsTr("Temperature offset")
                    minimumValue: -10
                    maximumValue: 10
                    stepSize: 0.1
                    value: app.settings.tempOffset
                    valueText: (value >= 0 ? "+" : "") + value.toFixed(1) + " °C"
                    onValueChanged: app.settings.tempOffset = value
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: qsTr("Added to every reading. The sensor is not calibrated in "
                               + "absolute terms; point it at something of known "
                               + "temperature and adjust until the reading matches. "
                               + "Readings also depend on the emissivity of the "
                               + "surface — shiny metal reads far too low.")
                }

                TextSwitch {
                    text: qsTr("Hottest and coldest spot")
                    description: qsTr("Mark the extremes and allow tap-to-measure "
                                      + "points on the picture.")
                    checked: app.settings.showMarkers
                    onClicked: app.settings.showMarkers = !app.settings.showMarkers
                }
                TextSwitch {
                    text: qsTr("Centre spot")
                    description: qsTr("Crosshair in the middle with its temperature.")
                    checked: app.settings.showCenter
                    onClicked: app.settings.showCenter = !app.settings.showCenter
                }
                TextSwitch {
                    text: qsTr("Colour bar")
                    description: qsTr("Scale from coldest to hottest along the right edge.")
                    checked: app.settings.showBar
                    onClicked: app.settings.showBar = !app.settings.showBar
                }
                TextSwitch {
                    text: qsTr("Min/max readout")
                    description: qsTr("The hottest and coldest temperature at the top "
                                      + "of the picture, on screen and in snapshots.")
                    checked: app.settings.showReadout
                    onClicked: app.settings.showReadout = !app.settings.showReadout
                }

                ComboBox {
                    label: qsTr("Location overlay")
                    description: qsTr("Show GPS coordinates at the bottom of the picture "
                                      + "and burn them into snapshots. Needs the location "
                                      + "permission and a GPS fix.")
                    currentIndex: app.settings.geoFormat
                    menu: ContextMenu {
                        MenuItem { text: qsTr("Off") }
                        MenuItem { text: qsTr("Decimal degrees") }
                        MenuItem { text: qsTr("Nautical (deg + min)") }
                        MenuItem { text: qsTr("Navigation (deg/min/sec)") }
                    }
                    onCurrentIndexChanged: app.settings.geoFormat = currentIndex
                }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Clear measurement points")
                    enabled: app.points.count > 0
                    onClicked: app.points.clear()
                }

                Item { width: 1; height: Theme.paddingMedium }

                SectionHeader { text: qsTr("Snapshots") }

                TextSwitch {
                    text: qsTr("Burn in date and time")
                    description: qsTr("Shown in the corner of the picture and written "
                                      + "into every snapshot.")
                    checked: app.settings.showTimestamp
                    onClicked: app.settings.showTimestamp = !app.settings.showTimestamp
                }
                TextSwitch {
                    text: qsTr("Save temperature grid as CSV")
                    description: qsTr("Writes the raw temperature of every sensor pixel "
                                      + "next to the picture, oriented like it, as "
                                      + "comma-separated values.")
                    checked: app.settings.saveCsv
                    onClicked: app.settings.saveCsv = !app.settings.saveCsv
                }

                SectionHeader { text: qsTr("Video") }

                Slider {
                    id: bitrateSlider
                    width: parent.width
                    label: qsTr("Video compression")
                    // Bitrate in kbit/s. Left = strong compression / small file,
                    // right = light compression / better quality.
                    minimumValue: 200
                    maximumValue: 4000
                    stepSize: 100
                    value: app.settings.videoBitrate / 1000
                    // ~ MB per minute = kbit/s × 60 / 8000.
                    valueText: (value * 0.0075).toFixed(1) + " " + qsTr("MB/min")
                    onValueChanged: app.settings.videoBitrate = Math.round(value) * 1000
                    enabled: !app.recorder.recording
                }
                Label {
                    x: Theme.horizontalPageMargin
                    width: parent.width - 2 * Theme.horizontalPageMargin
                    wrapMode: Text.Wrap
                    font.pixelSize: Theme.fontSizeExtraSmall
                    color: Theme.secondaryColor
                    text: qsTr("Videos are encoded as VP8/WebM with the overlay burnt "
                               + "in. Lower means smaller files and stronger compression; "
                               + "move right for more detail. Applies to the next "
                               + "recording. Switch to video mode with the button next "
                               + "to the gallery, then use the red record button.")
                }

                SectionHeader { text: qsTr("Behaviour") }

                TextSwitch {
                    text: qsTr("Keep the display on")
                    description: qsTr("Prevents blanking while the camera is live.")
                    checked: app.settings.keepDisplayOn
                    onClicked: app.settings.keepDisplayOn = !app.settings.keepDisplayOn
                }

                SectionHeader { text: qsTr("Storage") }

                DetailItem {
                    label: qsTr("Snapshots")
                    value: app.captures.pictureDir
                }

                Item { width: 1; height: Theme.paddingLarge }

                Button {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Open the gallery")
                    onClicked: pageStack.push(Qt.resolvedUrl("GalleryPage.qml"))
                }


                Item { width: 1; height: Theme.paddingLarge }
            }

            VerticalScrollDecorator {}
        }
}
