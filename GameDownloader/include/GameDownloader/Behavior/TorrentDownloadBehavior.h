/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace GGS {
  namespace Core {
    class Service;
  }

  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    class ServiceState;
    class GameDownloadService;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT TorrentDownloadBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Downloaded = 1
        };

        explicit TorrentDownloadBehavior(QObject *parent = 0);
        ~TorrentDownloadBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

        void setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper);

      signals:
        void downloadFinished(GGS::GameDownloader::ServiceState *state);

        void downloadProgressChanged(
          GGS::GameDownloader::ServiceState *state,
          qint8 progress, 
          GGS::Libtorrent::EventArgs::ProgressEventArgs args);

      private slots:
        void torrentPausedSlot(QString id);
        void torrentDownloadFinishedSlot(QString id);
        void torrentDownloadFailedSlot(QString id);
        void torrentProgress(GGS::Libtorrent::EventArgs::ProgressEventArgs arg);

      private:
        void setState(GGS::GameDownloader::ServiceState *state);
        ServiceState* state(const QString& id);
        
        void syncStartTorrent(GGS::GameDownloader::ServiceState *state);
        void syncStopTorrent(GGS::GameDownloader::ServiceState *state);

        GGS::Libtorrent::Wrapper *_wrapper;
        QMutex _mutex;
        QHash<QString, GGS::GameDownloader::ServiceState*> _stateMap;

      };

    }
  }
}
