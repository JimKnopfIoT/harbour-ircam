// palettes.h — 256-entry false-colour LUTs for the thermal image. GPLv3+, © JimKnopfIoT.
#ifndef PALETTES_H
#define PALETTES_H

#include <QColor>
#include <QRgb>
#include <QStringList>

namespace palettes {

enum Id {
    Lava = 0,
    Ironbow,
    Rainbow,
    WhiteHot,
    BlackHot,
    GreyRed,
    Count
};

// Names in Id order.
QStringList names();

// 256-entry table for id; pointer owned by this module.
const QRgb *lut(int id);

// Colour at normalised position 0..1.
QColor colorAt(int id, double t);

} // namespace palettes

#endif // PALETTES_H
