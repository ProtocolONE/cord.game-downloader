#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QObject>

namespace P1 {
  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT CreateFastResumeBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Created = 1
        };

        explicit CreateFastResumeBehavior(QObject *parent = 0);
        virtual ~CreateFastResumeBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper);
      private:
        P1::Libtorrent::Wrapper *_wrapper;
      };
    }
  }
}