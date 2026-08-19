// capturestore.cpp — see capturestore.h. GPLv3+, © JimKnopfIoT.
#include "capturestore.h"
#include "snapshotcomposer.h"
#include "thermalcamera.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <QVariantList>
#include <vector>

static const char *SUBDIR       = "IrCam";
static const char *FILE_PREFIX  = "IrCam_";
static const char *STAMP_FORMAT = "yyyyMMdd-HHmmss";

CaptureStore::CaptureStore(QObject *parent)
    : QAbstractListModel(parent)
{
    m_pictureDir = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)
                 + QLatin1Char('/') + QLatin1String(SUBDIR);
    refresh();
}

int CaptureStore::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.count();
}

QHash<int, QByteArray> CaptureStore::roleNames() const
{
    QHash<int, QByteArray> r;
    r[PathRole]      = "path";
    r[FileNameRole]  = "fileName";
    r[TimestampRole] = "timestamp";
    r[SizeBytesRole] = "sizeBytes";
    r[SizeTextRole]  = "sizeText";
    r[HasCsvRole]    = "hasCsv";
    r[UrlRole]       = "url";
    r[IsVideoRole]   = "isVideo";
    return r;
}

QVariant CaptureStore::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.count())
        return QVariant();
    const Item &it = m_items.at(index.row());
    switch (role) {
    case PathRole:      return it.path;
    case FileNameRole:  return it.fileName;
    case TimestampRole: return it.timestamp;
    case SizeBytesRole: return it.sizeBytes;
    case SizeTextRole:  return humanSize(it.sizeBytes);
    case HasCsvRole:    return it.hasCsv;
    case UrlRole:       return QUrl::fromLocalFile(it.path).toString();
    case IsVideoRole:   return it.isVideo;
    }
    return QVariant();
}

QString CaptureStore::humanSize(qint64 bytes)
{
    if (bytes < 1024)
        return tr("%1 B").arg(bytes);
    if (bytes < 1024 * 1024)
        return tr("%1 kB").arg(qreal(bytes) / 1024.0, 0, 'f', 0);
    return tr("%1 MB").arg(qreal(bytes) / (1024.0 * 1024.0), 0, 'f', 1);
}

QString CaptureStore::csvPathFor(const QString &jpgPath)
{
    const QFileInfo fi(jpgPath);
    return fi.absolutePath() + QLatin1Char('/') + fi.completeBaseName() + QLatin1String(".csv");
}

bool CaptureStore::ensureDir(const QString &dir)
{
    if (QDir(dir).exists() || QDir().mkpath(dir))
        return true;
    emit error(tr("Cannot create folder %1").arg(dir));
    return false;
}

QString CaptureStore::newVideoPath()
{
    if (!ensureDir(m_pictureDir)) {
        emit error(tr("Cannot create %1.").arg(m_pictureDir));
        return QString();
    }
    return makePath(m_pictureDir, QLatin1String("webm"));
}

QString CaptureStore::makePath(const QString &dir, const QString &extension) const
{
    const QString stamp = QDateTime::currentDateTime().toString(QLatin1String(STAMP_FORMAT));
    const QString base = dir + QLatin1Char('/') + QLatin1String(FILE_PREFIX) + stamp;
    QString candidate = base + QLatin1Char('.') + extension;
    int n = 2;
    while (QFile::exists(candidate) && n < 1000) {
        candidate = base + QLatin1Char('-') + QString::number(n) + QLatin1Char('.') + extension;
        ++n;
    }
    return candidate;
}

QString CaptureStore::saveSnapshot(ThermalCamera *camera, const QVariantMap &options)
{
    if (!camera) {
        emit error(tr("No camera."));
        return QString();
    }
    snapshot::Options opt;
    opt.rotation    = options.value(QStringLiteral("rotation"), 0).toInt();
    opt.mirrored    = options.value(QStringLiteral("mirrored"), false).toBool();
    opt.showMarkers = options.value(QStringLiteral("showMarkers"), false).toBool();
    opt.showCenter  = options.value(QStringLiteral("showCenter"), false).toBool();
    opt.showBar     = options.value(QStringLiteral("showBar"), false).toBool();
    opt.showReadout = options.value(QStringLiteral("showReadout"), false).toBool();
    opt.fahrenheit  = options.value(QStringLiteral("fahrenheit"), false).toBool();
    opt.stampText   = options.value(QStringLiteral("stampText")).toString();
    opt.geoText     = options.value(QStringLiteral("geoText")).toString();
    opt.elementRotation = options.value(QStringLiteral("elementRotation"), 0).toInt();
    const QVariantList pts = options.value(QStringLiteral("points")).toList();
    for (int i = 0; i < pts.count(); ++i) {
        const QVariantMap m = pts.at(i).toMap();
        opt.points.append(QPointF(m.value(QStringLiteral("nx")).toDouble(),
                                  m.value(QStringLiteral("ny")).toDouble()));
    }

    const QImage image = snapshot::compose(camera, opt);
    if (image.isNull()) {
        emit error(tr("No frame to save yet."));
        return QString();
    }
    if (!ensureDir(m_pictureDir))
        return QString();

    const QString path = makePath(m_pictureDir, QLatin1String("jpg"));
    if (!image.save(path, "JPEG", 95)) {
        emit error(tr("Cannot write %1").arg(path));
        return QString();
    }

    if (options.value(QStringLiteral("saveCsv"), false).toBool()) {
        int tw = 0, th = 0;
        const std::vector<float> temps = camera->currentTemps(&tw, &th);
        if (!temps.empty() && tw > 0 && th > 0) {
            // Orient the grid like the picture (same rules as snapshot::mapToImage).
            const int rot = ((opt.rotation % 360) + 360) % 360;
            const bool swap = (rot == 90 || rot == 270);
            const int ow = swap ? th : tw;
            const int oh = swap ? tw : th;
            std::vector<float> out(size_t(ow) * oh);
            for (int y = 0; y < th; ++y) {
                for (int x = 0; x < tw; ++x) {
                    const QPointF d = snapshot::mapToImage((x + 0.5) / tw, (y + 0.5) / th,
                                                           QSize(ow, oh), rot, opt.mirrored);
                    const int ox = qBound(0, int(d.x()), ow - 1);
                    const int oy = qBound(0, int(d.y()), oh - 1);
                    out[size_t(oy) * ow + ox] = temps[size_t(y) * tw + x];
                }
            }
            QFile f(csvPathFor(path));
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream ts(&f);
                for (int y = 0; y < oh; ++y) {
                    for (int x = 0; x < ow; ++x) {
                        if (x) ts << ',';
                        ts << QString::number(out[size_t(y) * ow + x], 'f', 2);
                    }
                    ts << '\n';
                }
            } else {
                emit error(tr("Cannot write %1").arg(f.fileName()));
            }
        }
    }

    insertItem(path, true);
    return path;
}

void CaptureStore::insertItem(const QString &path, bool announce)
{
    const QFileInfo fi(path);
    Item it;
    it.path = fi.absoluteFilePath();
    it.fileName = fi.fileName();
    it.timestamp = fi.lastModified();
    it.sizeBytes = fi.size();
    it.hasCsv = QFile::exists(csvPathFor(it.path));

    beginInsertRows(QModelIndex(), 0, 0);
    m_items.prepend(it);
    endInsertRows();
    emit countChanged();

    m_lastCapturePath = it.path;
    emit lastCaptureChanged();
    if (announce)
        emit captured(it.path);
}

bool CaptureStore::remove(int index)
{
    if (index < 0 || index >= m_items.count())
        return false;
    const QString path = m_items.at(index).path;
    if (QFile::exists(path) && !QFile::remove(path)) {
        emit error(tr("Cannot delete %1").arg(QFileInfo(path).fileName()));
        return false;
    }
    const QString csv = csvPathFor(path);
    if (QFile::exists(csv))
        QFile::remove(csv);

    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();
    emit countChanged();

    if (m_lastCapturePath == path) {
        m_lastCapturePath = m_items.isEmpty() ? QString() : m_items.first().path;
        emit lastCaptureChanged();
    }
    return true;
}

QString CaptureStore::baseName(int index) const
{
    if (index < 0 || index >= m_items.count())
        return QString();
    return QFileInfo(m_items.at(index).path).completeBaseName();
}

QString CaptureStore::pathAt(int index) const
{
    if (index < 0 || index >= m_items.count())
        return QString();
    return m_items.at(index).path;
}

QVariantMap CaptureStore::itemAt(int index) const
{
    QVariantMap m;
    if (index < 0 || index >= m_items.count())
        return m;
    const Item &it = m_items.at(index);
    m["path"] = it.path;
    m["url"] = QUrl::fromLocalFile(it.path).toString();
    m["fileName"] = it.fileName;
    m["sizeText"] = humanSize(it.sizeBytes);
    m["hasCsv"] = it.hasCsv;
    m["isVideo"] = it.isVideo;
    return m;
}

bool CaptureStore::rename(int index, const QString &newBaseName)
{
    if (index < 0 || index >= m_items.count())
        return false;
    const QString wanted = newBaseName.trimmed();
    if (wanted.isEmpty()) {
        emit error(tr("The name cannot be empty."));
        return false;
    }
    if (wanted.contains(QLatin1Char('/')) || wanted.startsWith(QLatin1Char('.'))) {
        emit error(tr("A name cannot contain “/” or start with a dot."));
        return false;
    }
    const QFileInfo fi(m_items.at(index).path);
    const QString target = fi.absolutePath() + QLatin1Char('/') + wanted + QLatin1Char('.') + fi.suffix();
    if (target == fi.absoluteFilePath())
        return true;
    if (QFile::exists(target)) {
        emit error(tr("“%1” already exists.").arg(QFileInfo(target).fileName()));
        return false;
    }
    if (!QFile::rename(fi.absoluteFilePath(), target)) {
        emit error(tr("Could not rename “%1”.").arg(fi.fileName()));
        return false;
    }
    const QString oldCsv = csvPathFor(fi.absoluteFilePath());
    if (QFile::exists(oldCsv))
        QFile::rename(oldCsv, csvPathFor(target));

    Item &it = m_items[index];
    it.path = target;
    it.fileName = QFileInfo(target).fileName();
    const QModelIndex mi = this->index(index, 0);
    QVector<int> roles;
    roles << PathRole << FileNameRole << UrlRole;
    emit dataChanged(mi, mi, roles);

    if (m_lastCapturePath == fi.absoluteFilePath()) {
        m_lastCapturePath = target;
        emit lastCaptureChanged();
    }
    return true;
}

void CaptureStore::refresh()
{
    beginResetModel();
    m_items.clear();
    QStringList filter;
    filter << QLatin1String("*.jpg") << QLatin1String("*.jpeg") << QLatin1String("*.webm");
    const QList<QFileInfo> found = QDir(m_pictureDir).entryInfoList(filter, QDir::Files);
    for (int i = 0; i < found.count(); ++i) {
        const QFileInfo &fi = found.at(i);
        Item it;
        it.path = fi.absoluteFilePath();
        it.fileName = fi.fileName();
        it.timestamp = fi.lastModified();
        it.sizeBytes = fi.size();
        it.isVideo = (fi.suffix().toLower() == QLatin1String("webm"));
        it.hasCsv = !it.isVideo && QFile::exists(csvPathFor(it.path));
        int pos = 0;
        while (pos < m_items.count() && m_items.at(pos).timestamp > it.timestamp)
            ++pos;
        m_items.insert(pos, it);
    }
    endResetModel();
    emit countChanged();
    const QString newest = m_items.isEmpty() ? QString() : m_items.first().path;
    if (newest != m_lastCapturePath) {
        m_lastCapturePath = newest;
        emit lastCaptureChanged();
    }
}
