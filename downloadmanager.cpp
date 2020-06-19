#include "downloadmanager.h"

#include <iostream>

DownloadManager::DownloadManager()
{
  QObject::connect(&m_manager, SIGNAL(finished(QNetworkReply*)),
                   SLOT(downloadFinished(QNetworkReply*)));
}

void DownloadManager::append(const QString& url)
{
  m_urls.push_back(url);
}

void DownloadManager::append(const QStringList& urlList)
{
  for(const auto& url : urlList)
  {
    m_urls.push_back(url);
  }
}

void DownloadManager::doDownload(const QUrl &url)
{
  QNetworkRequest request(url);
  QNetworkReply  *reply = m_manager.get(request);

#if QT_CONFIG(ssl)
  QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                   SLOT(sslErrors(QList<QSslError>)));
#endif

  m_currentDownloads.append(reply);
}

QString DownloadManager::saveFileName(const QUrl &url, const QString &dir)
{
  QString path     = url.path();
  QString baseName = QFileInfo(path).fileName();

  if (baseName.isEmpty())
    baseName = "download_media";

  if (QFile::exists(dir + baseName))
  {
    // already exists
    baseName += ".exists";
  }

  return baseName;
}

bool DownloadManager::saveToDisk(const QString &fileName, QIODevice *data)
{
  QFile file(m_path + fileName);

  if (!file.open(QIODevice::WriteOnly))
  {
    std::cerr << "Could not open " << qPrintable(fileName) << " for writing: " << qPrintable(file.errorString()) << std::endl;
    return false;
  }

  file.write(data->readAll());
  file.close();

  return true;
}

bool DownloadManager::isHttpRedirect(QNetworkReply *reply)
{
  int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  return statusCode == 301 || statusCode == 302 || statusCode == 303
      || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::setPath(const QString &path)
{
  m_path = path;
}

void DownloadManager::execute()
{
  m_numberOfUrls = m_urls.size();
  m_value        = 100 / m_numberOfUrls;
  m_progress     = m_value;

  for (const auto& strUrl : m_urls)
  {
    QUrl url = QUrl::fromEncoded(strUrl.toLocal8Bit());
    doDownload(url);
  }
}

void DownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#if QT_CONFIG(ssl)
  for (const QSslError &error : sslErrors)
  {
    std::cerr << "SSL error: " << qPrintable(error.errorString()) << std::endl;
  }
#else
  Q_UNUSED(sslErrors);
#endif
}

void DownloadManager::downloadFinished(QNetworkReply *reply)
{
  QUrl url = reply->url();

  if (reply->error())
  {
    QList<QStandardItem *> items;
    items.append(new QStandardItem("-"));
    items.append(new QStandardItem("-"));
    items.append(new QStandardItem("failed, network error"));
    items.append(new QStandardItem("-"));

    emit downloadMessage(items);
  }
  else
  {
    if (isHttpRedirect(reply))
    {
      QList<QStandardItem *> items;
      items.append(new QStandardItem("-"));
      items.append(new QStandardItem("-"));
      items.append(new QStandardItem("failed, request redirected"));
      items.append(new QStandardItem("-"));

      emit downloadMessage(items);
    }
    else
    {

      QString fileName  = saveFileName(url, m_path);
      QString extension = QFileInfo(url.path()).suffix();

      if (fileName.contains(".exists"))
      {
        fileName.remove(".exists");

        QList<QStandardItem *> items;
        items.append(new QStandardItem(fileName));
        items.append(new QStandardItem(extension));
        items.append(new QStandardItem("already exists, skipped"));
        items.append(new QStandardItem(m_path));

        emit downloadMessage(items);
      }
      else if (saveToDisk(fileName, reply))
      {
        QList<QStandardItem *> items;
        items.append(new QStandardItem(fileName));
        items.append(new QStandardItem(extension));
        items.append(new QStandardItem("succeeded"));
        items.append(new QStandardItem(m_path));

        emit downloadMessage(items);
      }
      else
      {
        QList<QStandardItem *> items;
        items.append(new QStandardItem(fileName));
        items.append(new QStandardItem(extension));
        items.append(new QStandardItem("failed, cannot write data on disk"));
        items.append(new QStandardItem(m_path));

        emit downloadMessage(items);
      }
    }
  }

  emit progressChanged(m_progress);
  m_progress += m_value;

  m_currentDownloads.removeAll(reply);
  reply->deleteLater();

  if (m_currentDownloads.isEmpty())
  {
    // all downloads finished
    m_urls.clear();

    emit progressChanged(100);
    emit finished(true);
  }
}
