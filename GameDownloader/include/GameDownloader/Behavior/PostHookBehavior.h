#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>
#include <QtCore/QMultiMap>
#include <QtCore/QHash>

namespace P1 {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    class ServiceState;
    class GameDownloadService;

    namespace Behavior {
      class DOWNLOADSERVICE_EXPORT PostHookBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1,
          
          // Хук решил что обновление прошлои плохо и отправил в начало.
          ReturnToStart = 2,
        };

        explicit PostHookBehavior(QObject *parent = 0);
        virtual ~PostHookBehavior(void);

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void setGameDownloaderService(GameDownloadService *gameDownloadService);
        void registerHook(const QString& serviceId, int preHookPriority, HookBase *hook);

      signals:
        void stopping(P1::GameDownloader::ServiceState *state);
        void postHooksCompleted(P1::GameDownloader::ServiceState *state, 
          P1::GameDownloader::HookBase::HookResult result);

      private slots:
        void postHooksCompletedSlot(P1::GameDownloader::ServiceState *state, 
          P1::GameDownloader::HookBase::HookResult result);

      private:
        GameDownloadService *_gameDownloadService;
        QHash<QString, QMultiMap<int, HookBase*> > _afterDownloadHookMap;
        QSet<HookBase *> _registredHooks;

        void postHookLoop(P1::GameDownloader::ServiceState *state);
      };


    }
  }
}
