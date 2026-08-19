// thermalcamera.h — V4L2 capture for the InfiRay P2 Pro (UVC, /dev/videoN). GPLv3+, © JimKnopfIoT.
//
// Stream: YUYV 256x384 = two stacked 256x192 planes. Rows 0..191 = image (Y
// channel); rows 192..383 = 16-bit LE raw temperature, °C = raw/64 - 273.15.
// Capture runs in a std::thread; QML reads go through m_mx or an atomic.
#ifndef THERMALCAMERA_H
#define THERMALCAMERA_H

#include <QImage>
#include <QMutex>
#include <QVariantList>
#include <QVariantMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <atomic>
#include <thread>
#include <vector>

#include "cameraprofile.h"

class ThermalCamera : public QObject
{
    Q_OBJECT
    // running = started and not stopped; streaming = frames arriving now.
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)
    Q_PROPERTY(bool streaming READ streaming NOTIFY streamingChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
    Q_PROPERTY(QString statusDetail READ statusDetail NOTIFY statusChanged)
    Q_PROPERTY(QString devicePath READ devicePath NOTIFY statusChanged)
    Q_PROPERTY(qreal fps READ fps NOTIFY fpsChanged)

    // °C (offset applied); positions normalised 0..1 over the sensor image (landscape, x right, y down).
    Q_PROPERTY(double tMin READ tMin NOTIFY tempUpdated)
    Q_PROPERTY(double tMax READ tMax NOTIFY tempUpdated)
    Q_PROPERTY(double tCenter READ tCenter NOTIFY tempUpdated)
    // Palette window (may be wider than tMin..tMax, see MIN_SPAN_C); used by the colour bar.
    Q_PROPERTY(double tWinLo READ tWinLo NOTIFY tempUpdated)
    Q_PROPERTY(double tWinHi READ tWinHi NOTIFY tempUpdated)
    Q_PROPERTY(double tMinX READ tMinX NOTIFY tempUpdated)
    Q_PROPERTY(double tMinY READ tMinY NOTIFY tempUpdated)
    Q_PROPERTY(double tMaxX READ tMaxX NOTIFY tempUpdated)
    Q_PROPERTY(double tMaxY READ tMaxY NOTIFY tempUpdated)
    Q_PROPERTY(bool tempValid READ tempValid NOTIFY tempUpdated)

    // Calibration offset in °C, added to every reported temperature.
    Q_PROPERTY(double tempOffset READ tempOffset WRITE setTempOffset NOTIFY tempOffsetChanged)
    // Unsharp-mask strength on the upscaled picture (0 = off).
    Q_PROPERTY(double sharpen READ sharpen WRITE setSharpen NOTIFY sharpenChanged)
    // Index into palettes::Id.
    Q_PROPERTY(int palette READ palette WRITE setPalette NOTIFY paletteChanged)
    Q_PROPERTY(QStringList paletteNames READ paletteNames CONSTANT)

    // Camera identification / database.
    Q_PROPERTY(QString cameraName READ cameraName NOTIFY camerasChanged)
    Q_PROPERTY(bool cameraKnown READ cameraKnown NOTIFY camerasChanged)
    Q_PROPERTY(QVariantList profiles READ profiles NOTIFY camerasChanged)   // the whole table
    Q_PROPERTY(QVariantMap detected READ detected NOTIFY camerasChanged)     // current camera, measured

public:
    enum Status { Idle, Searching, Streaming, Error };
    Q_ENUMS(Status)

    explicit ThermalCamera(QObject *parent = 0);
    ~ThermalCamera();

    bool running() const { return m_run.load(); }
    bool streaming() const { return m_streaming.load(); }
    Status status() const { return m_status; }
    QString statusText() const;
    QString statusDetail() const;
    QString devicePath() const;
    qreal fps() const { return m_fps; }

    double tMin();
    double tMax();
    double tCenter();
    double tWinLo();
    double tWinHi();
    double tMinX();
    double tMinY();
    double tMaxX();
    double tMaxY();
    bool tempValid();

    double tempOffset() const { return m_tempOffset.load(); }
    void setTempOffset(double v);
    double sharpen() const { return m_sharpen.load(); }
    void setSharpen(double v);
    int palette() const { return m_palette.load(); }
    void setPalette(int id);
    QStringList paletteNames() const;

    QString cameraName();
    bool cameraKnown();
    QVariantList profiles();
    QVariantMap detected();
    // Save the currently detected camera to the user DB under `name`.
    Q_INVOKABLE bool saveDetectedCamera(const QString &name);
    // The current camera as a JSON line, for submitting as a data point.
    Q_INVOKABLE QString exportDataPoint();

    // Thread-safe copy of the latest display image (768x576, RGB32).
    QImage currentImage();
    // Thread-safe copy of the latest °C grid (offset applied), row-major; empty if none.
    std::vector<float> currentTemps(int *width, int *height);

    // °C (offset applied) at a normalised sensor position; NaN if unknown.
    Q_INVOKABLE double tempAt(double nx, double ny);

    // `steps` palette colours as "#rrggbb", hottest first — for the QML colour bar.
    Q_INVOKABLE QStringList paletteStops(int steps) const;

    // Start looking for the camera (scans /dev/video*); an explicit path skips the scan.
    Q_INVOKABLE void start(const QString &device = QString());
    Q_INVOKABLE void stop();

signals:
    void runningChanged();
    void streamingChanged();
    void statusChanged();
    void fpsChanged();
    void frameAvailable();
    void tempUpdated();
    void tempOffsetChanged();
    void sharpenChanged();
    void paletteChanged();
    void camerasChanged();

private:
    void loop();
    void simulate();
    int matchProfile(int fd, int vid, int pid);          // index into m_profiles, or -1
    CameraProfile probe(int fd, int vid, int pid);       // self-diagnose an unknown camera
    static bool readVidPid(const QString &node, int *vid, int *pid);
    bool processFrame(const unsigned char *data, int w,
                      std::vector<float> &temps,
                      double &emaLo, double &emaHi, bool &emaInit);
    QString findDevice();
    bool streamFrom(const QString &dev);   // returns when the device is lost or stop()
    void setStatus(Status s, const QString &detail = QString(), const QString &dev = QString());
    void setStreaming(bool on);

    mutable QMutex m_mx;
    std::atomic<bool> m_run;
    std::atomic<bool> m_streaming;
    std::thread m_thread;
    QString m_forcedDevice;

    // Camera database + the profile currently in use (worker thread owns m_active).
    QVector<CameraProfile> m_profiles;
    CameraProfile m_active;
    QVariantMap m_detected;

    // status/detail/device: written by the worker, read by QML, guarded by m_mx.
    Status m_status;
    QString m_detail;
    QString m_device;
    qreal m_fps;

    QImage m_frame;
    // Reused pipeline scratch buffers (worker thread).
    std::vector<unsigned char> m_small, m_up, m_blur, m_tmp;
    std::vector<float> m_temps;  // °C without offset
    int m_tw, m_th;
    double m_tMin, m_tMax, m_tCenter;
    double m_winLo, m_winHi;
    double m_tMinX, m_tMinY, m_tMaxX, m_tMaxY;
    bool m_tempValid;

    std::atomic<double> m_tempOffset;
    std::atomic<double> m_sharpen;
    std::atomic<int> m_palette;
};

#endif // THERMALCAMERA_H
