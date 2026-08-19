// thermalcamera.cpp — see thermalcamera.h. GPLv3+, © JimKnopfIoT.
#include "thermalcamera.h"
#include "palettes.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QElapsedTimer>
#include <QMutexLocker>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <linux/videodev2.h>

// Sensor: 256x192 image; UVC stream 256x384 (temperature plane appended below).
static const int SRC_W = 256;
static const int SRC_H = 384;
static const int IMG_H = 192;
static const int UPSCALE = 3;            // 256x192 -> 768x576
static const double RANGE_SMOOTH = 0.7;  // displayed min/max smoothing (1 = frozen)
// Smallest palette window (°C); keeps a near-uniform scene from blowing up to a white/black blob.
static const double MIN_SPAN_C = 6.0;
static const int RESCAN_MS = 1500;       // device rescan interval while unplugged

static int xioctl(int fd, unsigned long req, void *arg)
{
    int r;
    do { r = ioctl(fd, req, arg); } while (r == -1 && errno == EINTR);
    return r;
}

// Picture pipeline runs on 8-bit intensity (fixed-point) up to the palette lookup.

// Bilinear 3x upscale of an 8-bit w×h image into (3w)×(3h).
static void upscale3(const unsigned char *src, int w, int h, unsigned char *dst)
{
    const int W = w * UPSCALE, H = h * UPSCALE;
    // Per-column source index + weight (8-bit fixed point), centre-aligned.
    std::vector<int> x0(W), x1(W), wx(W);
    for (int X = 0; X < W; ++X) {
        double sx = (X + 0.5) / UPSCALE - 0.5;
        if (sx < 0) sx = 0;
        if (sx > w - 1) sx = w - 1;
        const int i = int(sx);
        x0[X] = i;
        x1[X] = i + 1 < w ? i + 1 : i;
        wx[X] = int((sx - i) * 256.0 + 0.5);
    }
    for (int Y = 0; Y < H; ++Y) {
        double sy = (Y + 0.5) / UPSCALE - 0.5;
        if (sy < 0) sy = 0;
        if (sy > h - 1) sy = h - 1;
        const int j = int(sy);
        const int j1 = j + 1 < h ? j + 1 : j;
        const int wy = int((sy - j) * 256.0 + 0.5);
        const unsigned char *r0 = src + j * w;
        const unsigned char *r1 = src + j1 * w;
        unsigned char *o = dst + Y * W;
        for (int X = 0; X < W; ++X) {
            const int a = r0[x0[X]] * (256 - wx[X]) + r0[x1[X]] * wx[X];
            const int b = r1[x0[X]] * (256 - wx[X]) + r1[x1[X]] * wx[X];
            o[X] = (unsigned char)((a * (256 - wy) + b * wy + 32768) >> 16);
        }
    }
}

// Separable 5-tap Gaussian ([1,4,6,4,1]/16) on an 8-bit image, edges clamped; tmp = scratch.
static void gauss5(const unsigned char *src, int w, int h, unsigned char *dst, unsigned char *tmp)
{
    for (int y = 0; y < h; ++y) {
        const unsigned char *s = src + y * w;
        unsigned char *d = tmp + y * w;
        for (int x = 0; x < w; ++x) {
            const int xm2 = x >= 2 ? x - 2 : 0, xm1 = x >= 1 ? x - 1 : 0;
            const int xp1 = x + 1 < w ? x + 1 : w - 1, xp2 = x + 2 < w ? x + 2 : w - 1;
            d[x] = (unsigned char)((s[xm2] + 4 * s[xm1] + 6 * s[x] + 4 * s[xp1] + s[xp2] + 8) >> 4);
        }
    }
    for (int y = 0; y < h; ++y) {
        const int ym2 = y >= 2 ? y - 2 : 0, ym1 = y >= 1 ? y - 1 : 0;
        const int yp1 = y + 1 < h ? y + 1 : h - 1, yp2 = y + 2 < h ? y + 2 : h - 1;
        const unsigned char *a = tmp + ym2 * w, *b = tmp + ym1 * w, *c = tmp + y * w;
        const unsigned char *d1 = tmp + yp1 * w, *e = tmp + yp2 * w;
        unsigned char *o = dst + y * w;
        for (int x = 0; x < w; ++x)
            o[x] = (unsigned char)((a[x] + 4 * b[x] + 6 * c[x] + 4 * d1[x] + e[x] + 8) >> 4);
    }
}

// Unsharp mask: out = up + amt*(up - blur), in place on up.
static void unsharp(unsigned char *up, const unsigned char *blur, int n, double amt)
{
    const int k = int(amt * 256.0 + 0.5);
    for (int i = 0; i < n; ++i) {
        int v = up[i] + ((k * (int(up[i]) - int(blur[i]))) >> 8);
        up[i] = (unsigned char)(v < 0 ? 0 : v > 255 ? 255 : v);
    }
}

ThermalCamera::ThermalCamera(QObject *parent)
    : QObject(parent)
    , m_run(false)
    , m_streaming(false)
    , m_status(Idle)
    , m_fps(0)
    , m_tw(0), m_th(0)
    , m_tMin(0), m_tMax(0), m_tCenter(0)
    , m_winLo(0), m_winHi(0)
    , m_tMinX(0), m_tMinY(0), m_tMaxX(0), m_tMaxY(0)
    , m_tempValid(false)
    , m_tempOffset(0.0)
    , m_sharpen(1.2)
    , m_palette(palettes::Lava)
{
    m_profiles = cameraprofiles::builtins();
    m_profiles += cameraprofiles::loadUserDb();
    if (!m_profiles.isEmpty())
        m_active = m_profiles.first();
}

ThermalCamera::~ThermalCamera()
{
    stop();
}

void ThermalCamera::setStatus(Status s, const QString &detail, const QString &dev)
{
    {
        QMutexLocker lock(&m_mx);
        if (m_status == s && m_detail == detail && m_device == dev)
            return;
        m_status = s;
        m_detail = detail;
        m_device = dev;
    }
    qWarning() << "[ircam]" << s << detail << dev;
    emit statusChanged();
}

void ThermalCamera::setStreaming(bool on)
{
    if (m_streaming.load() == on)
        return;
    m_streaming.store(on);
    emit streamingChanged();
}

QString ThermalCamera::statusText() const
{
    QMutexLocker lock(&m_mx);
    switch (m_status) {
    case Idle:      return tr("Camera off");
    case Searching: return tr("Looking for camera…");
    case Streaming: return tr("Live");
    case Error:     return tr("Camera error");
    }
    return QString();
}

QString ThermalCamera::statusDetail() const
{
    QMutexLocker lock(&m_mx);
    return m_detail;
}

QString ThermalCamera::devicePath() const
{
    QMutexLocker lock(&m_mx);
    return m_device;
}

double ThermalCamera::tMin()    { QMutexLocker l(&m_mx); return m_tMin + m_tempOffset.load(); }
double ThermalCamera::tMax()    { QMutexLocker l(&m_mx); return m_tMax + m_tempOffset.load(); }
double ThermalCamera::tCenter() { QMutexLocker l(&m_mx); return m_tCenter + m_tempOffset.load(); }
double ThermalCamera::tWinLo()  { QMutexLocker l(&m_mx); return m_winLo + m_tempOffset.load(); }
double ThermalCamera::tWinHi()  { QMutexLocker l(&m_mx); return m_winHi + m_tempOffset.load(); }
double ThermalCamera::tMinX()   { QMutexLocker l(&m_mx); return m_tMinX; }
double ThermalCamera::tMinY()   { QMutexLocker l(&m_mx); return m_tMinY; }
double ThermalCamera::tMaxX()   { QMutexLocker l(&m_mx); return m_tMaxX; }
double ThermalCamera::tMaxY()   { QMutexLocker l(&m_mx); return m_tMaxY; }
bool ThermalCamera::tempValid() { QMutexLocker l(&m_mx); return m_tempValid; }

double ThermalCamera::tempAt(double nx, double ny)
{
    QMutexLocker lock(&m_mx);
    if (!m_tempValid || m_tw <= 0 || m_th <= 0)
        return std::numeric_limits<double>::quiet_NaN();
    const int x = qBound(0, int(nx * m_tw), m_tw - 1);
    const int y = qBound(0, int(ny * m_th), m_th - 1);
    return double(m_temps[y * m_tw + x]) + m_tempOffset.load();
}

QImage ThermalCamera::currentImage()
{
    QMutexLocker lock(&m_mx);
    return m_frame;
}

std::vector<float> ThermalCamera::currentTemps(int *width, int *height)
{
    QMutexLocker lock(&m_mx);
    if (!m_tempValid) {
        if (width) *width = 0;
        if (height) *height = 0;
        return std::vector<float>();
    }
    std::vector<float> out(m_temps);
    const float off = float(m_tempOffset.load());
    for (size_t i = 0; i < out.size(); ++i)
        out[i] += off;
    if (width) *width = m_tw;
    if (height) *height = m_th;
    return out;
}

void ThermalCamera::setTempOffset(double v)
{
    if (v < -50) v = -50;
    if (v > 50) v = 50;
    if (qFuzzyCompare(m_tempOffset.load(), v))
        return;
    m_tempOffset.store(v);
    emit tempOffsetChanged();
    emit tempUpdated();          // labels follow immediately
}

void ThermalCamera::setSharpen(double v)
{
    if (v < 0) v = 0;
    if (v > 4) v = 4;
    if (qFuzzyCompare(m_sharpen.load(), v))
        return;
    m_sharpen.store(v);
    emit sharpenChanged();
}

void ThermalCamera::setPalette(int id)
{
    if (id < 0 || id >= palettes::Count)
        id = palettes::Lava;
    if (m_palette.load() == id)
        return;
    m_palette.store(id);
    emit paletteChanged();
}

QString ThermalCamera::cameraName()
{
    QMutexLocker l(&m_mx);
    return m_detected.value(QStringLiteral("name")).toString();
}

bool ThermalCamera::cameraKnown()
{
    QMutexLocker l(&m_mx);
    return m_detected.value(QStringLiteral("known"), false).toBool();
}

QVariantList ThermalCamera::profiles()
{
    QVariantList v;
    for (int i = 0; i < m_profiles.size(); ++i)
        v.append(m_profiles[i].toMap());
    return v;
}

QVariantMap ThermalCamera::detected()
{
    QMutexLocker l(&m_mx);
    return m_detected;
}

bool ThermalCamera::saveDetectedCamera(const QString &name)
{
    CameraProfile p = m_active;
    if (!name.trimmed().isEmpty())
        p.name = name.trimmed();
    p.builtin = false;
    if (!cameraprofiles::appendUserDb(p))
        return false;
    m_profiles = cameraprofiles::builtins();
    m_profiles += cameraprofiles::loadUserDb();
    emit camerasChanged();
    return true;
}

QString ThermalCamera::exportDataPoint()
{
    return QString::fromUtf8(QJsonDocument(QJsonObject::fromVariantMap(m_active.toMap()))
                             .toJson(QJsonDocument::Compact));
}

QStringList ThermalCamera::paletteNames() const
{
    return palettes::names();
}

QStringList ThermalCamera::paletteStops(int steps) const
{
    QStringList out;
    if (steps < 2)
        steps = 2;
    for (int i = 0; i < steps; ++i)
        out << palettes::colorAt(m_palette.load(), 1.0 - double(i) / (steps - 1)).name();
    return out;
}

void ThermalCamera::start(const QString &device)
{
    if (m_run.load())
        return;
    if (m_thread.joinable())
        m_thread.join();
    m_forcedDevice = device;
    m_run.store(true);
    emit runningChanged();
    setStatus(Searching, tr("Plug the camera into the USB-C port."));
    m_thread = std::thread(&ThermalCamera::loop, this);
}

void ThermalCamera::stop()
{
    const bool wasRunning = m_run.load();
    m_run.store(false);
    if (m_thread.joinable())
        m_thread.join();
    if (wasRunning) {
        setStreaming(false);
        setStatus(Idle);
        emit runningChanged();
        if (m_fps != 0) { m_fps = 0; emit fpsChanged(); }
    }
}

// Read a video node's USB vid:pid from its sysfs modalias ("usb:vXXXXpXXXX...").
bool ThermalCamera::readVidPid(const QString &node, int *vid, int *pid)
{
    *vid = -1; *pid = -1;
    QFile f(QStringLiteral("/sys/class/video4linux/") + node + QStringLiteral("/device/modalias"));
    if (!f.open(QIODevice::ReadOnly))
        return false;
    const QString m = QString::fromLatin1(f.readAll());
    const int vi = m.indexOf(QLatin1String("usb:v"));
    if (vi < 0)
        return false;
    bool okV = false, okP = false;
    *vid = m.mid(vi + 5, 4).toInt(&okV, 16);
    const int pi = m.indexOf(QLatin1Char('p'), vi + 5);
    if (pi > 0)
        *pid = m.mid(pi + 1, 4).toInt(&okP, 16);
    return okV && okP;
}

// Index of the first profile this device satisfies (vid:pid first, then format-only).
int ThermalCamera::matchProfile(int fd, int vid, int pid)
{
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = 0; i < m_profiles.size(); ++i) {
            const CameraProfile &p = m_profiles[i];
            if (pass == 0 && !(p.vid == vid && p.pid == pid))
                continue;
            struct v4l2_format fmt;
            memset(&fmt, 0, sizeof fmt);
            fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            fmt.fmt.pix.width = p.streamW;
            fmt.fmt.pix.height = p.streamH;
            fmt.fmt.pix.pixelformat = p.pixelFormat;
            fmt.fmt.pix.field = V4L2_FIELD_NONE;
            if (xioctl(fd, VIDIOC_TRY_FMT, &fmt) == 0
                    && int(fmt.fmt.pix.width) == p.streamW
                    && int(fmt.fmt.pix.height) == p.streamH
                    && fmt.fmt.pix.pixelformat == p.pixelFormat)
                return i;
        }
    }
    return -1;
}

// Characterise an unknown UVC camera from its enumerated formats: pick the
// largest supported frame and treat it image-only. Radiometric auto-detection
// happens later on the live stream. Confidence stays low so the UI asks the user.
CameraProfile ThermalCamera::probe(int fd, int vid, int pid)
{
    CameraProfile best;
    struct v4l2_fmtdesc fd0;
    for (unsigned fi = 0; ; ++fi) {
        memset(&fd0, 0, sizeof fd0);
        fd0.index = fi;
        fd0.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (xioctl(fd, VIDIOC_ENUM_FMT, &fd0) != 0)
            break;
        struct v4l2_frmsizeenum fs;
        for (unsigned si = 0; ; ++si) {
            memset(&fs, 0, sizeof fs);
            fs.index = si;
            fs.pixel_format = fd0.pixelformat;
            if (xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &fs) != 0)
                break;
            if (fs.type != V4L2_FRMSIZE_TYPE_DISCRETE)
                continue;
            const int wpx = fs.discrete.width, hpx = fs.discrete.height;
            if (wpx * hpx > best.streamW * best.streamH) {
                best.streamW = wpx; best.streamH = hpx;
                best.pixelFormat = fd0.pixelformat;
            }
        }
    }
    best.vid = vid; best.pid = pid;
    best.imageW = best.streamW;
    best.imageH = best.streamH;
    best.hasTemp = false;
    best.tempMode = QStringLiteral("none");
    best.name = QStringLiteral("Unrecognised camera");
    return best;
}

// Scan /dev/video* for a usable UVC capture node; match it to a profile or probe
// it. Sets m_active and the detected map. Returns the device path or empty.
QString ThermalCamera::findDevice()
{
    if (!m_forcedDevice.isEmpty())
        return m_forcedDevice;

    QDir dev(QStringLiteral("/dev"));
    QStringList nodes = dev.entryList(QStringList() << QStringLiteral("video*"),
                                      QDir::System | QDir::Files, QDir::Name);
    for (int i = 0; i < nodes.count(); ++i) {
        const QString node = nodes.at(i);
        const QString path = QStringLiteral("/dev/") + node;
        const QByteArray p = path.toUtf8();
        int fd = ::open(p.constData(), O_RDWR | O_NONBLOCK);
        if (fd < 0)
            continue;
        struct v4l2_capability cap;
        memset(&cap, 0, sizeof cap);
        bool usable = false;
        if (xioctl(fd, VIDIOC_QUERYCAP, &cap) == 0) {
            unsigned caps = (cap.capabilities & V4L2_CAP_DEVICE_CAPS)
                          ? cap.device_caps : cap.capabilities;
            usable = (caps & V4L2_CAP_VIDEO_CAPTURE)
                  && !(caps & V4L2_CAP_VIDEO_M2M)
                  && !(caps & V4L2_CAP_VIDEO_OUTPUT)
                  && qstrcmp(reinterpret_cast<const char *>(cap.driver), "uvcvideo") == 0;
        }
        if (!usable) { ::close(fd); continue; }

        int vid = -1, pid = -1;
        readVidPid(node, &vid, &pid);
        const int idx = matchProfile(fd, vid, pid);
        CameraProfile chosen;
        bool known = false;
        if (idx >= 0) { chosen = m_profiles[idx]; known = true; }
        else          { chosen = probe(fd, vid, pid); }
        ::close(fd);

        if (!chosen.valid())
            continue;
        m_active = chosen;
        QVariantMap d = chosen.toMap();
        d["known"] = known;
        d["vidHex"] = QString::asprintf("%04x", vid < 0 ? 0 : vid);
        d["pidHex"] = QString::asprintf("%04x", pid < 0 ? 0 : pid);
        {
            QMutexLocker lock(&m_mx);
            m_detected = d;
        }
        emit camerasChanged();
        return path;
    }
    return QString();
}

// One raw frame in, display image + measurements out. Geometry and temperature
// decoding come from m_active (the matched profile), so any UVC "image (+ optional
// temperature) plane" camera works, not only the P2 Pro.
bool ThermalCamera::processFrame(const unsigned char *data, int w,
                                 std::vector<float> &temps,
                                 double &emaLo, double &emaHi, bool &emaInit)
{
    const QRgb *lut = palettes::lut(m_palette.load());
    const int iw = m_active.imageW > 0 ? m_active.imageW : w;
    const int ih = m_active.imageH > 0 ? m_active.imageH : 1;
    const bool linear = (m_active.tempMode == QLatin1String("linear"));
    const int toff = m_active.tempRowOffset;
    const double sc = m_active.tempScale, oc = m_active.tempOffsetC;

    std::vector<unsigned char> &small = m_small;
    small.resize(size_t(iw) * ih);
    bool tok = false;
    double tmin = 1e9, tmax = -1e9, tcenter = 0, winLoOut = 0, winHiOut = 0;
    int mnX = 0, mnY = 0, mxX = 0, mxY = 0;

    if (linear) {
        // Temperature plane: 16-bit LE, °C = raw*scale + offset. Everything derives from it.
        temps.resize(size_t(iw) * ih);
        for (int y = 0; y < ih; ++y) {
            const unsigned char *row = data + (size_t(toff + y) * w) * 2;
            float *trow = temps.data() + size_t(y) * iw;
            for (int x = 0; x < iw; ++x) {
                const int raw = row[x * 2] | (row[x * 2 + 1] << 8);
                const double c = raw * sc + oc;
                trow[x] = float(c);
                if (c < tmin) { tmin = c; mnX = x; mnY = y; }
                if (c > tmax) { tmax = c; mxX = x; mxY = y; }
            }
        }
        tok = (tmax > -60.0 && tmax < 600.0 && tmin > -60.0 && tmin < 600.0);
        if (tok) {
            if (!emaInit) { emaLo = tmin; emaHi = tmax; emaInit = true; }
            else {
                emaLo = RANGE_SMOOTH * emaLo + (1 - RANGE_SMOOTH) * tmin;
                emaHi = RANGE_SMOOTH * emaHi + (1 - RANGE_SMOOTH) * tmax;
            }
        }
        double winLo = emaLo, winHi = emaHi;
        if (winHi - winLo < MIN_SPAN_C) {
            const double c = (winHi + winLo) / 2.0;
            winLo = c - MIN_SPAN_C / 2.0;
            winHi = c + MIN_SPAN_C / 2.0;
        }
        const double inv = 255.0 / (winHi - winLo);
        for (int y = 0; y < ih; ++y) {
            const float *trow = temps.data() + size_t(y) * iw;
            unsigned char *o = small.data() + size_t(y) * iw;
            for (int x = 0; x < iw; ++x)
                o[x] = (unsigned char)qBound(0, int((trow[x] - winLo) * inv), 255);
        }
        tcenter = temps[size_t(ih / 2) * iw + iw / 2];
        winLoOut = winLo; winHiOut = winHi;
    } else {
        // No temperature plane: auto-contrast the Y channel of the image rows.
        int lo = 255, hi = 0;
        for (int y = 0; y < ih; ++y) {
            const unsigned char *row = data + (size_t(y) * w) * 2;
            for (int x = 0; x < iw; ++x) { const int Y = row[x * 2]; if (Y < lo) lo = Y; if (Y > hi) hi = Y; }
        }
        int span = hi - lo; if (span < 1) span = 1;
        for (int y = 0; y < ih; ++y) {
            const unsigned char *row = data + (size_t(y) * w) * 2;
            unsigned char *o = small.data() + size_t(y) * iw;
            for (int x = 0; x < iw; ++x)
                o[x] = (unsigned char)qBound(0, (row[x * 2] - lo) * 255 / span, 255);
        }
    }

    const int W = iw * UPSCALE, H = ih * UPSCALE;
    m_up.resize(size_t(W) * H);
    upscale3(small.data(), iw, ih, m_up.data());
    const double amt = m_sharpen.load();
    if (amt > 0.0) {
        m_blur.resize(size_t(W) * H);
        m_tmp.resize(size_t(W) * H);
        gauss5(m_up.data(), W, H, m_blur.data(), m_tmp.data());
        unsharp(m_up.data(), m_blur.data(), W * H, amt);
    }
    QImage fin(W, H, QImage::Format_RGB32);
    for (int y = 0; y < H; ++y) {
        const unsigned char *u = m_up.data() + size_t(y) * W;
        QRgb *o = reinterpret_cast<QRgb *>(fin.scanLine(y));
        for (int x = 0; x < W; ++x)
            o[x] = lut[u[x]];
    }

    {
        QMutexLocker lock(&m_mx);
        m_frame = fin;
        if (linear && tok) {
            m_temps = temps;
            m_tw = iw; m_th = ih;
            m_tMin = emaLo; m_tMax = emaHi; m_tCenter = tcenter;
            m_winLo = winLoOut; m_winHi = winHiOut;
            m_tMinX = (mnX + 0.5) / iw; m_tMinY = (mnY + 0.5) / ih;
            m_tMaxX = (mxX + 0.5) / iw; m_tMaxY = (mxY + 0.5) / ih;
            m_tempValid = true;
        } else {
            m_tempValid = false;
        }
    }
    return linear ? tok : true;
}

// Synthetic frames for development (IRCAM_SIMULATE=1); same layout as the camera.
void ThermalCamera::simulate()
{
    m_active = cameraprofiles::builtins().first();     // P2 Pro geometry
    {
        QMutexLocker lock(&m_mx);
        m_detected = m_active.toMap();
        m_detected["known"] = true;
    }
    emit camerasChanged();
    setStatus(Streaming, tr("Simulated camera"), QStringLiteral("simulator"));
    std::vector<unsigned char> buf(size_t(SRC_W) * SRC_H * 2);
    // IRCAM_RAWFILE replays a captured raw frame instead of the synthetic scene.
    std::vector<unsigned char> canned;
    {
        const QByteArray rawPath = qgetenv("IRCAM_RAWFILE");
        if (!rawPath.isEmpty()) {
            FILE *fp = fopen(rawPath.constData(), "rb");
            if (fp) {
                canned.resize(size_t(SRC_W) * SRC_H * 2);
                if (fread(canned.data(), 1, canned.size(), fp) != canned.size())
                    canned.clear();
                fclose(fp);
            }
        }
    }
    std::vector<float> temps(SRC_W * IMG_H);
    double emaLo = 0, emaHi = 0;
    bool emaInit = false;
    int frames = 0;
    int fpsCount = 0;
    QElapsedTimer fpsClock;
    fpsClock.start();
    double t = 0;
    while (m_run.load()) {
        t += 0.04;
        const double bx = SRC_W * (0.5 + 0.3 * std::cos(t * 0.7));
        const double by = IMG_H * (0.5 + 0.3 * std::sin(t * 0.9));
        for (int y = 0; y < IMG_H; ++y) {
            unsigned char *prow = &buf[size_t(y) * SRC_W * 2];
            unsigned char *trow = &buf[size_t(IMG_H + y) * SRC_W * 2];
            for (int x = 0; x < SRC_W; ++x) {
                const double dx = (x - bx) / 30.0, dy = (y - by) / 30.0;
                const double cx = (x - 30) / 18.0, cy = (y - 30) / 18.0;
                double c = 22.0 + 4.0 * y / IMG_H
                         + 40.0 * std::exp(-(dx * dx + dy * dy))
                         - 12.0 * std::exp(-(cx * cx + cy * cy))
                         + 0.3 * ((x * 7 + y * 13 + frames) % 11) / 11.0;
                const int Y = qBound(0, int((c - 10.0) / 60.0 * 255.0), 255);
                prow[x * 2] = uchar(Y);
                prow[x * 2 + 1] = 128;
                const int raw = int((c + 273.15) * 64.0);
                trow[x * 2] = uchar(raw & 0xff);
                trow[x * 2 + 1] = uchar((raw >> 8) & 0xff);
            }
        }
        const bool tok = processFrame(canned.empty() ? buf.data() : canned.data(),
                                      SRC_W, temps, emaLo, emaHi, emaInit);
        if (frames++ == 0)
            setStreaming(true);
        emit frameAvailable();
        if (tok)
            emit tempUpdated();
        if (++fpsCount >= 10) {
            const qint64 ms = fpsClock.restart();
            if (ms > 0) { m_fps = fpsCount * 1000.0 / ms; emit fpsChanged(); }
            fpsCount = 0;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

void ThermalCamera::loop()
{
    if (!qgetenv("IRCAM_SIMULATE").isEmpty()) {
        simulate();
        return;
    }
    while (m_run.load()) {
        const QString dev = findDevice();
        if (dev.isEmpty()) {
            setStatus(Searching, tr("No thermal camera found. Plug the P2 Pro into the USB-C port."));
            // Sleep in small steps so stop() is honoured promptly.
            for (int i = 0; i < RESCAN_MS / 50 && m_run.load(); ++i)
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }
        streamFrom(dev);
        setStreaming(false);
        // Brief pause before rescanning so a replug does not race node removal.
        for (int i = 0; i < 10 && m_run.load(); ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

bool ThermalCamera::streamFrom(const QString &dev)
{
    const QByteArray devUtf8 = dev.toUtf8();
    int fd = ::open(devUtf8.constData(), O_RDWR | O_NONBLOCK);
    if (fd < 0) {
        setStatus(Error, tr("Cannot open %1: %2").arg(dev, QString::fromLocal8Bit(strerror(errno))), dev);
        return false;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = m_active.streamW;
    fmt.fmt.pix.height = m_active.streamH;
    fmt.fmt.pix.pixelformat = m_active.pixelFormat;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    if (xioctl(fd, VIDIOC_S_FMT, &fmt) < 0
            || int(fmt.fmt.pix.width) != m_active.streamW
            || int(fmt.fmt.pix.height) != m_active.streamH) {
        setStatus(Error, tr("%1 does not deliver the expected %2×%3 stream.")
                  .arg(dev).arg(m_active.streamW).arg(m_active.streamH), dev);
        ::close(fd);
        return false;
    }
    const int w = fmt.fmt.pix.width;

    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof req);
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (xioctl(fd, VIDIOC_REQBUFS, &req) < 0 || req.count < 2) {
        setStatus(Error, tr("Cannot allocate video buffers on %1.").arg(dev), dev);
        ::close(fd);
        return false;
    }
    struct Buf { void *start; size_t len; };
    Buf bufs[8];
    const unsigned nbuf = req.count > 8 ? 8 : req.count;
    unsigned mapped = 0;
    bool setupOk = true;
    for (unsigned i = 0; i < nbuf && setupOk; ++i) {
        struct v4l2_buffer b;
        memset(&b, 0, sizeof b);
        b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        b.memory = V4L2_MEMORY_MMAP;
        b.index = i;
        if (xioctl(fd, VIDIOC_QUERYBUF, &b) < 0) { setupOk = false; break; }
        bufs[i].len = b.length;
        bufs[i].start = mmap(0, b.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, b.m.offset);
        if (bufs[i].start == MAP_FAILED) { setupOk = false; break; }
        ++mapped;
        xioctl(fd, VIDIOC_QBUF, &b);
    }
    enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (!setupOk || xioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        setStatus(Error, tr("Cannot start streaming from %1.").arg(dev), dev);
        for (unsigned i = 0; i < mapped; ++i)
            munmap(bufs[i].start, bufs[i].len);
        ::close(fd);
        return false;
    }
    setStatus(Streaming, QString(), dev);

    std::vector<float> temps(size_t(m_active.imageW) * m_active.imageH);
    double emaLo = 0, emaHi = 0;
    bool emaInit = false;
    int frames = 0;
    int timeouts = 0;
    QElapsedTimer fpsClock;
    fpsClock.start();
    int fpsCount = 0;

    while (m_run.load()) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        const int r = select(fd + 1, &fds, 0, 0, &tv);
        if (r < 0) {
            if (errno == EINTR)
                continue;
            break;                         // device gone
        }
        if (r == 0) {
            // ~25 fps normally; three silent seconds = unplugged.
            if (++timeouts >= 3)
                break;
            continue;
        }
        timeouts = 0;

        struct v4l2_buffer b;
        memset(&b, 0, sizeof b);
        b.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        b.memory = V4L2_MEMORY_MMAP;
        if (xioctl(fd, VIDIOC_DQBUF, &b) < 0) {
            if (errno == EAGAIN)
                continue;
            break;                         // ENODEV etc.: unplugged
        }

        const bool tok = processFrame(static_cast<const unsigned char *>(bufs[b.index].start), w,
                                      temps, emaLo, emaHi, emaInit);
        xioctl(fd, VIDIOC_QBUF, &b);

        if (frames++ == 0)
            setStreaming(true);
        emit frameAvailable();
        if (tok)
            emit tempUpdated();

        if (++fpsCount >= 10) {
            const qint64 ms = fpsClock.restart();
            if (ms > 0) {
                m_fps = fpsCount * 1000.0 / ms;
                emit fpsChanged();
            }
            fpsCount = 0;
        }
    }

    xioctl(fd, VIDIOC_STREAMOFF, &type);
    for (unsigned i = 0; i < mapped; ++i)
        munmap(bufs[i].start, bufs[i].len);
    ::close(fd);
    if (m_run.load())
        setStatus(Searching, tr("Camera disconnected."));
    return true;
}
