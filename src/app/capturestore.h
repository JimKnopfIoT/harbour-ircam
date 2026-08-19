// capturestore.h — snapshot storage (~/Pictures/IrCam, JPEG + optional °C CSV) and its list model. GPLv3+, © JimKnopfIoT.
#ifndef CAPTURESTORE_H
#define CAPTURESTORE_H

#include <QAbstractListModel>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QVariantMap>

class ThermalCamera;

class CaptureStore : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)
    Q_PROPERTY(QString pictureDir READ pictureDir CONSTANT)
    Q_PROPERTY(QString lastCapturePath READ lastCapturePath NOTIFY lastCaptureChanged)

public:
    enum Roles {
        PathRole = Qt::UserRole + 1,
        FileNameRole,
        TimestampRole,
        SizeBytesRole,
        SizeTextRole,
        HasCsvRole,
        UrlRole,
        IsVideoRole
    };

    explicit CaptureStore(QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

    int count() const { return m_items.count(); }
    QString pictureDir() const { return m_pictureDir; }
    QString lastCapturePath() const { return m_lastCapturePath; }

    // Render+write the current picture; options keys: rotation, mirrored, showMarkers,
    // showCenter, showBar, showReadout, fahrenheit, stampText, geoText, saveCsv, points.
    // Returns the JPEG path or "" on failure (error() emitted).
    Q_INVOKABLE QString saveSnapshot(ThermalCamera *camera, const QVariantMap &options);

    Q_INVOKABLE bool remove(int index);
    // Rename keeping the extension; a CSV sidecar is renamed along.
    Q_INVOKABLE bool rename(int index, const QString &newBaseName);
    Q_INVOKABLE QString baseName(int index) const;
    Q_INVOKABLE QString pathAt(int index) const;
    // {path, url, fileName, sizeText, hasCsv} for row `index`.
    Q_INVOKABLE QVariantMap itemAt(int index) const;
    Q_INVOKABLE void refresh();
    // A fresh timestamped .webm path in the pictures dir (dir is created).
    Q_INVOKABLE QString newVideoPath();

signals:
    void countChanged();
    void lastCaptureChanged();
    void error(const QString &message);
    void captured(const QString &path);

private:
    struct Item {
        QString path;
        QString fileName;
        QDateTime timestamp;
        qint64 sizeBytes;
        bool hasCsv;
        bool isVideo;
    };

    QString makePath(const QString &dir, const QString &extension) const;
    bool ensureDir(const QString &dir);
    static QString humanSize(qint64 bytes);
    static QString csvPathFor(const QString &jpgPath);
    void insertItem(const QString &path, bool announce);

    QString m_pictureDir;
    QString m_lastCapturePath;
    QList<Item> m_items;   // newest first
};

#endif // CAPTURESTORE_H
