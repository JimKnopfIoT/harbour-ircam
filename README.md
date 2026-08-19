# harbour-ircam

**A thermal camera viewer for the InfiRay P2 Pro on Sailfish OS.**

This app is more or less a byproduct of my much larger **LabCam** app
([JimKnopfIoT/labcam-phone](https://github.com/JimKnopfIoT/labcam-phone)): I
took the pure IR function out of it and provide it here on its own. It turns the
InfiRay P2 Pro USB-C thermal camera into a live viewfinder with real temperature
measurement, running entirely on the device — no network access, no telemetry.
Snapshots carry no location or device data unless I switch the location overlay
on myself.

Tested on a Sony Xperia 10 III with an InfiRay P2 Pro.

## Screenshots

<table>
  <tr>
    <td><img src="media/1.jpg" width="150"></td>
    <td><img src="media/2.jpg" width="150"></td>
    <td><img src="media/3.jpg" width="150"></td>
    <td><img src="media/4.jpg" width="150"></td>
    <td><img src="media/5.jpg" width="150"></td>
  </tr>
  <tr>
    <td><img src="media/6.jpg" width="150"></td>
    <td><img src="media/7.jpg" width="150"></td>
    <td><img src="media/8.jpg" width="150"></td>
    <td><img src="media/9.jpg" width="150"></td>
    <td><img src="media/10.jpg" width="150"></td>
  </tr>
  <tr>
    <td><img src="media/11.jpg" width="150"></td>
    <td><img src="media/12.jpg" width="150"></td>
    <td><img src="media/13.jpg" width="150"></td>
  </tr>
</table>

## What it does

- **Live thermal view** in every orientation, with a choice of palettes
  (Lava/Inferno, Ironbow, Rainbow, White-hot, Black-hot, and Grey with red
  peaks). Pinch to zoom, drag to pan, double-tap to cycle zoom.
- **Temperature measurement** decoded from the camera's own 16-bit data:
  the hottest and coldest spot, an optional centre spot, and up to six
  tap-to-place measurement points, plus a colour bar scaled to the picture.
- **Robust auto-scaling** — the palette is stretched across the scene's
  temperature range but never across less than a few degrees, so a nearly
  uniform scene stays soft instead of blowing up into a hard white blob.
- **Overlays** you can toggle from the viewfinder: a min/max readout on the
  picture and an optional location line (GPS coordinates in decimal, nautical
  or navigation format) — both drawn on the live picture and burnt into
  snapshots.
- **Snapshots** saved as JPEG in `~/Pictures/IrCam`, with the overlays burnt in
  at three times the sensor resolution. Optionally the raw per-pixel temperature
  grid is written next to each picture as CSV.
- **Gallery** — swipe the centre of a picture to step through captures; rename,
  delete and share from the pulley.

## Supported hardware

The **InfiRay P2 Pro** (USB `0bda:5830`), a 256×192 USB-C thermal camera. It
is a standard UVC device, so the kernel presents it as `/dev/videoN` and the
app reads it over V4L2. The stream is 256×384 — the picture on top, a 16-bit
temperature value per pixel underneath. Other cameras of the same family that
deliver the same stream layout (e.g. the Topdon TC001) are likely to work but
have not been tested.

## A note on accuracy

Temperatures are what the sensor reports, corrected only by an offset you can
set. Readings depend on the emissivity of the surface: matte paint, skin, wood
and most plastics read well; bare or shiny metal reflects its surroundings and
reads far too low. Treat the readings as a guide, not a calibrated measurement.

## Building

```sh
mb2 --target SailfishOS-<version>-aarch64 build
```

Requires `sailfishsilica-qt5` and `nemo-qml-plugin-configuration-qt5` at
runtime; a Qt5 + sailfishapp toolchain to build. No external libraries.

## Credit

The app structure follows the sibling harbour-pipecam. The temperature decode
(`raw / 64 − 273.15`) is the same formula the open pyThermalCamera project uses
for this sensor family.

## Licence

GNU General Public License v3.0 or later. Provided as is, with no warranty.
