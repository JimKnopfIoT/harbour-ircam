// snapshotcomposer.cpp — see snapshotcomposer.h. GPLv3+, © JimKnopfIoT.
#include "snapshotcomposer.h"
#include "palettes.h"
#include "thermalcamera.h"

#include <QFont>
#include <QFontMetrics>
#include <QLinearGradient>
#include <QPainter>
#include <QPainterPath>
#include <QTransform>
#include <cmath>
#include <QtMath>

namespace snapshot {

QString formatTemp(double celsius, bool fahrenheit, int decimals)
{
    if (qIsNaN(celsius))
        return QStringLiteral("–");
    const double v = fahrenheit ? celsius * 9.0 / 5.0 + 32.0 : celsius;
    return QString::number(v, 'f', decimals) + QChar(0xB0) + (fahrenheit ? QLatin1String("F") : QLatin1String("C"));
}

QPointF mapToImage(qreal nx, qreal ny, const QSize &size, int rotation, bool mirrored)
{
    // `size` is the rotated output; map in unrotated space, then rotate the point.
    rotation = ((rotation % 360) + 360) % 360;
    const bool swap = (rotation == 90 || rotation == 270);
    const qreal uw = swap ? size.height() : size.width();
    const qreal uh = swap ? size.width() : size.height();
    qreal x = (mirrored ? 1.0 - nx : nx) * uw;
    qreal y = ny * uh;
    switch (rotation) {
    case 90:  return QPointF(uh - y, x);
    case 180: return QPointF(uw - x, uh - y);
    case 270: return QPointF(y, uw - x);
    default:  return QPointF(x, y);
    }
}

static void drawCross(QPainter &p, const QPointF &c, qreal r, const QColor &color,
                      const QString &label, const QFont &font)
{
    QPen pen(color);
    pen.setWidthF(qMax(qreal(2.0), qreal(r / 4.0)));
    pen.setCapStyle(Qt::RoundCap);
    p.setPen(pen);
    p.drawLine(QPointF(c.x() - r, c.y()), QPointF(c.x() + r, c.y()));
    p.drawLine(QPointF(c.x(), c.y() - r), QPointF(c.x(), c.y() + r));

    p.setFont(font);
    const QFontMetrics fm(font);
    QPointF tp(c.x() + r + 4, c.y() - r);
    // Keep the label inside the picture.
    const QRectF clip(0, 0, p.device()->width(), p.device()->height());
    const qreal tw = fm.width(label);
    if (tp.x() + tw > clip.right())
        tp.setX(c.x() - r - 4 - tw);
    if (tp.y() - fm.ascent() < clip.top())
        tp.setY(c.y() + r + fm.ascent());
    // Dark outline then fill, so it reads on any palette colour.
    QPainterPath path;
    path.addText(tp, font, label);
    p.setPen(QPen(QColor(0, 0, 0, 200), qMax(qreal(2.0), qreal(font.pixelSize() / 6.0))));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawPath(path);
}

static void drawOutlinedText(QPainter &p, const QPointF &pos, const QString &text,
                             const QFont &font, const QColor &fill)
{
    QPainterPath path;
    path.addText(pos, font, text);
    p.setPen(QPen(QColor(0, 0, 0, 200), qMax(qreal(2.0), qreal(font.pixelSize() / 6.0))));
    p.setBrush(Qt::NoBrush);
    p.drawPath(path);
    p.setPen(Qt::NoPen);
    p.setBrush(fill);
    p.drawPath(path);
}

QImage compose(ThermalCamera *camera, const Options &opt)
{
    if (!camera)
        return QImage();
    QImage base = camera->currentImage();
    if (base.isNull())
        return QImage();
    base = base.convertToFormat(QImage::Format_RGB32);
    if (opt.mirrored)
        base = base.mirrored(true, false);
    const int rot = ((opt.rotation % 360) + 360) % 360;
    if (rot != 0) {
        QTransform t;
        t.rotate(rot);
        base = base.transformed(t, Qt::FastTransformation);
    }

    const QSize size = base.size();
    const int rotForMap = ((opt.rotation % 360) + 360) % 360;
    const int er = ((opt.elementRotation % 360) + 360) % 360;
    const bool swap = (er == 90 || er == 270);
    const qreal uw = swap ? size.height() : size.width();   // user-view width
    const qreal uh = swap ? size.width() : size.height();
    const qreal cx = size.width() / 2.0, cy = size.height() / 2.0;
    const qreal rad = er * M_PI / 180.0;

    // Image-pixel point -> rotated, centre-origin element frame.
    auto toLocal = [&](const QPointF &q) {
        const qreal dx = q.x() - cx, dy = q.y() - cy;
        const qreal a = -rad, c = std::cos(a), sn = std::sin(a);
        return QPointF(dx * c - dy * sn, dx * sn + dy * c);
    };

    const qreal unit = qMin(uw, uh) / 100.0;
    QFont font;
    font.setBold(true);
    font.setPixelSize(qMax(12, int(unit * 3.6)));

    QPainter p(&base);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.translate(cx, cy);
    p.rotate(er);

    // The whole image is the picture; its edges in the rotated frame:
    QPointF q1 = toLocal(QPointF(0, 0)),           q2 = toLocal(QPointF(size.width(), 0));
    QPointF q3 = toLocal(QPointF(0, size.height())), q4 = toLocal(QPointF(size.width(), size.height()));
    const qreal picL = qMin(qMin(q1.x(), q2.x()), qMin(q3.x(), q4.x()));
    const qreal picR = qMax(qMax(q1.x(), q2.x()), qMax(q3.x(), q4.x()));
    const qreal picT = qMin(qMin(q1.y(), q2.y()), qMin(q3.y(), q4.y()));
    const qreal picB = qMax(qMax(q1.y(), q2.y()), qMax(q3.y(), q4.y()));

    const bool tvalid = camera->tempValid();
    const QColor green(0x5a, 0xff, 0x5a);
    const QColor red(0xff, 0x3b, 0x30);
    const QColor cyan(0x5a, 0xc8, 0xfa);
    const QColor white(Qt::white);
    const qreal r = unit * 2.2;
    const qreal m = qRound(qMin(uw, uh) * 0.03);

    // cross + label, positioned in the rotated frame (label kept inside picture)
    auto cross = [&](const QPointF &pos, const QColor &color, const QString &label) {
        QPen pen(color); pen.setWidthF(qMax(qreal(2.0), qreal(r / 4.0))); pen.setCapStyle(Qt::RoundCap);
        p.setPen(pen);
        p.drawLine(QPointF(pos.x() - r, pos.y()), QPointF(pos.x() + r, pos.y()));
        p.drawLine(QPointF(pos.x(), pos.y() - r), QPointF(pos.x(), pos.y() + r));
        const QFontMetrics fm(font);
        const qreal lw = fm.width(label);
        qreal tx = pos.x() + r + 4;                 // default to the right of the cross
        if (tx + lw > picR - 2)                     // would leave the picture -> flip left
            tx = pos.x() - r - 4 - lw;
        tx = qBound(picL + 2, tx, picR - lw - 2);   // keep fully inside horizontally
        qreal ty = pos.y() - r + fm.ascent();
        ty = qBound(picT + fm.ascent() + 2, ty, picB - 2);   // and vertically
        drawOutlinedText(p, QPointF(tx, ty), label, font, color);
    };

    if (tvalid && opt.showBar) {
        const qreal bw = qMax(qreal(10), qMin(uw, uh) * 0.03);
        const qreal by = picT + unit * 4.0;
        const qreal bh = (picB - picT) - unit * 8.0;
        const qreal bx = picR - bw - m;
        QLinearGradient g(0, by, 0, by + bh);       // hot on top
        for (int i = 0; i <= 8; ++i)
            g.setColorAt(i / 8.0, palettes::colorAt(camera->palette(), 1.0 - i / 8.0));
        p.setPen(QPen(QColor(255, 255, 255, 160), 1));
        p.setBrush(g);
        p.drawRect(QRectF(bx, by, bw, bh));
        QFont lf(font); lf.setPixelSize(qMax(11, int(unit * 3.2)));
        const QFontMetrics fm(lf);
        const double tmax = camera->tWinHi(), tmin = camera->tWinLo();
        const QString sMax = formatTemp(tmax, opt.fahrenheit, 0);
        const QString sMid = formatTemp((tmax + tmin) / 2, opt.fahrenheit, 0);
        const QString sMin = formatTemp(tmin, opt.fahrenheit, 0);
        const qreal lx = bx - 4;
        drawOutlinedText(p, QPointF(lx - fm.width(sMax), by + fm.ascent()), sMax, lf, white);
        drawOutlinedText(p, QPointF(lx - fm.width(sMid), by + bh / 2 + fm.ascent() / 2), sMid, lf, white);
        drawOutlinedText(p, QPointF(lx - fm.width(sMin), by + bh), sMin, lf, white);
    }

    if (tvalid && opt.showReadout) {
        QFont rf(font);
        const QFontMetrics fm(rf);
        const QString hot = QChar(0x25B2) + QLatin1String(" ") + formatTemp(camera->tMax(), opt.fahrenheit, 1);
        const QString cold = QChar(0x25BC) + QLatin1String(" ") + formatTemp(camera->tMin(), opt.fahrenheit, 1);
        const qreal ry = picT + m + fm.ascent();
        drawOutlinedText(p, QPointF(picL + m, ry), hot, rf, QColor(0xff, 0x6b, 0x60));
        drawOutlinedText(p, QPointF(picL + m + fm.width(hot) + unit * 3.0, ry), cold, rf, QColor(0x7f, 0xd4, 0xff));
    }

    if (tvalid && opt.showMarkers) {
        cross(toLocal(mapToImage(camera->tMaxX(), camera->tMaxY(), size, rotForMap, opt.mirrored)),
              red, formatTemp(camera->tMax(), opt.fahrenheit, 1));
        cross(toLocal(mapToImage(camera->tMinX(), camera->tMinY(), size, rotForMap, opt.mirrored)),
              cyan, formatTemp(camera->tMin(), opt.fahrenheit, 1));
        for (int i = 0; i < opt.points.count(); ++i) {
            const QPointF pt = opt.points.at(i);
            const double t = camera->tempAt(pt.x(), pt.y());
            cross(toLocal(mapToImage(pt.x(), pt.y(), size, rotForMap, opt.mirrored)),
                  green, QString::number(i + 1) + QLatin1String(": ") + formatTemp(t, opt.fahrenheit, 1));
        }
    }

    if (tvalid && opt.showCenter) {
        const QPointF c = toLocal(mapToImage(0.5, 0.5, size, rotForMap, opt.mirrored));
        const qreal cr = unit * 3.0;
        QPen pen(white); pen.setWidthF(qMax(qreal(2.0), qreal(unit / 2.5)));
        p.setPen(pen); p.setBrush(Qt::NoBrush);
        p.drawEllipse(c, cr, cr);
        p.drawLine(QPointF(c.x() - cr * 1.6, c.y()), QPointF(c.x() - cr * 0.6, c.y()));
        p.drawLine(QPointF(c.x() + cr * 0.6, c.y()), QPointF(c.x() + cr * 1.6, c.y()));
        p.drawLine(QPointF(c.x(), c.y() - cr * 1.6), QPointF(c.x(), c.y() - cr * 0.6));
        p.drawLine(QPointF(c.x(), c.y() + cr * 0.6), QPointF(c.x(), c.y() + cr * 1.6));
        const QFontMetrics fm(font);
        drawOutlinedText(p, QPointF(c.x() + cr * 1.8, c.y() - cr * 1.0 + fm.ascent() / 2),
                         formatTemp(camera->tCenter(), opt.fahrenheit, 1), font, white);
    }

    qreal bottomLine = picB - unit * 1.5;
    if (!opt.geoText.isEmpty()) {
        QFont gf(font); gf.setBold(false); gf.setPixelSize(qMax(11, int(unit * 3.0)));
        drawOutlinedText(p, QPointF(picL + m, bottomLine), opt.geoText, gf, Qt::white);
        bottomLine -= unit * 4.0;
    }
    if (!opt.stampText.isEmpty()) {
        QFont sf(font); sf.setBold(false); sf.setPixelSize(qMax(11, int(unit * 3.2)));
        drawOutlinedText(p, QPointF(picL + m, bottomLine), opt.stampText, sf, QColor(0xff, 0xd2, 0x5a));
    }

    p.end();
    return base;
}

} // namespace snapshot
