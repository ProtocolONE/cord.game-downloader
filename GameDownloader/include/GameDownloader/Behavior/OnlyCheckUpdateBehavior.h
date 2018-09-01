#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class OnlyCheckUpdateBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          UpdateFound = 1,
          UpdateNotFound = 2,
        };

        explicit OnlyCheckUpdateBehavior(QObject *parent = 0);
        virtual ~OnlyCheckUpdateBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

      private slots:
        void checkUpdateResult(P1::GameDownloader::ServiceState *state, bool isModified);
        void checkUpdateError(P1::GameDownloader::ServiceState *state);
      };


    }
  }
}
