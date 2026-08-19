// CameraDbView.qml — the camera database: what is connected, the supported
// table, and how to contribute a new camera. GPLv3+, © JimKnopfIoT.
import QtQuick 2.6
import Sailfish.Silica 1.0

SilicaFlickable {
    id: view
    contentHeight: col.height + Theme.paddingLarge

    property var det: app.camera.detected

    function fmtId(m) {
        return m.vidHex && m.pidHex ? m.vidHex + ":" + m.pidHex : "—"
    }
    function tempLabel(m) {
        if (!m.hasTemp) return qsTr("image only")
        return m.tempMode === "linear" ? qsTr("radiometric")
             : m.tempMode === "ht301" ? qsTr("radiometric (calibrated)")
             : qsTr("image only")
    }

    Column {
        id: col
        width: parent.width
        spacing: Theme.paddingMedium

        PageHeader { title: qsTr("Cameras") }

        // --- currently connected -------------------------------------------
        SectionHeader { text: qsTr("Connected now") }

        DetailItem {
            label: qsTr("Camera")
            value: view.det.name ? view.det.name : qsTr("None")
        }
        DetailItem { label: qsTr("USB ID"); value: view.fmtId(view.det) }
        DetailItem {
            label: qsTr("Stream")
            visible: view.det.streamW > 0
            value: view.det.streamW + "×" + view.det.streamH + " " + (view.det.fourcc ? view.det.fourcc : "")
        }
        DetailItem {
            label: qsTr("Picture")
            visible: view.det.imageW > 0
            value: view.det.imageW + "×" + view.det.imageH
        }
        DetailItem {
            label: qsTr("Temperature")
            visible: view.det.streamW > 0
            value: view.tempLabel(view.det)
                   + (view.det.hasTemp && view.det.tempMode === "linear"
                      ? "  (raw×" + view.det.tempScale.toFixed(5) + view.det.tempOffsetC.toFixed(2) + ")" : "")
        }

        Label {
            visible: view.det.streamW > 0 && !view.det.known
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.highlightColor
            text: qsTr("This camera is not in the table yet. The picture works; "
                       + "temperature needs the right decoding. Save it and send "
                       + "the data point so it can go into a future release.")
        }

        Row {
            visible: view.det.streamW > 0 && !view.det.known
            x: Theme.horizontalPageMargin
            spacing: Theme.paddingMedium
            Button {
                text: qsTr("Save camera")
                onClicked: {
                    if (app.camera.saveDetectedCamera(view.det.name))
                        saveNote.text = qsTr("Saved to your camera database.")
                    else
                        saveNote.text = qsTr("Could not save.")
                }
            }
            Button {
                text: qsTr("Copy data point")
                onClicked: {
                    dataField.text = app.camera.exportDataPoint()
                    dataField.visible = true
                }
            }
        }
        Label {
            id: saveNote
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
            visible: text !== ""
        }
        TextField {
            id: dataField
            visible: false
            width: parent.width
            readOnly: true
            label: qsTr("Data point — copy and attach it on GitHub/OpenRepos")
        }

        // --- supported table -----------------------------------------------
        SectionHeader { text: qsTr("Supported cameras") }

        Repeater {
            model: app.camera.profiles
            delegate: Column {
                x: Theme.horizontalPageMargin
                width: view.width - 2 * Theme.horizontalPageMargin
                Label {
                    width: parent.width
                    text: modelData.name + (modelData.builtin ? "" : "  ★")
                    wrapMode: Text.Wrap
                    color: Theme.primaryColor
                    font.pixelSize: Theme.fontSizeSmall
                }
                Label {
                    width: parent.width
                    wrapMode: Text.Wrap
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeExtraSmall
                    text: (modelData.vid >= 0 ? modelData.vidHex ? modelData.vidHex : "" : "")
                          + "USB " + ("0000" + modelData.vid.toString(16)).slice(-4)
                          + ":" + ("0000" + modelData.pid.toString(16)).slice(-4)
                          + " · " + modelData.streamW + "×" + modelData.streamH + " " + modelData.fourcc
                          + " · " + qsTr("picture") + " " + modelData.imageW + "×" + modelData.imageH
                          + " · " + view.tempLabel(modelData)
                          + (modelData.hasTemp && modelData.tempMode === "linear"
                             ? " (raw×" + modelData.tempScale.toFixed(5) + modelData.tempOffsetC.toFixed(2) + ")" : "")
                }
                Item { width: 1; height: Theme.paddingSmall }
            }
        }
        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
            text: qsTr("★ = added by you (user database).")
        }

        // --- capabilities & limits -----------------------------------------
        SectionHeader { text: qsTr("What works, what doesn't") }
        Label {
            x: Theme.horizontalPageMargin
            width: parent.width - 2 * Theme.horizontalPageMargin
            wrapMode: Text.Wrap
            font.pixelSize: Theme.fontSizeExtraSmall
            color: Theme.secondaryColor
            text: qsTr("Works: USB-C thermal cameras that show up as a standard "
                       + "UVC video node and deliver an image (plus, for "
                       + "temperature, a radiometric plane). Same-layout cameras "
                       + "of the P2 Pro family are recognised automatically.\n\n"
                       + "Limited: a camera whose picture works but whose "
                       + "temperature encoding differs shows the image only until "
                       + "its decoding is added.\n\n"
                       + "Won't work: cameras that are not UVC (no /dev/video "
                       + "node) or that need a vendor handshake before they "
                       + "stream — those need their own driver, not a table row.")
        }
    }

    VerticalScrollDecorator {}
}
