/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#pragma once

#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>
#include <QtCore/QMultiMap>
#include <QtCore/QHash>

namespace GGS {
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
        ~PreHookBehavior(void);
        
        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

        void setGameDownloaderService(GameDownloadService *gameDownloadService);
        void registerHook(const QString& serviceId, int preHookPriority, HookBase *hook);

      signals:
        void stopping(GGS::GameDownloader::ServiceState *state);
        void preHooksCompleted(GGS::GameDownloader::ServiceState *state, 
          GGS::GameDownloader::HookBase::HookResult result);

      private slots:
        void preHooksCompletedSlot(GGS::GameDownloader::ServiceState *state, 
          GGS::GameDownloader::HookBase::HookResult result);

      private:
        GameDownloadService *_gameDownloadService;
        QHash<QString, QMultiMap<int, HookBase*> > _beforeDownloadHookMap;
        QSet<HookBase *> _registredHooks;

        void preHookLoop(GGS::GameDownloader::ServiceState *state);
      };


    }
  }
}
