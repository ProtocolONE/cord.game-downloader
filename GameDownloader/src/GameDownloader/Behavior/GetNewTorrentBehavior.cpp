#include <GameDownloader/Behavior/GetNewTorrentBehavior.h>
#include <GameDownloader/Behavior/Private/DownloadFileHelper.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <UpdateSystem/Hasher/Md5FileHasher.h>

#include <Core/Service.h>
#include <Settings/Settings.h>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      GetNewTorrentBehavior::GetNewTorrentBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      GetNewTorrentBehavior::~GetNewTorrentBehavior()
      {

      }

      void GetNewTorrentBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        DownloadFileHelper *helper = new DownloadFileHelper(state, this);
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(finished(P1::GameDownloader::ServiceState *)),
          this, SLOT(internalFinished(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(P1::GameDownloader::ServiceState *)),
          this, SLOT(internalError(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        QUrl uri = state->service()->torrentUrlWithArea();
        QString fileName = QString("%1.torrent").arg(state->id());
        uri = uri.resolved(QUrl(fileName));

        helper->download(uri.toString(), CheckUpdateHelper::getTorrentPath(state));
      }

      void GetNewTorrentBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
      }

      void GetNewTorrentBehavior::internalFinished(P1::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();

        P1::Hasher::Md5FileHasher hasher;
        QString hash = hasher.getFileHash(CheckUpdateHelper::getTorrentPath(state));

        CheckUpdateHelper::saveTorrenthash(hash, state);
        CheckUpdateHelper::saveLastModifiedDate(state->torrentLastModifedDate(), state);

        emit this->next(Downloaded, state);
      }

      void GetNewTorrentBehavior::internalError(P1::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->failed(state);
      }

    }
  }
}