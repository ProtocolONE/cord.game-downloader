/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <GameDownloader/Behavior/GetPatchVersionBehavior.h>
#include <GameDownloader/Behavior/Private/DownloadFileHelper.h>
#include <GameDownloader/Behavior/Private/CheckPatchExist.h>

#include <LibtorrentWrapper/Wrapper>

#include <Core/Service.h>
#include <Settings/Settings.h>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      GetPatchVersionBehavior::GetPatchVersionBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      GetPatchVersionBehavior::~GetPatchVersionBehavior()
      {
      }

      void GetPatchVersionBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());
        Q_CHECK_PTR(this->_wrapper);

        DownloadFileHelper *helper = new DownloadFileHelper(state, this);

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(GGS::GameDownloader::ServiceState *)),
          this, SLOT(internalError(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(finished(GGS::GameDownloader::ServiceState *)),
          this, SLOT(internalFinished(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        const GGS::Core::Service *service = state->service();

        QUrl uri = service->torrentUrlWithArea();
        QString fileName = QString("%1.torrent").arg(service->id());
        uri = uri.resolved(QUrl(fileName));

        QString path = QString("%1/patch/%3/%4.torrent")
          .arg(service->torrentFilePath())
          .arg(service->areaString())
          .arg(service->id());

        helper->download(uri.toString(), path);
      }

      void GetPatchVersionBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
      }

      void GetPatchVersionBehavior::internalError(GGS::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->failed(state);
      }

      void GetPatchVersionBehavior::internalFinished(GGS::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();

        const GGS::Core::Service *service = state->service();
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

      void GetPatchVersionBehavior::checkPatchExist(GGS::GameDownloader::ServiceState *state)
      {
        CheckPatchExist* pathExist = new CheckPatchExist(state, this);
        SIGNAL_CONNECT_CHECK(QObject::connect(pathExist, SIGNAL(found(GGS::GameDownloader::ServiceState *)),
          this, SLOT(checkPatchFound(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(pathExist, SIGNAL(notFound(GGS::GameDownloader::ServiceState *)),
          this, SLOT(checkPatchNotFound(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        pathExist->startCheck(this->getServicePatchUrl(state) + ".7z");
      }

      void GetPatchVersionBehavior::setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper)
      {
        this->_wrapper = wrapper;
      }

      void GetPatchVersionBehavior::checkPatchFound(GGS::GameDownloader::ServiceState *state)
      {
        CheckPatchExist *reply = qobject_cast<CheckPatchExist*>(QObject::sender());
        if (!reply)
          return;

        reply->deleteLater();

        DownloadFileHelper *helper = new DownloadFileHelper(state, this);
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(finished(GGS::GameDownloader::ServiceState *)),
          this, SLOT(internalPatchFinished(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(GGS::GameDownloader::ServiceState *)),
          this, SLOT(internalPatchError(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        const GGS::Core::Service *service = state->service();
        QString url = this->getServicePatchUrl(state);

        QString path = QString("%1/patch/%2/%3/%4.torrent")
          .arg(service->torrentFilePath())
          .arg(state->patchVersion())
          .arg(service->areaString())
          .arg(service->id());

        helper->download(url, path);
      }

      void GetPatchVersionBehavior::checkPatchNotFound(GGS::GameDownloader::ServiceState *state)
      {
        CheckPatchExist *reply = qobject_cast<CheckPatchExist*>(QObject::sender());
        if (!reply) {
          return;
        }

        reply->deleteLater();
        emit this->next(PatchNotFound, state);
      }

      void GetPatchVersionBehavior::internalPatchFinished(GGS::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->next(PatchFound, state);
      }

      void GetPatchVersionBehavior::internalPatchError(GGS::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->next(PatchNotFound, state);
      }

      QString GetPatchVersionBehavior::getServicePatchUrl(GGS::GameDownloader::ServiceState *state)
      {
        const GGS::Core::Service *service = state->service();
        QString patchVersion = state->patchVersion();     

        QString url = service->torrentUrl().toString();

        QString tail = QString("delta/%1/%2/%3.torrent").arg(patchVersion, service->areaString(), state->id());
        url.append(tail);

        return url;
      }
    }
  }
}