#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/ServiceState.h>

#include <UpdateSystem/Downloader/downloadmanager.h>
#include <UpdateSystem/Downloader/DynamicRetryTimeout.h>
#include <UpdateSystem/Downloader/RetryFileDownloader.h>
#include <UpdateSystem/Downloader/MultiFileDownloader.h>
#include <UpdateSystem/Downloader/MultiFileDownloaderWithExtracter.h>
#include <UpdateSystem/Hasher/Md5FileHasher.h>

#ifdef USE_MINI_ZIP_LIB
#include <UpdateSystem/Extractor/MiniZipExtractor.h>
#else
#include <UpdateSystem/Extractor/SevenZipExtractor.h>
#endif

#include <Settings/Settings.h>
#include <Core/Service.h>

#include <QtCore/QDateTime>
#include <QtCore/QDir>
#include <QtCore/QRegExp>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkRequest>

using namespace P1::Downloader;
using namespace P1::Extractor;
using namespace P1::Core;
using namespace P1::Hasher;

namespace P1 {
  namespace GameDownloader {
    CheckUpdateHelper::CheckUpdateHelper(QObject *parent)
      : QObject(parent)
      , _headRequestRetryCount(0)
      , _maxHeadRequestRetryCount(3)
    {
      this->_manager = new QNetworkAccessManager(this);
#ifdef USE_MINI_ZIP_LIB
      this->_extractor = new MiniZipExtractor(this);
#else
      this->_extractor = new SevenZipExtractor(this);
#endif
    }

    CheckUpdateHelper::~CheckUpdateHelper()
    {
    }

    QUrl CheckUpdateHelper::getTorrentUrlWithArchiveExtension()
    {
      QUrl url = this->_state->service()->torrentUrlWithArea();
#ifdef USE_MINI_ZIP_LIB
      QString fileName = QString("%1.torrent.zip").arg(this->_state->id());
#else
      QString fileName = QString("%1.torrent.7z").arg(this->_state->id());
#endif
      url = url.resolved(QUrl(fileName));
      return url;
    }

    QString CheckUpdateHelper::getTorrentUrlWithoutArchiveExtension()
    {
      QUrl url = this->_state->service()->torrentUrlWithArea();
      QString fileName = QString("%1.torrent").arg(this->_state->id());
      url = url.resolved(QUrl(fileName));
      return url.toString();
    }

    QString CheckUpdateHelper::getTorrentPath(P1::GameDownloader::ServiceState *state)
    {
      const P1::Core::Service *service = state->service();
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

      CRITICAL_LOG << "Error on head request in check update. Error: " << error << " service " << this->_state->id();
      reply->deleteLater();

      this->_headRequestRetryCount++;
      if (this->_headRequestRetryCount < this->_maxHeadRequestRetryCount)
        this->startCheck(this->_state, this->_checkUpdateType);
      else
        emit this->error(this->_state);
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
        emit this->error(this->_state);
        return;
      }

      this->_lastModified = QString::fromLatin1(reply->rawHeader(QByteArray("Last-Modified")));

      QString torrentPath = CheckUpdateHelper::getTorrentPath(this->_state);
      Md5FileHasher hasher;

      bool isDownloadRequired = this->_checkUpdateType == ForceDownloadTorrent 
        || httpCode == 200
        || !QFile::exists(torrentPath)
        || this->torrentHash() != hasher.getFileHash(torrentPath);

      if (isDownloadRequired) {
        this->startDownloadTorrent();
        return;
      }

      emit this->checkUpdateProgressChanged(this->_state, 100);
      emit this->result(this->_state, false);
    }

    void CheckUpdateHelper::startCheck(P1::GameDownloader::ServiceState *state, CheckUpdateType checkUpdateType)
    {
      Q_CHECK_PTR(state);
      Q_CHECK_PTR(state->service());

      this->_checkUpdateType = checkUpdateType;
      this->_state = state;
      emit this->checkUpdateProgressChanged(state, 0);
      QNetworkRequest request(this->getTorrentUrlWithArchiveExtension());

      QString oldLastModifed = this->loadLastModifiedDate();
      request.setRawHeader("If-Modified-Since", oldLastModifed.toLatin1());

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
      QString path = CheckUpdateHelper::getTorrentPath(this->_state);
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

        emit this->checkUpdateProgressChanged(this->_state, static_cast<quint8>(progress));
      }
    }

    void CheckUpdateHelper::downloadResult(bool isError, DownloadResults error)
    {
      if (isError) {
        emit this->error(this->_state);
      } else {
        emit this->checkUpdateProgressChanged(this->_state, 100);
        emit this->result(this->_state, true);
        
        Md5FileHasher hasher;
        QString hash = hasher.getFileHash(CheckUpdateHelper::getTorrentPath(this->_state));

        CheckUpdateHelper::saveLastModifiedDate(this->_lastModified, this->_state);
        CheckUpdateHelper::saveTorrenthash(hash, this->_state);
      }
    }

    void CheckUpdateHelper::downloadWarning(bool isError, DownloadResults error)
    {
    }

    void CheckUpdateHelper::saveLastModifiedDate(const QString& date, P1::GameDownloader::ServiceState *state)
    {
     Settings::Settings settings; 
     settings.beginGroup("GameDownloader");
     settings.beginGroup("CheckUpdate");
     settings.beginGroup(state->id());
     settings.setValue("LastModified", date, true);
    }

    QString CheckUpdateHelper::loadLastModifiedDate() const
    {
      Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(this->_state->id());
      return settings.value("LastModified", "").toString();
    }

    void CheckUpdateHelper::setMaxHeadRequestRetry(int count)
    {
      this->_maxHeadRequestRetryCount = count;
    }

    void CheckUpdateHelper::saveTorrenthash(const QString& date, P1::GameDownloader::ServiceState *state)
    {
      Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(state->id());
      settings.setValue("TorrentHash", date, true);
    }

    QString CheckUpdateHelper::torrentHash() const
    {
      Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(this->_state->id());
      return settings.value("TorrentHash", "").toString();
    }

  }
}