<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="de_DE">
<context>
    <name>AboutPage</name>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="22"/>
        <source>About IrCam</source>
        <translation>Über IrCam</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="36"/>
        <source>Live view, temperature measurement and snapshots for the InfiRay P2 Pro thermal camera.</source>
        <translation>Livebild, Temperaturmessung und Schnappschüsse für die Wärmebildkamera InfiRay P2 Pro.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="42"/>
        <source>Supported hardware</source>
        <translation>Unterstützte Hardware</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="50"/>
        <source>InfiRay P2 Pro (USB 0bda:5830), a 256×192 USB-C thermal camera. It is a standard UVC device: the kernel driver presents it as /dev/videoN and IrCam reads it over V4L2. The stream is 256×384 — the picture on top, a 16-bit temperature value for every pixel underneath, which is where all readings come from.

Other cameras of the same family that deliver the same stream layout (e.g. the Topdon TC001) should work too, but have not been tested.</source>
        <translation>InfiRay P2 Pro (USB 0bda:5830), eine 256×192-Wärmebildkamera für USB-C. Sie ist ein normales UVC-Gerät: Der Kerneltreiber stellt sie als /dev/videoN bereit und IrCam liest sie über V4L2. Der Stream ist 256×384 groß — oben das Bild, darunter ein 16-Bit-Temperaturwert für jeden Pixel; daraus stammen alle Messwerte.

Andere Kameras derselben Familie mit demselben Stream-Aufbau (z. B. Topdon TC001) sollten ebenfalls funktionieren, sind aber nicht getestet.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="61"/>
        <source>Measuring</source>
        <translation>Messen</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="69"/>
        <source>Temperatures are what the sensor reports, corrected only by the offset you set. Readings depend on the emissivity of the surface: matte paint, skin, wood and most plastics read well; bare or shiny metal reflects its surroundings and reads far too low. Use the readings as a guide, not as a calibrated measurement.</source>
        <translation>Die Temperaturen sind das, was der Sensor meldet, nur um den eingestellten Offset korrigiert. Die Werte hängen vom Emissionsgrad der Oberfläche ab: matte Farbe, Haut, Holz und die meisten Kunststoffe werden gut gemessen; blankes oder glänzendes Metall spiegelt seine Umgebung und wird viel zu kalt angezeigt. Die Werte sind ein Anhaltspunkt, keine kalibrierte Messung.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="78"/>
        <source>Snapshots</source>
        <translation>Schnappschüsse</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="86"/>
        <source>Snapshots are saved as JPEG in ~/Pictures/IrCam with the markers and colour bar drawn in, at three times the sensor resolution. Optionally the raw temperature grid is written next to each picture as CSV. No location or device information is added to any file.</source>
        <translation>Schnappschüsse werden als JPEG in ~/Pictures/IrCam gespeichert, mit eingezeichneten Markern und Farbskala, in dreifacher Sensorauflösung. Optional wird das rohe Temperaturraster neben jedem Bild als CSV abgelegt. Keine Datei enthält Orts- oder Geräteinformationen.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="93"/>
        <source>Privacy</source>
        <translation>Datenschutz</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="101"/>
        <source>Everything runs on the device: no telemetry, no cloud component. Snapshots carry no location or device data unless you switch the location overlay on yourself — then the GPS coordinates you see are drawn into the picture.</source>
        <translation>Alles läuft auf dem Gerät: keine Telemetrie, keine Cloud. Schnappschüsse enthalten keine Standort- oder Gerätedaten, außer du schaltest die Standort-Einblendung selbst ein — dann werden die angezeigten GPS-Koordinaten ins Bild gezeichnet.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="108"/>
        <source>Licence</source>
        <translation>Lizenz</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="116"/>
        <source>GNU General Public License v3.0 or later. Provided as is, with no warranty.</source>
        <translation>GNU General Public License v3.0 oder neuer. Ohne Gewähr, wie sie ist.</translation>
    </message>
</context>
<context>
    <name>CameraDbView</name>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="16"/>
        <location filename="../qml/components/CameraDbView.qml" line="19"/>
        <source>image only</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="17"/>
        <source>radiometric</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="18"/>
        <source>radiometric (calibrated)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="27"/>
        <source>Cameras</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="30"/>
        <source>Connected now</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="33"/>
        <source>Camera</source>
        <translation type="unfinished">Kamera</translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="34"/>
        <source>None</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="36"/>
        <source>USB ID</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="38"/>
        <source>Stream</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="43"/>
        <source>Picture</source>
        <translation type="unfinished">Bild</translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="48"/>
        <source>Temperature</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="62"/>
        <source>This camera is not in the table yet. The picture works; temperature needs the right decoding. Save it and send the data point so it can go into a future release.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="72"/>
        <source>Save camera</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="75"/>
        <source>Saved to your camera database.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="77"/>
        <source>Could not save.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="81"/>
        <source>Copy data point</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="102"/>
        <source>Data point — copy and attach it on GitHub/OpenRepos</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="106"/>
        <source>Supported cameras</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="129"/>
        <source>picture</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="143"/>
        <source>★ = added by you (user database).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="147"/>
        <source>What works, what doesn&apos;t</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/components/CameraDbView.qml" line="154"/>
        <source>Works: USB-C thermal cameras that show up as a standard UVC video node and deliver an image (plus, for temperature, a radiometric plane). Same-layout cameras of the P2 Pro family are recognised automatically.

Limited: a camera whose picture works but whose temperature encoding differs shows the image only until its decoding is added.

Won&apos;t work: cameras that are not UVC (no /dev/video node) or that need a vendor handshake before they stream — those need their own driver, not a table row.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>CaptureStore</name>
    <message>
        <location filename="../src/app/capturestore.cpp" line="67"/>
        <source>%1 B</source>
        <translation>%1 B</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="69"/>
        <source>%1 kB</source>
        <translation>%1 kB</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="70"/>
        <source>%1 MB</source>
        <translation>%1 MB</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="83"/>
        <source>Cannot create folder %1</source>
        <translation>Ordner %1 kann nicht angelegt werden</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="90"/>
        <source>Cannot create %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="112"/>
        <source>No camera.</source>
        <translation>Keine Kamera.</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="135"/>
        <source>No frame to save yet.</source>
        <translation>Noch kein Bild zum Speichern.</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="143"/>
        <location filename="../src/app/capturestore.cpp" line="177"/>
        <source>Cannot write %1</source>
        <translation>%1 kann nicht geschrieben werden</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="213"/>
        <source>Cannot delete %1</source>
        <translation>%1 kann nicht gelöscht werden</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="267"/>
        <source>The name cannot be empty.</source>
        <translation>Der Name darf nicht leer sein.</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="271"/>
        <source>A name cannot contain “/” or start with a dot.</source>
        <translation>Ein Name darf kein „/“ enthalten und nicht mit einem Punkt beginnen.</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="279"/>
        <source>“%1” already exists.</source>
        <translation>„%1“ existiert bereits.</translation>
    </message>
    <message>
        <location filename="../src/app/capturestore.cpp" line="283"/>
        <source>Could not rename “%1”.</source>
        <translation>„%1“ konnte nicht umbenannt werden.</translation>
    </message>
</context>
<context>
    <name>CaptureViewPage</name>
    <message>
        <location filename="../qml/pages/CaptureViewPage.qml" line="51"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../qml/pages/CaptureViewPage.qml" line="54"/>
        <source>Deleting</source>
        <translation>Wird gelöscht</translation>
    </message>
    <message>
        <location filename="../qml/pages/CaptureViewPage.qml" line="64"/>
        <source>Rename</source>
        <translation>Umbenennen</translation>
    </message>
    <message>
        <location filename="../qml/pages/CaptureViewPage.qml" line="77"/>
        <source>Share</source>
        <translation>Teilen</translation>
    </message>
    <message>
        <location filename="../qml/pages/CaptureViewPage.qml" line="183"/>
        <source>temperature grid saved as CSV</source>
        <translation>Temperaturraster als CSV gespeichert</translation>
    </message>
</context>
<context>
    <name>GalleryPage</name>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="40"/>
        <source>Snapshots</source>
        <translation>Schnappschüsse</translation>
    </message>
    <message numerus="yes">
        <location filename="../qml/pages/GalleryPage.qml" line="42"/>
        <source>%n item(s)</source>
        <translation type="unfinished">
            <numerusform></numerusform>
            <numerusform></numerusform>
        </translation>
    </message>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="49"/>
        <source>Refresh</source>
        <translation>Aktualisieren</translation>
    </message>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="67"/>
        <source>Rename</source>
        <translation>Umbenennen</translation>
    </message>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="80"/>
        <source>Delete</source>
        <translation>Löschen</translation>
    </message>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="85"/>
        <source>Deleting</source>
        <translation>Wird gelöscht</translation>
    </message>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="125"/>
        <source>VIDEO</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="154"/>
        <source>No snapshots yet</source>
        <translation>Noch keine Schnappschüsse</translation>
    </message>
    <message>
        <location filename="../qml/pages/GalleryPage.qml" line="155"/>
        <source>Snapshots you take will appear here.</source>
        <translation>Deine Schnappschüsse erscheinen hier.</translation>
    </message>
</context>
<context>
    <name>RenameDialog</name>
    <message>
        <location filename="../qml/pages/RenameDialog.qml" line="28"/>
        <source>Rename</source>
        <translation>Umbenennen</translation>
    </message>
    <message>
        <location filename="../qml/pages/RenameDialog.qml" line="29"/>
        <source>Cancel</source>
        <translation>Abbrechen</translation>
    </message>
    <message>
        <location filename="../qml/pages/RenameDialog.qml" line="35"/>
        <location filename="../qml/pages/RenameDialog.qml" line="36"/>
        <source>Name</source>
        <translation>Name</translation>
    </message>
    <message>
        <location filename="../qml/pages/RenameDialog.qml" line="53"/>
        <source>Saved as “%1”</source>
        <translation>Gespeichert als „%1“</translation>
    </message>
</context>
<context>
    <name>SettingsPage</name>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="34"/>
        <source>Settings</source>
        <translation>Einstellungen</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="36"/>
        <source>Camera</source>
        <translation>Kamera</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="39"/>
        <source>Camera on</source>
        <translation>Kamera an</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="48"/>
        <source>Turn picture by 180°</source>
        <translation>Bild um 180° drehen</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="49"/>
        <source>The camera can be plugged in facing either way. If the picture is upside down, switch this on instead of replugging.</source>
        <translation>Die Kamera lässt sich in beide Richtungen einstecken. Steht das Bild auf dem Kopf, schalte dies ein statt umzustecken.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="57"/>
        <source>Camera points at me</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="58"/>
        <source>Turn this on when the lens faces you instead of away. A user-facing camera turns the wrong way in landscape otherwise. This only fixes the rotation — use &quot;Mirror picture&quot; as well if you also want a left-right flip. (The plug direction cannot be detected automatically.)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="69"/>
        <source>Scaling</source>
        <translation>Skalierung</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="70"/>
        <source>How the 4:3 thermal picture fills the screen.</source>
        <translation>Wie das 4:3-Wärmebild den Bildschirm füllt.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="73"/>
        <source>Fit — show the whole picture</source>
        <translation>Einpassen — ganzes Bild zeigen</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="74"/>
        <source>Fill — crop the edges</source>
        <translation>Füllen — Ränder beschneiden</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="79"/>
        <source>Picture</source>
        <translation>Bild</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="82"/>
        <source>Palette</source>
        <translation>Palette</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="95"/>
        <source>Sharpening</source>
        <translation>Schärfung</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="104"/>
        <source>Measurement</source>
        <translation>Messung</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="107"/>
        <source>Fahrenheit</source>
        <translation>Fahrenheit</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="108"/>
        <source>Show and save temperatures in °F instead of °C.</source>
        <translation>Temperaturen in °F statt °C anzeigen und speichern.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="115"/>
        <source>Temperature offset</source>
        <translation>Temperatur-Offset</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="129"/>
        <source>Added to every reading. The sensor is not calibrated in absolute terms; point it at something of known temperature and adjust until the reading matches. Readings also depend on the emissivity of the surface — shiny metal reads far too low.</source>
        <translation>Wird zu jedem Messwert addiert. Der Sensor ist absolut nicht kalibriert; richte ihn auf etwas mit bekannter Temperatur und stelle nach, bis der Wert passt. Die Werte hängen außerdem vom Emissionsgrad der Oberfläche ab — glänzendes Metall wird viel zu kalt angezeigt.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="137"/>
        <source>Hottest and coldest spot</source>
        <translation>Heißester und kältester Punkt</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="138"/>
        <source>Mark the extremes and allow tap-to-measure points on the picture.</source>
        <translation>Extreme markieren und Messpunkte per Tippen aufs Bild erlauben.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="144"/>
        <source>Centre spot</source>
        <translation>Mittelpunkt</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="145"/>
        <source>Crosshair in the middle with its temperature.</source>
        <translation>Fadenkreuz in der Mitte mit seiner Temperatur.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="150"/>
        <source>Colour bar</source>
        <translation>Farbskala</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="151"/>
        <source>Scale from coldest to hottest along the right edge.</source>
        <translation>Skala von kalt nach heiß am rechten Rand.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="156"/>
        <source>Min/max readout</source>
        <translation>Min/Max-Anzeige</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="157"/>
        <source>The hottest and coldest temperature at the top of the picture, on screen and in snapshots.</source>
        <translation>Die heißeste und kälteste Temperatur am oberen Bildrand, auf dem Schirm und in Schnappschüssen.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="164"/>
        <source>Location overlay</source>
        <translation>Standort-Einblendung</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="165"/>
        <source>Show GPS coordinates at the bottom of the picture and burn them into snapshots. Needs the location permission and a GPS fix.</source>
        <translation>GPS-Koordinaten am unteren Bildrand anzeigen und in Schnappschüsse einbrennen. Braucht die Standort-Berechtigung und einen GPS-Fix.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="170"/>
        <source>Off</source>
        <translation>Aus</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="171"/>
        <source>Decimal degrees</source>
        <translation>Dezimalgrad</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="172"/>
        <source>Nautical (deg + min)</source>
        <translation>Seemännisch (Grad + Min)</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="173"/>
        <source>Navigation (deg/min/sec)</source>
        <translation>Navigation (Grad/Min/Sek)</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="180"/>
        <source>Clear measurement points</source>
        <translation>Messpunkte löschen</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="187"/>
        <location filename="../qml/pages/SettingsPage.qml" line="247"/>
        <source>Snapshots</source>
        <translation>Schnappschüsse</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="190"/>
        <source>Burn in date and time</source>
        <translation>Datum und Uhrzeit einbrennen</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="191"/>
        <source>Shown in the corner of the picture and written into every snapshot.</source>
        <translation>Wird in der Bildecke angezeigt und in jeden Schnappschuss geschrieben.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="197"/>
        <source>Save temperature grid as CSV</source>
        <translation>Temperaturraster als CSV speichern</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="198"/>
        <source>Writes the raw temperature of every sensor pixel next to the picture, oriented like it, as comma-separated values.</source>
        <translation>Schreibt die Rohtemperatur jedes Sensorpixels neben das Bild, genauso ausgerichtet, als kommagetrennte Werte.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="205"/>
        <source>Video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="210"/>
        <source>Video compression</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="218"/>
        <source>MB/min</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="228"/>
        <source>Videos are encoded as VP8/WebM with the overlay burnt in. Lower means smaller files and stronger compression; move right for more detail. Applies to the next recording. Switch to video mode with the button next to the gallery, then use the red record button.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="235"/>
        <source>Behaviour</source>
        <translation>Verhalten</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="238"/>
        <source>Keep the display on</source>
        <translation>Display anlassen</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="239"/>
        <source>Prevents blanking while the camera is live.</source>
        <translation>Verhindert das Abdunkeln, solange die Kamera läuft.</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="244"/>
        <source>Storage</source>
        <translation>Speicherort</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="255"/>
        <source>Open the gallery</source>
        <translation>Galerie öffnen</translation>
    </message>
    <message>
        <location filename="../qml/pages/SettingsPage.qml" line="25"/>
        <source>About IrCam</source>
        <translation>Über IrCam</translation>
    </message>
</context>
<context>
    <name>StatusOverlay</name>
    <message>
        <location filename="../qml/components/StatusOverlay.qml" line="54"/>
        <source>Open settings with the gear button and switch the camera on.</source>
        <translation>Öffne die Einstellungen über das Zahnrad und schalte die Kamera ein.</translation>
    </message>
</context>
<context>
    <name>ThermalCamera</name>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="163"/>
        <source>Camera off</source>
        <translation>Kamera aus</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="164"/>
        <source>Looking for camera…</source>
        <translation>Suche Kamera …</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="165"/>
        <source>Live</source>
        <translation>Live</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="166"/>
        <source>Camera error</source>
        <translation>Kamerafehler</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="328"/>
        <source>Plug the camera into the USB-C port.</source>
        <translation>Stecke die Kamera in den USB-C-Anschluss.</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="603"/>
        <source>Simulated camera</source>
        <translation>Simulierte Kamera</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="674"/>
        <source>No thermal camera found. Plug the P2 Pro into the USB-C port.</source>
        <translation>Keine Wärmebildkamera gefunden. Stecke die P2 Pro in den USB-C-Anschluss.</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="693"/>
        <source>Cannot open %1: %2</source>
        <translation>%1 kann nicht geöffnet werden: %2</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="707"/>
        <source>%1 does not deliver the expected %2×%3 stream.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="720"/>
        <source>Cannot allocate video buffers on %1.</source>
        <translation>Auf %1 können keine Videopuffer angelegt werden.</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="744"/>
        <source>Cannot start streaming from %1.</source>
        <translation>Streaming von %1 kann nicht gestartet werden.</translation>
    </message>
    <message>
        <location filename="../src/camera/thermalcamera.cpp" line="817"/>
        <source>Camera disconnected.</source>
        <translation>Kamera getrennt.</translation>
    </message>
</context>
<context>
    <name>VideoRecorder</name>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="123"/>
        <source>Already recording.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="127"/>
        <source>No camera.</source>
        <translation type="unfinished">Keine Kamera.</translation>
    </message>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="150"/>
        <source>No frame to record yet.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="171"/>
        <source>Video recording is unavailable: a required GStreamer element is missing (appsrc/videoconvert/vp8enc/webmmux/filesink).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="210"/>
        <source>Could not build the recording pipeline.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="218"/>
        <source>Could not start recording to %1.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="309"/>
        <location filename="../src/camera/videorecorder.cpp" line="346"/>
        <source>Recording failed: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../src/camera/videorecorder.cpp" line="336"/>
        <source>Timed out finalising the video file — it may be incomplete.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>ViewfinderPage</name>
    <message>
        <location filename="../qml/pages/ViewfinderPage.qml" line="401"/>
        <source>Video</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/ViewfinderPage.qml" line="401"/>
        <source>Photo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/pages/ViewfinderPage.qml" line="487"/>
        <source>Location</source>
        <translation>Standort</translation>
    </message>
    <message>
        <location filename="../qml/pages/ViewfinderPage.qml" line="324"/>
        <source>Sharpness</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>harbour-ircam</name>
    <message>
        <location filename="../qml/harbour-ircam.qml" line="104"/>
        <source>Decimal</source>
        <translation>Dezimal</translation>
    </message>
    <message>
        <location filename="../qml/harbour-ircam.qml" line="105"/>
        <source>Nautical</source>
        <translation>Seemännisch</translation>
    </message>
    <message>
        <location filename="../qml/harbour-ircam.qml" line="106"/>
        <source>Navigation</source>
        <translation>Navigation</translation>
    </message>
    <message>
        <location filename="../qml/harbour-ircam.qml" line="107"/>
        <location filename="../qml/harbour-ircam.qml" line="260"/>
        <source>Off</source>
        <translation>Aus</translation>
    </message>
    <message>
        <location filename="../qml/harbour-ircam.qml" line="260"/>
        <source>Normal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../qml/harbour-ircam.qml" line="260"/>
        <source>Superfine</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
