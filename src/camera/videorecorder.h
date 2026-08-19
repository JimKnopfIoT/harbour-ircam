// videorecorder.h — records the composed thermal view to a compressed WebM/VP8 file. GPLv3+, © JimKnopfIoT.
//
// Pipeline: appsrc (video/x-raw BGRx) -> videoconvert -> vp8enc -> webmmux -> filesink.
//
// vp8enc, not the HW H.264 encoder: droidvenc only accepts the camera's own
// (droidcamsrc) buffers, not our appsrc frames (each is snapshot::compose()
// output). No sw H.264 on the image; vp8enc encodes 768x576 at ~120 fps, far
// ahead of 25 fps, and target-bitrate is the one compression knob.
//
// do-timestamp=false: each PTS comes from a monotonic clock at the first frame,
// so playback tracks wall-clock time despite the jittery frame rate.
//
// webmmux writes its playable header only on EOS, so stop() pushes EOS and waits
// for the bus before teardown — skip the wait and the file is truncated.

#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <QElapsedTimer>
#include <QObject>
#include <QString>
#include <QVariantMap>

#include "snapshotcomposer.h"

typedef struct _GstElement GstElement;

class ThermalCamera;

class VideoRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ThermalCamera *source READ source WRITE setSource NOTIFY sourceChanged)
    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)
    Q_PROPERTY(QString outputPath READ outputPath NOTIFY recordingChanged)
    Q_PROPERTY(int frameCount READ frameCount NOTIFY progress)
    Q_PROPERTY(qint64 durationMs READ durationMs NOTIFY progress)
    Q_PROPERTY(qint64 bytesWritten READ bytesWritten NOTIFY progress)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    // Target bitrate in bits/second. The compression slider writes this; lower =
    // smaller file. Applied at the next start(); changing it mid-recording has no
    // effect on the clip already being written.
    Q_PROPERTY(int bitrate READ bitrate WRITE setBitrate NOTIFY bitrateChanged)

public:
    explicit VideoRecorder(QObject *parent = 0);
    ~VideoRecorder();

    // Must be called once before any recorder is constructed. Wraps gst_init().
    static void initGStreamer(int *argc, char ***argv);

    ThermalCamera *source() const { return m_source; }
    void setSource(ThermalCamera *source);

    bool recording() const { return m_pipeline != 0; }
    QString outputPath() const { return m_outputPath; }
    int frameCount() const { return m_frameCount; }
    qint64 durationMs() const;
    qint64 bytesWritten() const { return m_bytesWritten; }
    QString lastError() const { return m_lastError; }
    int bitrate() const { return m_bitrate; }
    void setBitrate(int bitsPerSecond);

    // Start writing to `path`. `options` carries the same keys as
    // CaptureStore::saveSnapshot (rotation, mirrored, showMarkers, showCenter,
    // showBar, showReadout, fahrenheit, stampText, geoText, elementRotation) so
    // recorded frames match the viewfinder. Returns false and sets lastError().
    Q_INVOKABLE bool start(const QString &path, const QVariantMap &options);

    // Push EOS, wait for the muxer to finalise, tear down. Safe to call when idle.
    Q_INVOKABLE void stop();

    // Live overlay updates while recording (orientation changes, moving fixes).
    Q_INVOKABLE void setElementRotation(int degrees);
    Q_INVOKABLE void setGeoText(const QString &text);
    Q_INVOKABLE void setStampText(const QString &text);

signals:
    void sourceChanged();
    void recordingChanged();
    void progress();
    void lastErrorChanged();
    void bitrateChanged();
    // Emitted once a file is closed and complete, so the gallery can pick it up.
    void recordingFinished(const QString &path);

private slots:
    void onSourceFrame();
    void onSourceDestroyed();

private:
    void teardown();
    void setError(const QString &err);
    void pollBus();

    ThermalCamera *m_source;
    snapshot::Options m_opts;
    int m_bitrate;
    GstElement *m_pipeline;
    GstElement *m_appsrc;
    int m_width;
    int m_height;
    QString m_outputPath;
    QString m_lastError;
    int m_frameCount;
    qint64 m_bytesWritten;
    qint64 m_firstFrameNs;
    qint64 m_lastFrameNs;
    QElapsedTimer m_clock;
};

#endif // VIDEORECORDER_H
