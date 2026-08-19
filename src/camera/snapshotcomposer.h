// snapshotcomposer.h — render the picture + overlays into the saved image. GPLv3+, © JimKnopfIoT.
#ifndef SNAPSHOTCOMPOSER_H
#define SNAPSHOTCOMPOSER_H

#include <QImage>
#include <QList>
#include <QPointF>
#include <QString>

class ThermalCamera;

namespace snapshot {

struct Options {
    int rotation;            // 0/90/180/270
    bool mirrored;
    bool showMarkers;
    bool showCenter;
    bool showBar;
    bool showReadout;
    bool fahrenheit;
    QString stampText;       // empty = no timestamp
    QString geoText;         // empty = no location line
    int elementRotation;     // overlay rotation to match the live view
    QList<QPointF> points;   // normalised sensor coords

    Options()
        : rotation(0), mirrored(false), showMarkers(false), showCenter(false)
        , showBar(false), showReadout(false), fahrenheit(false), elementRotation(0) {}
};

// Render the camera's current picture with the overlay; null if no frame yet.
QImage compose(ThermalCamera *camera, const Options &opt);

QString formatTemp(double celsius, bool fahrenheit, int decimals);

// Sensor position -> pixel in a rotated/mirrored image of `size`. Shared with the CSV writer.
QPointF mapToImage(qreal nx, qreal ny, const QSize &size, int rotation, bool mirrored);

} // namespace snapshot

#endif // SNAPSHOTCOMPOSER_H
