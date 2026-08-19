// thermalframeitem.cpp — see thermalframeitem.h. GPLv3+, © JimKnopfIoT.
#include "thermalframeitem.h"
#include "thermalcamera.h"

#include <QMatrix4x4>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>
#include <QSGTexture>
#include <QSGTransformNode>
#include <QtMath>

static const qreal MAX_ZOOM = 4.0;
static const QSize NOMINAL(768, 576);   // frame size before the first frame

ThermalFrameItem::ThermalFrameItem(QQuickItem *parent)
    : QQuickItem(parent)
    , m_camera(0)
    , m_fillMode(PreserveAspectFit)
    , m_mirrored(false)
    , m_rotation(0)
    , m_zoom(1.0)
    , m_pan(0, 0)
    , m_hasFrame(false)
    , m_textureDirty(false)
{
    setFlag(ItemHasContents, true);
    setClip(true);
}

ThermalFrameItem::~ThermalFrameItem()
{
}

qreal ThermalFrameItem::maxZoom() const
{
    return MAX_ZOOM;
}

void ThermalFrameItem::setCamera(ThermalCamera *camera)
{
    if (m_camera == camera)
        return;
    if (m_camera)
        disconnect(m_camera, 0, this, 0);
    m_camera = camera;
    if (m_camera) {
        connect(m_camera, SIGNAL(frameAvailable()), this, SLOT(onFrameAvailable()));
        connect(m_camera, SIGNAL(destroyed()), this, SLOT(onCameraDestroyed()));
    }
    if (m_hasFrame) {
        m_hasFrame = false;
        emit hasFrameChanged();
    }
    m_pendingImage = QImage();
    m_textureDirty = true;
    update();
    emit cameraChanged();
}

void ThermalFrameItem::onCameraDestroyed()
{
    m_camera = 0;
    emit cameraChanged();
}

void ThermalFrameItem::onFrameAvailable()
{
    if (!m_camera)
        return;
    m_pendingImage = m_camera->currentImage();
    m_textureDirty = true;
    if (!m_hasFrame && !m_pendingImage.isNull()) {
        m_hasFrame = true;
        emit hasFrameChanged();
        emit frameRectChanged();
    }
    update();
}

void ThermalFrameItem::geometryDirty()
{
    m_pan = clampPan(m_pan, frameSize());
    update();
    emit frameRectChanged();
}

void ThermalFrameItem::setFillMode(FillMode mode)
{
    if (m_fillMode == mode)
        return;
    m_fillMode = mode;
    geometryDirty();
    emit fillModeChanged();
}

void ThermalFrameItem::setMirrored(bool mirrored)
{
    if (m_mirrored == mirrored)
        return;
    m_mirrored = mirrored;
    update();
    emit mirroredChanged();
}

void ThermalFrameItem::setFrameRotation(int degrees)
{
    degrees = ((degrees % 360) + 360) % 360;
    degrees = (degrees / 90) * 90;
    if (m_rotation == degrees)
        return;
    m_rotation = degrees;
    geometryDirty();
    emit frameRotationChanged();
}

void ThermalFrameItem::setZoom(qreal zoom)
{
    if (zoom < 1.0) zoom = 1.0;
    if (zoom > MAX_ZOOM) zoom = MAX_ZOOM;
    if (qFuzzyCompare(m_zoom, zoom))
        return;
    m_zoom = zoom;
    geometryDirty();
    emit zoomChanged();
}

void ThermalFrameItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size())
        geometryDirty();
}

QSize ThermalFrameItem::frameSize() const
{
    return m_pendingImage.isNull() ? NOMINAL : m_pendingImage.size();
}

QSizeF ThermalFrameItem::rotatedSize(const QSize &fs) const
{
    if (m_rotation == 90 || m_rotation == 270)
        return QSizeF(fs.height(), fs.width());
    return QSizeF(fs.width(), fs.height());
}

qreal ThermalFrameItem::scale(const QSize &fs) const
{
    const QSizeF rs = rotatedSize(fs);
    if (rs.isEmpty() || width() <= 0 || height() <= 0)
        return 1.0;
    const qreal sFit  = qMin(width() / rs.width(), height() / rs.height());
    const qreal sFill = qMax(width() / rs.width(), height() / rs.height());
    return (m_fillMode == PreserveAspectCrop ? sFill : sFit) * m_zoom;
}

QPointF ThermalFrameItem::clampPan(const QPointF &pan, const QSize &fs) const
{
    const QSizeF rs = rotatedSize(fs);
    const qreal s = scale(fs);
    // Pan range = overflow on each axis; centred where the picture does not fill.
    const qreal maxX = qMax(qreal(0), qreal((rs.width()  * s - width())  / 2.0));
    const qreal maxY = qMax(qreal(0), qreal((rs.height() * s - height()) / 2.0));
    return QPointF(qBound(-maxX, pan.x(), maxX), qBound(-maxY, pan.y(), maxY));
}

bool ThermalFrameItem::canPan() const
{
    const QPointF limit = clampPan(QPointF(1e6, 1e6), frameSize());
    return limit.x() > 0.5 || limit.y() > 0.5;
}

void ThermalFrameItem::panBy(qreal dx, qreal dy)
{
    const QPointF clamped = clampPan(m_pan + QPointF(dx, dy), frameSize());
    if (qFuzzyCompare(clamped.x(), m_pan.x()) && qFuzzyCompare(clamped.y(), m_pan.y()))
        return;
    m_pan = clamped;
    update();
    emit frameRectChanged();
}

void ThermalFrameItem::resetPan()
{
    if (m_pan.isNull())
        return;
    m_pan = QPointF(0, 0);
    update();
    emit frameRectChanged();
}

QRectF ThermalFrameItem::frameRect() const
{
    const QSize fs = frameSize();
    const QSizeF rs = rotatedSize(fs);
    const qreal s = scale(fs);
    const QPointF pan = clampPan(m_pan, fs);
    return QRectF(width() / 2.0 + pan.x() - rs.width() * s / 2.0,
                  height() / 2.0 + pan.y() - rs.height() * s / 2.0,
                  rs.width() * s, rs.height() * s);
}

// Forward: sensor pos -> centre-relative -> scale -> mirror -> rotate -> +pan -> item centre.
QPointF ThermalFrameItem::toItemPoint(qreal nx, qreal ny) const
{
    const QSize fs = frameSize();
    const qreal s = scale(fs);
    const QPointF pan = clampPan(m_pan, fs);
    qreal dx = (nx - 0.5) * fs.width() * s;
    qreal dy = (ny - 0.5) * fs.height() * s;
    if (m_mirrored)
        dx = -dx;
    const qreal th = qDegreesToRadians(qreal(m_rotation));
    const qreal c = qCos(th), sn = qSin(th);
    return QPointF(width() / 2.0 + pan.x() + dx * c - dy * sn,
                   height() / 2.0 + pan.y() + dx * sn + dy * c);
}

QPointF ThermalFrameItem::toFramePoint(qreal x, qreal y) const
{
    const QSize fs = frameSize();
    const qreal s = scale(fs);
    const QPointF pan = clampPan(m_pan, fs);
    const qreal rx = x - width() / 2.0 - pan.x();
    const qreal ry = y - height() / 2.0 - pan.y();
    const qreal th = qDegreesToRadians(qreal(m_rotation));
    const qreal c = qCos(th), sn = qSin(th);
    qreal dx =  rx * c + ry * sn;          // inverse rotation
    qreal dy = -rx * sn + ry * c;
    if (m_mirrored)
        dx = -dx;
    if (s <= 0 || fs.isEmpty())
        return QPointF(0.5, 0.5);
    return QPointF(dx / (fs.width() * s) + 0.5, dy / (fs.height() * s) + 0.5);
}

QSGNode *ThermalFrameItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    QSGTransformNode *root = static_cast<QSGTransformNode *>(oldNode);

    const QImage image = m_pendingImage;
    if (image.isNull() || width() <= 0 || height() <= 0) {
        delete root;
        return 0;
    }
    if (!root)
        root = new QSGTransformNode();

    QSGSimpleTextureNode *node = root->childCount() > 0
            ? static_cast<QSGSimpleTextureNode *>(root->firstChild()) : 0;
    if (!node) {
        node = new QSGSimpleTextureNode();
        node->setFiltering(QSGTexture::Linear);
        root->appendChildNode(node);
        m_textureDirty = true;
    }
    if (m_textureDirty) {
        QSGTexture *texture = window()->createTextureFromImage(image);
        if (!texture) {
            delete root;
            return 0;
        }
        node->setOwnsTexture(true);
        node->setTexture(texture);
        m_textureDirty = false;
    }

    // Unrotated picture rectangle, centred on the item.
    const QSize fs = image.size();
    const qreal s = scale(fs);
    const qreal dw = fs.width() * s;
    const qreal dh = fs.height() * s;
    const qreal cx = width() / 2.0;
    const qreal cy = height() / 2.0;
    node->setRect(QRectF(cx - dw / 2.0, cy - dh / 2.0, dw, dh));
    node->setSourceRect(QRectF(0, 0, fs.width(), fs.height()));

    // Transform about the item centre: mirror, then rotate, then pan. Matches toItemPoint().
    const QPointF pan = clampPan(m_pan, fs);
    QMatrix4x4 m;
    m.translate(cx + pan.x(), cy + pan.y());
    m.rotate(m_rotation, 0, 0, 1);
    if (m_mirrored)
        m.scale(-1, 1, 1);
    m.translate(-cx, -cy);
    if (root->matrix() != m) {
        root->setMatrix(m);
        root->markDirty(QSGNode::DirtyMatrix);
    }
    return root;
}
