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
      class DOWNLOADSERVICE_EXPORT PreHookBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1
        };

        explicit PreHookBehavior(QObject *parent = 0);
        virtual ~PreHookBehavior(void);
        
        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void setGameDownloaderService(GameDownloadService *gameDownloadService);
        void registerHook(const QString& serviceId, int preHookPriority, HookBase *hook);

      signals:
        void stopping(P1::GameDownloader::ServiceState *state);
        void preHooksCompleted(P1::GameDownloader::ServiceState *state, 
          P1::GameDownloader::HookBase::HookResult result);

      private slots:
        void preHooksCompletedSlot(P1::GameDownloader::ServiceState *state, 
          P1::GameDownloader::HookBase::HookResult result);

      private:
        GameDownloadService *_gameDownloadService;
        QHash<QString, QMultiMap<int, HookBase*> > _beforeDownloadHookMap;
        QSet<HookBase *> _registredHooks;

        void preHookLoop(P1::GameDownloader::ServiceState *state);
      };


    }
  }
}
