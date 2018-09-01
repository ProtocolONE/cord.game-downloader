#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace P1 {
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
        virtual ~TorrentDownloadBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper);

      signals:
        void downloadFinished(P1::GameDownloader::ServiceState *state);

        void downloadProgressChanged(
          P1::GameDownloader::ServiceState *state,
          qint8 progress, 
          P1::Libtorrent::EventArgs::ProgressEventArgs args);

      private slots:
        void torrentPausedSlot(QString id);
        void torrentDownloadFinishedSlot(QString id);
        void torrentDownloadFailedSlot(QString id);
        void torrentProgress(P1::Libtorrent::EventArgs::ProgressEventArgs arg);

      private:
        void setState(P1::GameDownloader::ServiceState *state);
        ServiceState* state(const QString& id);
        bool isInstalled(const QString& id);

        void syncStartTorrent(P1::GameDownloader::ServiceState *state);
        void syncStopTorrent(P1::GameDownloader::ServiceState *state);

        P1::Libtorrent::Wrapper *_wrapper;
        QMutex _mutex;
        QHash<QString, P1::GameDownloader::ServiceState*> _stateMap;
        QHash<QString, bool> _installedStateMap;

      };

    }
  }
}
