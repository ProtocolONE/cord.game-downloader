#pragma once
#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace P1 {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT CheckUpdateBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1
        };

        explicit CheckUpdateBehavior(QObject *parent = 0);
        virtual ~CheckUpdateBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;  
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

      private slots:
        void checkUpdateResult(P1::GameDownloader::ServiceState *state, bool isUpdated);
        void checkUpdateError(P1::GameDownloader::ServiceState *state);
        void checkUpdateProgressChanged(P1::GameDownloader::ServiceState *state, quint8 progress);

      };
    }
  }
}
