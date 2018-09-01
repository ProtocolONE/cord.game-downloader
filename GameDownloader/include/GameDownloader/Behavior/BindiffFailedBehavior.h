#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT BindiffFailedBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1
        };

        explicit BindiffFailedBehavior(QObject *parent = 0);
        virtual ~BindiffFailedBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

      };


    }
  }
}