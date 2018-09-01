#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class GetNewTorrentBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Downloaded = 1
        };

        explicit GetNewTorrentBehavior(QObject *parent = 0);
        virtual ~GetNewTorrentBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

      signals:
        void progressChanged(const QString& serviceId, quint8 progress);

      private slots:
        void internalFinished(P1::GameDownloader::ServiceState *state);
        void internalError(P1::GameDownloader::ServiceState *state);

      };

    }
  }
}