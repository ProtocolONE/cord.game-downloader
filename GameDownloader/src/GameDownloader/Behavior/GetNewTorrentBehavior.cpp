/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/GetNewTorrentBehavior.h>
#include <GameDownloader/Behavior/Private/DownloadFileHelper.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <UpdateSystem/Hasher/Md5FileHasher>

#include <Core/Service.h>
#include <Settings/Settings.h>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      GetNewTorrentBehavior::GetNewTorrentBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      GetNewTorrentBehavior::~GetNewTorrentBehavior()
      {

      }

      void GetNewTorrentBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        DownloadFileHelper *helper = new DownloadFileHelper(state, this);
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(finished(GGS::GameDownloader::ServiceState *)),
          this, SLOT(internalFinished(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(GGS::GameDownloader::ServiceState *)),
          this, SLOT(internalError(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));
        //SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(progressChanged(GGS::GameDownloader::ServiceState *, quint8)),
        //  this, SIGNAL(progressChanged(GGS::GameDownloader::ServiceState *, quint8)),
        //  Qt::QueuedConnection));

        QUrl uri = state->service()->torrentUrlWithArea();
        QString fileName = QString("%1.torrent").arg(state->id());
        uri = uri.resolved(QUrl(fileName));

        helper->download(uri.toString(), CheckUpdateHelper::getTorrentPath(state));
      }

      void GetNewTorrentBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
      }

      void GetNewTorrentBehavior::internalFinished(GGS::GameDownloader::ServiceState *state)
      {
        DownloadFileHelper *helper = qobject_cast<DownloadFileHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();

        GGS::Hasher::Md5FileHasher hasher;
        QString hash = hasher.getFileHash(CheckUpdateHelper::getTorrentPath(state));

        CheckUpdateHelper::saveTorrenthash(hash, state);
        CheckUpdateHelper::saveLastModifiedDate(state->torrentLastModifedDate(), state);

        emit this->next(Downloaded, state);
      }

      void GetNewTorrentBehavior::internalError(GGS::GameDownloader::ServiceState *state)
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