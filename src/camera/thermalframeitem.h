// thermalframeitem.h — viewfinder QQuickItem: draws the frame as one scene-graph
// texture and maps sensor position <-> screen. GPLv3+, © JimKnopfIoT.
//
// Picture drawn unrotated, centred; the node carries mirror + rotate (0/90/180/270)
// + pan, scaled to fit/crop the rotated picture × zoom. toItemPoint()/toFramePoint()
// apply that transform and its inverse.
#ifndef THERMALFRAMEITEM_H
#define THERMALFRAMEITEM_H

#include <QImage>
#include <QPointF>
#include <QQuickItem>

class ThermalCamera;

class ThermalFrameItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(ThermalCamera *camera READ camera WRITE setCamera NOTIFY cameraChanged)
    Q_PROPERTY(FillMode fillMode READ fillMode WRITE setFillMode NOTIFY fillModeChanged)
    Q_PROPERTY(bool mirrored READ mirrored WRITE setMirrored NOTIFY mirroredChanged)
    // Picture rotation, multiples of 90. Not `rotation` (that would turn the item's coord system).
    Q_PROPERTY(int frameRotation READ frameRotation WRITE setFrameRotation NOTIFY frameRotationChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(qreal maxZoom READ maxZoom CONSTANT)
    Q_PROPERTY(bool hasFrame READ hasFrame NOTIFY hasFrameChanged)
    // Item-space rect the picture occupies (may extend beyond the item); for placing overlays.
    Q_PROPERTY(QRectF frameRect READ frameRect NOTIFY frameRectChanged)

public:
    enum FillMode { PreserveAspectFit, PreserveAspectCrop };
    Q_ENUMS(FillMode)

    explicit ThermalFrameItem(QQuickItem *parent = 0);
    ~ThermalFrameItem();

    ThermalCamera *camera() const { return m_camera; }
    void setCamera(ThermalCamera *camera);

    FillMode fillMode() const { return m_fillMode; }
    void setFillMode(FillMode mode);

    bool mirrored() const { return m_mirrored; }
    void setMirrored(bool mirrored);

    int frameRotation() const { return m_rotation; }
    void setFrameRotation(int degrees);

    qreal zoom() const { return m_zoom; }
    void setZoom(qreal zoom);
    qreal maxZoom() const;

    bool hasFrame() const { return m_hasFrame; }
    QRectF frameRect() const;

    Q_INVOKABLE void panBy(qreal dx, qreal dy);
    Q_INVOKABLE void resetPan();
    Q_INVOKABLE bool canPan() const;

    // Normalised sensor position (0..1) -> item pixels.
    Q_INVOKABLE QPointF toItemPoint(qreal nx, qreal ny) const;
    // Item pixels -> normalised sensor position (may fall outside 0..1).
    Q_INVOKABLE QPointF toFramePoint(qreal x, qreal y) const;

signals:
    void cameraChanged();
    void fillModeChanged();
    void mirroredChanged();
    void frameRotationChanged();
    void zoomChanged();
    void hasFrameChanged();
    void frameRectChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry);

private slots:
    void onFrameAvailable();
    void onCameraDestroyed();

private:
    QSize frameSize() const;
    // Picture-pixel -> item-pixel scale at the current zoom.
    qreal scale(const QSize &fs) const;
    QPointF clampPan(const QPointF &pan, const QSize &fs) const;
    // Rotated picture's bounding box, in picture pixels.
    QSizeF rotatedSize(const QSize &fs) const;
    void geometryDirty();

    ThermalCamera *m_camera;
    FillMode m_fillMode;
    bool m_mirrored;
    int m_rotation;
    qreal m_zoom;
    QPointF m_pan;
    bool m_hasFrame;
    bool m_textureDirty;
    QImage m_pendingImage;
};

#endif // THERMALFRAMEITEM_H
