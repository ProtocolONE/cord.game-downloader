#include <GameDownloader/Behavior/Private/CheckTorrentModify.h>
#include <GameDownloader/ServiceState.h>

#include <GameDownloader/CheckUpdateHelper.h>

#include <UpdateSystem/Hasher/Md5FileHasher.h>

#include <Settings/Settings.h>
#include <Core/Service.h>

#include <QtNetwork/QNetworkRequest>

using namespace P1::Core;
using namespace P1::Hasher;

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      CheckTorrentModify::CheckTorrentModify(QObject *parent)
        : QObject(parent)
        , _headRequestRetryCount(0)
        , _maxHeadRequestRetryCount(3)
        , _manager(new QNetworkAccessManager(this))
      {
      }

      CheckTorrentModify::~CheckTorrentModify()
      {
      }

      QString CheckTorrentModify::lastModified() const
      {
        return this->_lastModified;
      }

      void CheckTorrentModify::saveLastModifiedDate(const QString& date)
      {
        Settings::Settings settings; 
        settings.beginGroup("GameDownloader");
        settings.beginGroup("CheckUpdate");
        settings.beginGroup(this->_state->id());
        settings.setValue("LastModified", date, true);
      }

      QString CheckTorrentModify::loadLastModifiedDate() const
      {
        Settings::Settings settings; 
        settings.beginGroup("GameDownloader");
        settings.beginGroup("CheckUpdate");
        settings.beginGroup(this->_state->id());
        return settings.value("LastModified", "").toString();
      }

      void CheckTorrentModify::saveTorrenthash(const QString& date)
      {
        Settings::Settings settings; 
        settings.beginGroup("GameDownloader");
        settings.beginGroup("CheckUpdate");
        settings.beginGroup(this->_state->id());
        settings.setValue("TorrentHash", date, true);
      }

      QString CheckTorrentModify::torrentHash() const
      {
        Settings::Settings settings; 
        settings.beginGroup("GameDownloader");
        settings.beginGroup("CheckUpdate");
        settings.beginGroup(this->_state->id());
        return settings.value("TorrentHash", "").toString();
      }

      void CheckTorrentModify::start(P1::GameDownloader::ServiceState *state)
      {
        this->_state = state;
        QUrl torrentUrl = this->getTorrentUrlWithArchiveExtension();
        QNetworkRequest request(torrentUrl);

        QString oldLastModifed = this->loadLastModifiedDate();
        request.setRawHeader("If-Modified-Since", oldLastModifed.toLatin1());

        QNetworkReply *reply = this->_manager->head(request);
        connect(reply, SIGNAL(finished()), this, SLOT(slotReplyDownloadFinished()));
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(slotError(QNetworkReply::NetworkError))); 
      }

      void CheckTorrentModify::slotError(QNetworkReply::NetworkError error)
      {
        QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
        if (!reply)
          return;

        CRITICAL_LOG << "Error on head request in check update. Error: " << error << " service " << this->_state->id();
        reply->deleteLater();

        this->_headRequestRetryCount++;
        if (this->_headRequestRetryCount < this->_maxHeadRequestRetryCount)
          this->start(this->_state);
        else
          emit this->error(this->_state);
      }

      void CheckTorrentModify::slotReplyDownloadFinished()
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

        bool isModified = httpCode == 200
          || !QFile::exists(torrentPath)
          || this->torrentHash() != hasher.getFileHash(torrentPath);

        emit result(this->_state, isModified);
      }

      QUrl CheckTorrentModify::getTorrentUrlWithArchiveExtension()
      {
        QUrl url = this->_state->service()->torrentUrlWithArea();
        QString fileName = QString("%1.torrent.7z").arg(this->_state->id());
        url = url.resolved(QUrl(fileName));
        return url;
      }

    }
  }
}