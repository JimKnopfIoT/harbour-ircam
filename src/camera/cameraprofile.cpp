// cameraprofile.cpp — see cameraprofile.h. GPLv3+, © JimKnopfIoT.
#include "cameraprofile.h"

#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QStandardPaths>
#include <linux/videodev2.h>

QVariantMap CameraProfile::toMap() const
{
    QVariantMap m;
    m["name"] = name;
    m["vid"] = vid;
    m["pid"] = pid;
    m["streamW"] = streamW;
    m["streamH"] = streamH;
    m["pixelFormat"] = double(pixelFormat);
    m["fourcc"] = cameraprofiles::fourccName(pixelFormat);
    m["imageW"] = imageW;
    m["imageH"] = imageH;
    m["hasTemp"] = hasTemp;
    m["tempMode"] = tempMode;
    m["tempRowOffset"] = tempRowOffset;
    m["tempScale"] = tempScale;
    m["tempOffsetC"] = tempOffsetC;
    m["builtin"] = builtin;
    return m;
}

CameraProfile CameraProfile::fromMap(const QVariantMap &m)
{
    CameraProfile p;
    p.name = m.value("name").toString();
    p.vid = m.value("vid", -1).toInt();
    p.pid = m.value("pid", -1).toInt();
    p.streamW = m.value("streamW").toInt();
    p.streamH = m.value("streamH").toInt();
    p.pixelFormat = unsigned(m.value("pixelFormat", double(V4L2_PIX_FMT_YUYV)).toDouble());
    p.imageW = m.value("imageW").toInt();
    p.imageH = m.value("imageH").toInt();
    p.hasTemp = m.value("hasTemp", false).toBool();
    p.tempMode = m.value("tempMode", QStringLiteral("none")).toString();
    p.tempRowOffset = m.value("tempRowOffset").toInt();
    p.tempScale = m.value("tempScale").toDouble();
    p.tempOffsetC = m.value("tempOffsetC").toDouble();
    p.builtin = m.value("builtin", false).toBool();
    return p;
}

namespace cameraprofiles {

QVector<CameraProfile> builtins()
{
    QVector<CameraProfile> v;
    CameraProfile p2;
    p2.name = QStringLiteral("InfiRay P2 Pro / Topdon TC001");
    p2.vid = 0x0bda; p2.pid = 0x5830;
    p2.streamW = 256; p2.streamH = 384; p2.pixelFormat = V4L2_PIX_FMT_YUYV;
    p2.imageW = 256; p2.imageH = 192;
    p2.hasTemp = true; p2.tempMode = QStringLiteral("linear"); p2.tempRowOffset = 192;
    p2.tempScale = 1.0 / 64.0; p2.tempOffsetC = -273.15;
    p2.builtin = true;
    v.append(p2);
    return v;
}

QString userDbPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dir + QStringLiteral("/cameras.json");
}

QVector<CameraProfile> loadUserDb()
{
    QVector<CameraProfile> v;
    QFile f(userDbPath());
    if (!f.open(QIODevice::ReadOnly))
        return v;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    if (!doc.isArray())
        return v;
    const QJsonArray a = doc.array();
    for (int i = 0; i < a.size(); ++i) {
        CameraProfile p = CameraProfile::fromMap(a.at(i).toObject().toVariantMap());
        p.builtin = false;
        if (p.valid())
            v.append(p);
    }
    return v;
}

bool appendUserDb(const CameraProfile &p)
{
    if (!p.valid())
        return false;
    QVector<CameraProfile> v = loadUserDb();
    for (int i = 0; i < v.size(); ++i) {           // replace same vid:pid
        if (v[i].vid == p.vid && v[i].pid == p.pid) {
            v.removeAt(i);
            break;
        }
    }
    v.append(p);
    QJsonArray a;
    for (int i = 0; i < v.size(); ++i)
        a.append(QJsonObject::fromVariantMap(v[i].toMap()));
    const QString path = userDbPath();
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    f.write(QJsonDocument(a).toJson(QJsonDocument::Indented));
    return true;
}

QString fourccName(unsigned f)
{
    if (f == 0)
        return QString();
    char c[5] = { char(f & 0xff), char((f >> 8) & 0xff),
                  char((f >> 16) & 0xff), char((f >> 24) & 0xff), 0 };
    return QString::fromLatin1(c).trimmed();
}

} // namespace cameraprofiles
