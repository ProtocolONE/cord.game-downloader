/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/PreHookBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>

#include <QtCore/QtConcurrentRun>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      PreHookBehavior::PreHookBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
        QObject::connect(this, SIGNAL(preHooksCompleted(GGS::GameDownloader::ServiceState *, GGS::GameDownloader::HookBase::HookResult)),
          SLOT(preHooksCompletedSlot(GGS::GameDownloader::ServiceState *, GGS::GameDownloader::HookBase::HookResult )));
      }

      PreHookBehavior::~PreHookBehavior(void)
      {
      }

      void PreHookBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        QtConcurrent::run(this, &PreHookBehavior::preHookLoop, state);
      }

      void PreHookBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
        emit this->stopping(state);
      }

      void PreHookBehavior::preHookLoop(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->preHooksCompleted(state, HookBase::Continue); // Походу надо будет урезать результаты хуков 
        }

        const GGS::Core::Service *service = state->service();
        emit this->statusMessageChanged(state, QObject::tr("PRE_HOOK_DEFAULT_MESSAGE"));

        HookBase::HookResult result = HookBase::Continue;
        if(this->_beforeDownloadHookMap.contains(service->id())) {
          Q_FOREACH(HookBase *hook, this->_beforeDownloadHookMap[service->id()]) {
            if (state->state() != ServiceState::Started) {
              emit this->preHooksCompleted(state, HookBase::Continue); // Походу надо будет урезать результаты хуков 
              return;
            }

            result = hook->beforeDownload(this->_gameDownloadService, state);

            emit this->statusMessageChanged(state, QObject::tr("PRE_HOOK_DEFAULT_MESSAGE"));
            if (result != HookBase::Continue)
              break;
          }
        }

        emit this->preHooksCompleted(state, result);
      }

      void PreHookBehavior::preHooksCompletedSlot(GGS::GameDownloader::ServiceState *state, GGS::GameDownloader::HookBase::HookResult result)
      {
        // Можно тут кстати обработать разные выходы с хуков и вызвать либо фейли либо разные выходы
        if (result == HookBase::Continue)
          emit this->next(Finished, state);
        else
          emit failed(state);
      }

      void PreHookBehavior::setGameDownloaderService(GameDownloadService *gameDownloadService)
      {
        this->_gameDownloadService = gameDownloadService;
      }

      void PreHookBehavior::registerHook(const QString& serviceId, int preHookPriority, HookBase *hook)
      {
        this->_beforeDownloadHookMap[serviceId].insert(-preHookPriority, hook);
      }

    }
  }
}