#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT FinishBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        explicit FinishBehavior(QObject *parent = 0);
        virtual ~FinishBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

      private:
        bool removeDir(const QString &dirName);

      };

    }
  }
}