// videorecorder.cpp — see videorecorder.h for the rationale. GPLv3+, © JimKnopfIoT.
#include "videorecorder.h"
#include "thermalcamera.h"

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <QDebug>
#include <QFileInfo>
#include <QImage>

// How long to wait for webmmux to finalise the header after EOS. A WebM cluster
// flush on the phone's eMMC is quick, but a stall shouldn't hang the UI forever.
static const int EOS_TIMEOUT_MS = 5000;

// Nominal rate advertised in the caps. The true rate is jittery and is carried
// by the per-buffer timestamps; this only seeds the track header.
static const int NOMINAL_FPS = 25;

// Default target bitrate (bits/s) if the UI never sets one. 1.2 Mbit/s at the
// P2 Pro's small frame size is already good quality; the slider ranges below it.
static const int DEFAULT_BITRATE = 1200000;

void VideoRecorder::initGStreamer(int *argc, char ***argv)
{
    GError *err = 0;
    if (!gst_init_check(argc, argv, &err)) {
        qWarning() << "ircam: gst_init failed:" << (err ? err->message : "unknown");
        if (err)
            g_error_free(err);
    }
}

VideoRecorder::VideoRecorder(QObject *parent)
    : QObject(parent)
    , m_source(0)
    , m_bitrate(DEFAULT_BITRATE)
    , m_pipeline(0)
    , m_appsrc(0)
    , m_width(0)
    , m_height(0)
    , m_frameCount(0)
    , m_bytesWritten(0)
    , m_firstFrameNs(0)
    , m_lastFrameNs(-1)
{
}

VideoRecorder::~VideoRecorder()
{
    stop();
}

void VideoRecorder::setSource(ThermalCamera *source)
{
    if (m_source == source)
        return;

    if (m_source)
        disconnect(m_source, 0, this, 0);

    m_source = source;

    if (m_source) {
        connect(m_source, SIGNAL(frameAvailable()), this, SLOT(onSourceFrame()));
        // Siblings owned by QML; either can be torn down first.
        connect(m_source, SIGNAL(destroyed()), this, SLOT(onSourceDestroyed()));
    }
    emit sourceChanged();
}

void VideoRecorder::onSourceDestroyed()
{
    // Losing the camera mid-recording would leave a file with no header.
    if (m_pipeline)
        stop();
    m_source = 0;
    emit sourceChanged();
}

void VideoRecorder::setBitrate(int bitsPerSecond)
{
    if (bitsPerSecond < 50000)
        bitsPerSecond = 50000;
    if (m_bitrate == bitsPerSecond)
        return;
    m_bitrate = bitsPerSecond;
    emit bitrateChanged();
}

void VideoRecorder::setElementRotation(int degrees)
{
    m_opts.elementRotation = ((degrees % 360) + 360) % 360;
}

void VideoRecorder::setGeoText(const QString &text)
{
    m_opts.geoText = text;
}

void VideoRecorder::setStampText(const QString &text)
{
    m_opts.stampText = text;
}

qint64 VideoRecorder::durationMs() const
{
    if (m_lastFrameNs < 0)
        return 0;
    return m_lastFrameNs / 1000000;
}

void VideoRecorder::setError(const QString &err)
{
    m_lastError = err;
    qWarning() << "ircam: recorder:" << err;
    emit lastErrorChanged();
}

bool VideoRecorder::start(const QString &path, const QVariantMap &options)
{
    if (m_pipeline) {
        setError(tr("Already recording."));
        return false;
    }
    if (!m_source) {
        setError(tr("No camera."));
        return false;
    }

    // Parse the overlay options once — the same keys CaptureStore uses, so a
    // recorded frame matches a snapshot taken at the same moment.
    m_opts = snapshot::Options();
    m_opts.rotation    = options.value(QStringLiteral("rotation"), 0).toInt();
    m_opts.mirrored    = options.value(QStringLiteral("mirrored"), false).toBool();
    m_opts.showMarkers = options.value(QStringLiteral("showMarkers"), false).toBool();
    m_opts.showCenter  = options.value(QStringLiteral("showCenter"), false).toBool();
    m_opts.showBar     = options.value(QStringLiteral("showBar"), false).toBool();
    m_opts.showReadout = options.value(QStringLiteral("showReadout"), false).toBool();
    m_opts.fahrenheit  = options.value(QStringLiteral("fahrenheit"), false).toBool();
    m_opts.stampText   = options.value(QStringLiteral("stampText")).toString();
    m_opts.geoText     = options.value(QStringLiteral("geoText")).toString();
    m_opts.elementRotation = options.value(QStringLiteral("elementRotation"), 0).toInt();

    // Compose one frame up front to fix the video's dimensions. currentImage()
    // is valid whenever the camera is streaming; if it isn't, there is nothing
    // to record yet.
    const QImage probe = snapshot::compose(m_source, m_opts);
    if (probe.isNull()) {
        setError(tr("No frame to record yet."));
        return false;
    }
    m_width = probe.width();
    m_height = probe.height();

    m_lastError.clear();
    m_frameCount = 0;
    m_bytesWritten = 0;
    m_firstFrameNs = 0;
    m_lastFrameNs = -1;

    // Build by hand for a typed appsrc pointer and precise per-element errors.
    m_pipeline = gst_pipeline_new("ircam-recorder");
    m_appsrc = gst_element_factory_make("appsrc", "src");
    GstElement *conv = gst_element_factory_make("videoconvert", "conv");
    GstElement *enc  = gst_element_factory_make("vp8enc", "enc");
    GstElement *mux  = gst_element_factory_make("webmmux", "mux");
    GstElement *sink = gst_element_factory_make("filesink", "sink");

    if (!m_pipeline || !m_appsrc || !conv || !enc || !mux || !sink) {
        setError(tr("Video recording is unavailable: a required GStreamer "
                    "element is missing (appsrc/videoconvert/vp8enc/webmmux/filesink)."));
        teardown();
        return false;
    }

    // BGRx matches QImage::Format_RGB32 in memory on little-endian; videoconvert
    // turns it into the I420 vp8enc wants.
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "format",    G_TYPE_STRING, "BGRx",
                                        "width",     G_TYPE_INT, m_width,
                                        "height",    G_TYPE_INT, m_height,
                                        "framerate", GST_TYPE_FRACTION, NOMINAL_FPS, 1,
                                        NULL);
    g_object_set(G_OBJECT(m_appsrc),
                 "caps", caps,
                 "format", GST_FORMAT_TIME,
                 "is-live", TRUE,
                 "do-timestamp", FALSE,
                 "block", FALSE,
                 "max-bytes", (guint64)(32 * 1024 * 1024),
                 NULL);
    gst_caps_unref(caps);

    // deadline=1 (realtime) + cpu-used=8 keep the software encoder ahead of a
    // 25 fps stream; target-bitrate is the compression knob the slider drives.
    // A keyframe at least every 2 s keeps seeking usable without bloating size.
    g_object_set(G_OBJECT(enc),
                 "target-bitrate", m_bitrate,
                 "deadline", (gint64)1,
                 "cpu-used", 8,
                 "keyframe-max-dist", 2 * NOMINAL_FPS,
                 "end-usage", 1,          // 1 = CBR: honour target-bitrate
                 NULL);

    g_object_set(G_OBJECT(sink), "location", path.toUtf8().constData(), NULL);

    gst_bin_add_many(GST_BIN(m_pipeline), m_appsrc, conv, enc, mux, sink, NULL);
    if (!gst_element_link_many(m_appsrc, conv, enc, mux, sink, NULL)) {
        setError(tr("Could not build the recording pipeline."));
        teardown();
        return false;
    }

    const GstStateChangeReturn ret =
            gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        setError(tr("Could not start recording to %1.").arg(path));
        teardown();
        return false;
    }

    m_outputPath = path;
    m_clock.start();
    emit recordingChanged();
    emit progress();
    // Frames arrive via onSourceFrame(); the first one starts the clip at t=0.
    return true;
}

void VideoRecorder::onSourceFrame()
{
    if (!m_pipeline || !m_appsrc || !m_source)
        return;

    QImage image = snapshot::compose(m_source, m_opts);
    if (image.isNull())
        return;
    if (image.format() != QImage::Format_RGB32)
        image = image.convertToFormat(QImage::Format_RGB32);

    // A mid-recording orientation change would resize the frame; VP8 needs a
    // constant resolution, so ignore frames that no longer match the caps.
    if (image.width() != m_width || image.height() != m_height)
        return;

    const qint64 nowNs = m_clock.nsecsElapsed();
    if (m_frameCount == 0)
        m_firstFrameNs = nowNs;
    const qint64 ptsNs = nowNs - m_firstFrameNs;

    // Copy the pixels into a tightly-packed GstBuffer (row stride == width*4 for
    // RGB32, so a single fill is safe). GStreamer owns it until the encoder is
    // done; the QImage may be released the moment we return.
    const gsize size = gsize(image.width()) * gsize(image.height()) * 4;
    GstBuffer *buf = gst_buffer_new_allocate(NULL, size, NULL);
    if (!buf)
        return;
    GstMapInfo map;
    if (!gst_buffer_map(buf, &map, GST_MAP_WRITE)) {
        gst_buffer_unref(buf);
        return;
    }
    const int rowBytes = image.width() * 4;
    for (int y = 0; y < image.height(); ++y)
        memcpy(map.data + gsize(y) * rowBytes, image.constScanLine(y), rowBytes);
    gst_buffer_unmap(buf, &map);

    GST_BUFFER_PTS(buf) = GstClockTime(ptsNs);
    GST_BUFFER_DTS(buf) = GstClockTime(ptsNs);
    GST_BUFFER_DURATION(buf) =
            (m_lastFrameNs >= 0) ? GstClockTime(ptsNs - m_lastFrameNs)
                                 : GstClockTime(GST_SECOND / NOMINAL_FPS);

    const GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(m_appsrc), buf);
    if (ret != GST_FLOW_OK) {
        qWarning() << "ircam: recorder: push_buffer returned" << ret;
        return;
    }

    m_lastFrameNs = ptsNs;
    ++m_frameCount;
    m_bytesWritten = QFileInfo(m_outputPath).size();
    emit progress();

    pollBus();
}

void VideoRecorder::pollBus()
{
    if (!m_pipeline)
        return;

    GstBus *bus = gst_element_get_bus(m_pipeline);
    if (!bus)
        return;

    GstMessage *msg = gst_bus_pop_filtered(bus, GST_MESSAGE_ERROR);
    gst_object_unref(bus);
    if (!msg)
        return;

    GError *err = 0;
    gchar *dbg = 0;
    gst_message_parse_error(msg, &err, &dbg);
    qWarning() << "ircam: recorder pipeline error:"
               << (err ? err->message : "unknown")
               << "| debug:" << (dbg ? dbg : "(none)");
    setError(tr("Recording failed: %1")
             .arg(QString::fromUtf8(err ? err->message : "unknown")));
    if (err) g_error_free(err);
    g_free(dbg);
    gst_message_unref(msg);

    teardown();
    emit recordingChanged();
    emit progress();
}

void VideoRecorder::stop()
{
    if (!m_pipeline)
        return;

    const QString path = m_outputPath;

    if (m_appsrc) {
        gst_app_src_end_of_stream(GST_APP_SRC(m_appsrc));

        GstBus *bus = gst_element_get_bus(m_pipeline);
        if (bus) {
            GstMessage *msg = gst_bus_timed_pop_filtered(
                        bus, GstClockTime(EOS_TIMEOUT_MS) * GST_MSECOND,
                        GstMessageType(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
            if (!msg) {
                setError(tr("Timed out finalising the video file — it may be "
                            "incomplete."));
            } else {
                if (GST_MESSAGE_TYPE(msg) == GST_MESSAGE_ERROR) {
                    GError *err = 0;
                    gchar *dbg = 0;
                    gst_message_parse_error(msg, &err, &dbg);
                    qWarning() << "ircam: recorder pipeline error:"
                               << (err ? err->message : "unknown")
                               << "| debug:" << (dbg ? dbg : "(none)");
                    setError(tr("Recording failed: %1")
                             .arg(QString::fromUtf8(err ? err->message : "unknown")));
                    if (err) g_error_free(err);
                    g_free(dbg);
                }
                gst_message_unref(msg);
            }
            gst_object_unref(bus);
        }
    }

    teardown();
    emit recordingChanged();
    emit progress();

    if (!path.isEmpty() && QFileInfo(path).size() > 0)
        emit recordingFinished(path);
}

void VideoRecorder::teardown()
{
    if (m_pipeline) {
        gst_element_set_state(m_pipeline, GST_STATE_NULL);
        gst_object_unref(GST_OBJECT(m_pipeline));
    }
    m_pipeline = 0;
    m_appsrc = 0;
    m_outputPath.clear();
}
