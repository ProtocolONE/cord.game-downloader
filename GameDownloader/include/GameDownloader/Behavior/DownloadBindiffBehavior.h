#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace P1 {
  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DownloadBindiffBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Downloaded = 1
        };

        explicit DownloadBindiffBehavior(QObject *parent = 0);
        virtual ~DownloadBindiffBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper);

      signals:
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
        QString getPatchId(const P1::GameDownloader::ServiceState* state);

        ServiceState* state(const QString& id);

        void syncStartTorrent(P1::GameDownloader::ServiceState *state);
        void syncStopTorrent(P1::GameDownloader::ServiceState *state);

        QString getPatchTorrentPath(P1::GameDownloader::ServiceState *state);
        void moveT1toT0(P1::GameDownloader::ServiceState *state);

        P1::Libtorrent::Wrapper *_wrapper;
        QMutex _mutex;
        QHash<QString, P1::GameDownloader::ServiceState*> _stateMap;

      };

    }
  }
}