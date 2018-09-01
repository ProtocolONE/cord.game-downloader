#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace P1 {
  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT RehashClientBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          GameBroken = 1,
          GameFine = 2
        };

        explicit RehashClientBehavior(QObject *parent = 0);
        ~RehashClientBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper);

      signals:
        void downloadProgressChanged(P1::GameDownloader::ServiceState *, qint8, P1::Libtorrent::EventArgs::ProgressEventArgs);

      private slots:
        void torrentPausedSlot(QString id);
        void torrentDownloadFinishedSlot(QString id);
        void torrentDownloadFailedSlot(QString id);
        void torrentDownloadRehashed(QString id, bool isComplete);
        void torrentProgress(P1::Libtorrent::EventArgs::ProgressEventArgs arg);

      private:
        P1::Libtorrent::Wrapper *_wrapper;
        QMutex _mutex;
        QHash<QString, P1::GameDownloader::ServiceState*> _stateMap;

        void setState(P1::GameDownloader::ServiceState *state);
        P1::GameDownloader::ServiceState* state(const QString& id);

        void syncStartTorrent(P1::GameDownloader::ServiceState *state);
        void syncStopTorrent(P1::GameDownloader::ServiceState *state);
      };

    }
  }
}