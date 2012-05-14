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

#include <Settings/Settings>

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtNetwork/QNetworkRequest>
#include <curl/curl.h>

#include <QtCore/QDebug>

using namespace GGS::Downloader;
using namespace GGS::Extractor;
using namespace GGS::Core;

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

    QString CheckUpdateHelper::getTorrentUrlWithArchiveExtension()
    {
      QUrl url = this->_service->torrentUrlWithArea();
      QString fileName = QString("%1.torrent.7z").arg(this->_service->id());
      url = url.resolved(QUrl(fileName));
      return url.toString();
    }

    QString CheckUpdateHelper::getTorrentUrlWithoutArchiveExtension()
    {
      QUrl url = this->_service->torrentUrlWithArea();
      QString fileName = QString("%1.torrent").arg(this->_service->id());
      url = url.resolved(QUrl(fileName));
      return url.toString();
    }

    QString CheckUpdateHelper::getTorrentPath()
    {
      return QString("%1/%2/%3.torrent")
        .arg(this->_service->torrentFilePath())
        .arg(this->getServiceAreaString())
        .arg(this->_service->id());
    }

    QString CheckUpdateHelper::getServiceAreaString()
    {
      switch(this->_service->area()) {
      case Service::Live:
        return QString("live");
      case Service::Pts:
        return QString("pts");
      case Service::Tst:
        return QString("tst");
      default:
        return QString("");
      }
    }

    void CheckUpdateHelper::slotError(QNetworkReply::NetworkError error)
    {
      QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
      if (!reply)
        return;

      qDebug() << "Error on head request in check update. Error: " << error << " service " << this->_service->id();

      this->_headRequestRetryCount++;
      reply->deleteLater();

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

      QString lastModifiedHeadername("Last-Modified");
      this->_lastModified = QString::fromAscii(reply->rawHeader(lastModifiedHeadername.toAscii()));
      
      int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

      reply->deleteLater();

      // Http codes defined by rfc: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
      // 200 OK
      // 304 Not Modified
      if (this->_checkUpdateType == ForceDownloadTorrent && (httpCode == 304 || httpCode == 200)) {
        this->startDownloadTorrent();
        return;
      }

      if (httpCode == 304) {
        QString torrentPath = this->getTorrentPath();
        if (!QFile::exists(torrentPath)) {
          this->startDownloadTorrent();
        } else {
          emit this->checkUpdateProgressChanged(this->_service->id(), 100);
          emit this->result(this->_service, false);
        }
        
      } else if (httpCode == 200) {
        this->startDownloadTorrent();
      } 
    }

    void CheckUpdateHelper::startCheck(const GGS::Core::Service *service, CheckUpdateType checkUpdateType)
    {
      this->_checkUpdateType = checkUpdateType;
      this->_service = service;
      emit this->checkUpdateProgressChanged(service->id(), 0);
      QNetworkRequest request(QUrl(this->getTorrentUrlWithArchiveExtension()));

      QString oldLastModifed = this->loadLastModifiedDate();
      request.setRawHeader("If-Modified-Since", oldLastModifed.toAscii());

      QNetworkReply *reply = this->_manager->head(request);
      connect(reply, SIGNAL(finished()), this, SLOT(slotReplyDownloadFinished()));
      connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError))); 
    }

    void CheckUpdateHelper::startDownloadTorrent()
    {
      DynamicRetryTimeout* dynamicRetryTimeout = new DynamicRetryTimeout(this);
      dynamicRetryTimeout->addTimeout(5000);
      dynamicRetryTimeout->addTimeout(10000);
      dynamicRetryTimeout->addTimeout(15000);
      dynamicRetryTimeout->addTimeout(30000);

      RetryFileDownloader* retryDownloader = new RetryFileDownloader(this);
      retryDownloader->setMaxRetry(DynamicRetryTimeout::InfinityRetryCount);
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
      QString path = this->getTorrentPath();
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
        qreal progress = 100 * (static_cast<qreal>(currentFileDownloadSize) / static_cast<qreal>(currestFileSize));
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
        this->saveLastModifiedDate(this->_lastModified);
      }
    }

    void CheckUpdateHelper::downloadWarning(bool isError, DownloadResults error)
    {
    }

    void CheckUpdateHelper::saveLastModifiedDate(const QString& date)
    {
     Settings::Settings settings; 
     QString groupName = QString("GameDownloader/CheckUpdate/%1").arg(this->_service->id());
     settings.beginGroup(groupName);
     settings.setValue("LastModified", date, true);
    }

    QString CheckUpdateHelper::loadLastModifiedDate() const
    {
      Settings::Settings settings; 
      QString groupName = QString("GameDownloader/CheckUpdate/%1").arg(this->_service->id());
      settings.beginGroup(groupName);
      return settings.value("LastModified", "").toString();
    }

    void CheckUpdateHelper::setMaxHeadRequestRetry(int count)
    {
      this->_maxHeadRequestRetryCount = count;
    }
  }
}