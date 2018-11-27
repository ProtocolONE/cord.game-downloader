#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <GameDownloader/Behavior/GetPatchVersionBehavior.h>
#include <GameDownloader/Behavior/Private/DownloadFileHelper.h>
#include <GameDownloader/Behavior/Private/CheckPatchExist.h>

#include <LibtorrentWrapper/Wrapper.h>

#include <Core/Service.h>
#include <Settings/Settings.h>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      GetPatchVersionBehavior::GetPatchVersionBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      GetPatchVersionBehavior::~GetPatchVersionBehavior()
      {
      }

      void GetPatchVersionBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());
        Q_CHECK_PTR(this->_wrapper);

        DownloadFileHelper *helper = new DownloadFileHelper(state, this);

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(P1::GameDownloader::ServiceState *)),
          this, SLOT(internalError(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(finished(P1::GameDownloader::ServiceState *)),
          this, SLOT(internalFinished(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        const P1::Core::Service *service = state->service();

        QUrl uri = service->torrentUrlWithArea();
        QString fileName = QString("%1.torrent").arg(service->id());
        uri = uri.resolved(QUrl(fileName));

        QString path = QString("%1/patch/%3/%4.torrent")
          .arg(service->torrentFilePath())
          .arg(service->areaString())
          .arg(service->id());

        helper->download(uri.toString(), path);
      }

      void GetPatchVersionBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
      }

      void GetPatchVersionBehavior::internalError(P1::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->failed(state);
      }

      void GetPatchVersionBehavior::internalFinished(P1::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();

        const P1::Core::Service *service = state->service();
        QString t0 = CheckUpdateHelper::getTorrentPath(state);
        QString t1 = QString("%1/patch/%3/%4.torrent")
          .arg(service->torrentFilePath())
          .arg(service->areaString())
          .arg(service->id());

        QString newHash;
        QString oldHash;
        if (!this->_wrapper->getInfoHash(t1, newHash)
          || !this->_wrapper->getInfoHash(t0, oldHash)) {
            emit this->next(PatchNotFound, state);
            return;
        }

        QString patchVersion = QString("%1-%2").arg(newHash, oldHash);
        state->setPatchVersion(patchVersion);

        this->checkPatchExist(state);
      }

      void GetPatchVersionBehavior::checkPatchExist(P1::GameDownloader::ServiceState *state)
      {
        CheckPatchExist* pathExist = new CheckPatchExist(state, this);
        SIGNAL_CONNECT_CHECK(QObject::connect(pathExist, SIGNAL(found(P1::GameDownloader::ServiceState *)),
          this, SLOT(checkPatchFound(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(pathExist, SIGNAL(notFound(P1::GameDownloader::ServiceState *)),
          this, SLOT(checkPatchNotFound(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

#ifdef USE_MINI_ZIP_LIB
        pathExist->startCheck(this->getServicePatchUrl(state) + ".zip");
#else
        pathExist->startCheck(this->getServicePatchUrl(state) + ".7z");
#endif
      }

      void GetPatchVersionBehavior::setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper)
      {
        this->_wrapper = wrapper;
      }

      void GetPatchVersionBehavior::checkPatchFound(P1::GameDownloader::ServiceState *state)
      {
        CheckPatchExist *reply = qobject_cast<CheckPatchExist*>(QObject::sender());
        if (!reply)
          return;

        reply->deleteLater();

        DownloadFileHelper *helper = new DownloadFileHelper(state, this);
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(finished(P1::GameDownloader::ServiceState *)),
          this, SLOT(internalPatchFinished(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(P1::GameDownloader::ServiceState *)),
          this, SLOT(internalPatchError(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        const P1::Core::Service *service = state->service();
        QString url = this->getServicePatchUrl(state);

        QString path = QString("%1/patch/%2/%3/%4.torrent")
          .arg(service->torrentFilePath())
          .arg(state->patchVersion())
          .arg(service->areaString())
          .arg(service->id());

        helper->download(url, path);
      }

      void GetPatchVersionBehavior::checkPatchNotFound(P1::GameDownloader::ServiceState *state)
      {
        CheckPatchExist *reply = qobject_cast<CheckPatchExist*>(QObject::sender());
        if (!reply) {
          return;
        }

        reply->deleteLater();
        emit this->next(PatchNotFound, state);
      }

      void GetPatchVersionBehavior::internalPatchFinished(P1::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->next(PatchFound, state);
      }

      void GetPatchVersionBehavior::internalPatchError(P1::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->next(PatchNotFound, state);
      }

      QString GetPatchVersionBehavior::getServicePatchUrl(P1::GameDownloader::ServiceState *state)
      {
        const P1::Core::Service *service = state->service();
        QString patchVersion = state->patchVersion();     

        QString url = service->torrentUrl().toString();

        QString tail = QString("delta/%1/%2/%3.torrent").arg(patchVersion, service->areaString(), state->id());
        url.append(tail);

        return url;
      }
    }
  }
}