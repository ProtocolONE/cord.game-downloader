#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace P1 {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT ReadOnlyBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1
        };

        explicit ReadOnlyBehavior(QObject *parent = 0);
        virtual ~ReadOnlyBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void keepCalmMessage(P1::GameDownloader::ServiceState *state);
        void processFinished(int result, P1::GameDownloader::ServiceState *state);

      signals:
        void scanFolders(const QStringList & folders, P1::GameDownloader::ServiceState *state);
      };

    }
  }
}
