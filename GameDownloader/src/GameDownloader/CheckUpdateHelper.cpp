/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/CheckUpdateHelper.h>

#include <UpdateSystem/Downloader/downloadmanager>
#include <UpdateSystem/Downloader/DynamicRetryTimeout>
#include <UpdateSystem/Downloader/RetryFileDownloader>
#include <UpdateSystem/Downloader/MultiFileDownloader>
#include <UpdateSystem/Downloader/MultiFileDownloaderWithExtracter>
#include <UpdateSystem/Extractor/SevenZipExtractor>
#include <UpdateSystem/Hasher/Md5FileHasher>

#include <Settings/Settings>
#include <Core/Service>

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkRequest>

using namespace GGS::Downloader;
using namespace GGS::Extractor;
using namespace GGS::Core;
using namespace GGS::Hasher;

namespace GGS {
  namespace GameDownloader {
    CheckUpdateHelper::CheckUpdateHelper(QObject *parent)
      : QObject(parent)
      , _headRequestRetryCount(0)
      , _maxHeadRequestRetryCount(3)
    {
      this->_manager = new QNetworkAccessManager(this);
      this->_extractor = new SevenZipExtactor(this);
    }

    CheckUpdateHelper::~CheckUpdateHelper()
    {
    }

    QUrl CheckUpdateHelper::getTorrentUrlWithArchiveExtension()
    {
      QUrl url = this->_service->torrentUrlWithArea();
      QString fileName = QString("%1.torrent.7z").arg(this->_service->id());
      url = url.resolved(QUrl(fileName));
      return url;
    }

    QString CheckUpdateHelper::getTorrentUrlWithoutArchiveExtension()
    {
      QUrl url = this->_service->torrentUrlWithArea();
      QString fileName = QString("%1.torrent").arg(this->_service->id());
      url = url.resolved(QUrl(fileName));
      return url.toString();
    }

    QString CheckUpdateHelper::getTorrentPath(const GGS::Core::Service *service)
    {
      return QString("%1/%2/%3.torrent")
        .arg(service->torrentFilePath())
        .arg(service->areaString())
        .arg(service->id());
    }

    void CheckUpdateHelper::slotError(QNetworkReply::NetworkError error)
    {
      QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
      if (!reply)
        return;

      CRITICAL_LOG << "Error on head request in check update. Error: " << error << " service " << this->_service->id();
      reply->deleteLater();

      this->_headRequestRetryCount++;
      if (this->_headRequestRetryCount < this->_maxHeadRequestRetryCount)
        this->startCheck(this->_service, this->_checkUpdateType);
      else
        emit this->error(this->_service);
    }

    void CheckUpdateHelper::slotReplyDownloadFinished()
    {
      QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
      if (!reply)
        return;

      reply->deleteLater();

      int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      // Http codes defined by rfc: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
      // 200 OK
      // 304 Not Modified
      if (httpCode != 304 && httpCode != 200) {
        CRITICAL_LOG << "Http error" << httpCode;
        emit this->error(this->_service);
        return;
      }

      this->_lastModified = QString::fromAscii(reply->rawHeader(QByteArray("Last-Modified")));

      QString torrentPath = CheckUpdateHelper::getTorrentPath(this->_service);
      Md5FileHasher hasher;

      bool isDownloadRequired = this->_checkUpdateType == ForceDownloadTorrent 
        || httpCode == 200
        || !QFile::exists(torrentPath)
        || this->torrentHash() != hasher.getFileHash(torrentPath);

      if (isDownloadRequired) {
        this->startDownloadTorrent();
        return;
      }

      emit this->checkUpdateProgressChanged(this->_service->id(), 100);
      emit this->result(this->_service, false);
    }

    void CheckUpdateHelper::startCheck(const GGS::Core::Service *service, CheckUpdateType checkUpdateType)
    {
      Q_ASSERT(service);
      this->_checkUpdateType = checkUpdateType;
      this->_service = service;
      emit this->checkUpdateProgressChanged(service->id(), 0);
      QNetworkRequest request(this->getTorrentUrlWithArchiveExtension());

      QString oldLastModifed = this->loadLastModifiedDate();
      request.setRawHeader("If-Modified-Since", oldLastModifed.toAscii());

      QNetworkReply *reply = this->_manager->head(request);
      connect(reply, SIGNAL(finished()), this, SLOT(slotReplyDownloadFinished()));
      connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError))); 
    }

    void CheckUpdateHelper::startDownloadTorrent()
    {
      DynamicRetryTimeout* dynamicRetryTimeout = new DynamicRetryTimeout(this);
      *dynamicRetryTimeout << 5000 << 10000 << 15000 << 30000;

      RetryFileDownloader* retryDownloader = new RetryFileDownloader(this);
      retryDownloader->setMaxRetry(5);
      retryDownloader->setTimeout(dynamicRetryTimeout);

      DownloadManager* downloader = new DownloadManager(this);     
      retryDownloader->setDownloader(downloader);

      MultiFileDownloader* multi = new MultiFileDownloader(this);
      multi->setDownloader(retryDownloader);

      MultiFileDownloaderWithExtracter* multiExtractor = new MultiFileDownloaderWithExtracter(this);
      multiExtractor->setExtractor(this->_extractor);
      multiExtractor->setMultiDownloader(multi);
      multiExtractor->setResultCallback(this);

      QString url = this->getTorrentUrlWithoutArchiveExtension();
      QString path = CheckUpdateHelper::getTorrentPath(this->_service);
      multiExtractor->addFile(url, path);
      multiExtractor->start();
    }

    void CheckUpdateHelper::fileDownloaded(const QString& filePath)
    {
      // nothing to do here, but must exist because of interface
    }

    void CheckUpdateHelper::downloadProgress(quint64 downloadSize, quint64 currentFileDownloadSize, quint64 currestFileSize)
    {
      if (currestFileSize > 0) {
        qreal progress = 100.0f * (static_cast<qreal>(currentFileDownloadSize) / currestFileSize);
        if (progress < 0)
          progress = 0;

        if (progress > 100)
           progress = 100;

        emit this->checkUpdateProgressChanged(this->_service->id(), static_cast<quint8>(progress));
      }
    }

    void CheckUpdateHelper::downloadResult(bool isError, DownloadResults error)
    {
      if (isError) {
        emit this->error(this->_service);
      } else {
        emit this->checkUpdateProgressChanged(this->_service->id(), 100);
        emit this->result(this->_service, true);
        
        Md5FileHasher hasher;
        QString hash = hasher.getFileHash(this->getTorrentPath(this->_service));

        this->saveLastModifiedDate(this->_lastModified);
        this->saveTorrenthash(hash);
      }
    }

    void CheckUpdateHelper::downloadWarning(bool isError, DownloadResults error)
    {
    }

    void CheckUpdateHelper::saveLastModifiedDate(const QString& date)
    {
     Settings::Settings settings; 
     settings.beginGroup("GameDownloader");
     settings.beginGroup("CheckUpdate");
     settings.beginGroup(this->_service->id());
     settings.setValue("LastModified", date, true);
    }

    QString CheckUpdateHelper::loadLastModifiedDate() const
    {
      Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(this->_service->id());
      return settings.value("LastModified", "").toString();
    }

    void CheckUpdateHelper::setMaxHeadRequestRetry(int count)
    {
      this->_maxHeadRequestRetryCount = count;
    }

    void CheckUpdateHelper::saveTorrenthash(const QString& date)
    {
      Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(this->_service->id());
      settings.setValue("TorrentHash", date, true);
    }

    QString CheckUpdateHelper::torrentHash() const
    {
      Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(this->_service->id());
      return settings.value("TorrentHash", "").toString();
    }

  }
}