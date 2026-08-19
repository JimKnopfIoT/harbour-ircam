// cameraprofile.h — describes one supported thermal camera. GPLv3+, © JimKnopfIoT.
//
// Everything the capture path needs per camera lives here, so adding a camera
// is a data row, not code. Built-in rows ship with the app; a user DB (JSON in
// the config dir) adds more, keyed by vid:pid.
#ifndef CAMERAPROFILE_H
#define CAMERAPROFILE_H

#include <QString>
#include <QVariantMap>
#include <QVector>

struct CameraProfile {
    QString name;
    int vid, pid;             // USB IDs; -1 = wildcard (match by stream format only)
    int streamW, streamH;     // V4L2 format requested
    unsigned pixelFormat;     // V4L2_PIX_FMT_* (fourcc)
    int imageW, imageH;       // visible picture region (imageH == streamH if no temp plane)
    bool hasTemp;             // radiometric temperature available
    QString tempMode;         // "none" | "linear" (raw16*scale+offset) | "ht301" (calibrated)
    int tempRowOffset;        // first row of the temperature plane (linear mode)
    double tempScale;         // °C = raw16 * tempScale + tempOffsetC   (linear mode)
    double tempOffsetC;
    bool builtin;             // shipped vs. user-added

    CameraProfile()
        : vid(-1), pid(-1), streamW(0), streamH(0), pixelFormat(0)
        , imageW(0), imageH(0), hasTemp(false), tempMode(QStringLiteral("none"))
        , tempRowOffset(0), tempScale(0), tempOffsetC(0), builtin(false) {}

    bool valid() const { return streamW > 0 && streamH > 0 && pixelFormat != 0; }
    QVariantMap toMap() const;                 // for QML display / export
    static CameraProfile fromMap(const QVariantMap &m);
};

namespace cameraprofiles {

// The shipped table (P2 Pro family first).
QVector<CameraProfile> builtins();

// User DB: JSON array under QStandardPaths::AppDataLocation/cameras.json.
QString userDbPath();
QVector<CameraProfile> loadUserDb();
bool appendUserDb(const CameraProfile &p);     // adds/replaces by vid:pid

// Pretty fourcc, e.g. "YUYV".
QString fourccName(unsigned f);

} // namespace cameraprofiles

#endif // CAMERAPROFILE_H
