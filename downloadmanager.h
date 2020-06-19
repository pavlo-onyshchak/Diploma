#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QtNetwork>
#include <QStandardItemModel>

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

class DownloadManager: public QObject
{
    Q_OBJECT

    QNetworkAccessManager m_manager;
    QVector<QNetworkReply *> m_currentDownloads;
    QStringList m_urls;
    QString m_path;

    int m_numberOfUrls;
    int m_value;
    int m_progress;

public:
    DownloadManager();

    void append(const QString &url);
    void append(const QStringList &urlList);
    void doDownload(const QUrl &url);

    static QString saveFileName(const QUrl &url, const QString &dir);
    static bool isHttpRedirect(QNetworkReply *reply);

    void setPath(const QString& path);
    bool saveToDisk(const QString &filename, QIODevice *data);

  signals:
    void downloadMessage(QList<QStandardItem *>);
    void progressChanged(int);
    void finished(bool);

public slots:
    void execute();
    void downloadFinished(QNetworkReply *reply);
    void sslErrors(const QList<QSslError> &errors);
};

#endif // DOWNLOADMANAGER_H
