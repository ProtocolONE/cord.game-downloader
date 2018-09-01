#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

namespace P1 {
  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT GetPatchVersionBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          PatchFound = 1,
          PatchNotFound = 2
        };

        explicit GetPatchVersionBehavior(QObject *parent = 0);
        virtual ~GetPatchVersionBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;
        void setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper);

      private slots:
        void internalFinished(P1::GameDownloader::ServiceState *state);
        void internalError(P1::GameDownloader::ServiceState *state);

        void internalPatchFinished(P1::GameDownloader::ServiceState *state);
        void internalPatchError(P1::GameDownloader::ServiceState *state);

        void checkPatchFound(P1::GameDownloader::ServiceState *state);
        void checkPatchNotFound(P1::GameDownloader::ServiceState *state);

      private:
        P1::Libtorrent::Wrapper *_wrapper;
        
        void checkPatchExist(P1::GameDownloader::ServiceState *state);
        QString getServicePatchUrl(P1::GameDownloader::ServiceState *state);
      };

    }
  }
}