/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/PostHookBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>

#include <QtConcurrent/QtConcurrentRun>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      PostHookBehavior::PostHookBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
        QObject::connect(this, SIGNAL(postHooksCompleted(GGS::GameDownloader::ServiceState *, GGS::GameDownloader::HookBase::HookResult)),
          SLOT(postHooksCompletedSlot(GGS::GameDownloader::ServiceState *, GGS::GameDownloader::HookBase::HookResult )));
      }

      PostHookBehavior::~PostHookBehavior(void)
      {
      }

      void PostHookBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        QtConcurrent::run(this, &PostHookBehavior::postHookLoop, state);
      }

      void PostHookBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
        emit this->stopping(state);
      }

      void PostHookBehavior::postHookLoop(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->postHooksCompleted(state, HookBase::Continue); // Походу надо будет урезать результаты хуков 
        }

        const GGS::Core::Service *service = state->service();
        emit this->statusMessageChanged(state, QObject::tr("PRE_HOOK_DEFAULT_MESSAGE"));

        HookBase::HookResult result = HookBase::Continue;
        if(this->_afterDownloadHookMap.contains(service->id())) {
          Q_FOREACH(HookBase *hook, this->_afterDownloadHookMap[service->id()]) {
            if (state->state() != ServiceState::Started) {
              emit this->postHooksCompleted(state, HookBase::Continue); // Походу надо будет урезать результаты хуков 
              return;
            }

            result = hook->afterDownload(this->_gameDownloadService, state);

            emit this->statusMessageChanged(state, QObject::tr("PRE_HOOK_DEFAULT_MESSAGE"));
            if (result != HookBase::Continue)
              break;
          }
        }

        emit this->postHooksCompleted(state, result);
      }

      void PostHookBehavior::postHooksCompletedSlot(ServiceState *state, HookBase::HookResult result)
      {
        // UNDONE Можно тут кстати обработать разные выходы с хуков и вызвать либо фейли либо разные выходы
        if (result == HookBase::Continue)
          emit this->next(Finished, state);
        else if (result == HookBase::CheckUpdate)
          emit this->next(ReturnToStart, state);
        else
          emit failed(state);
      }

      void PostHookBehavior::setGameDownloaderService(GameDownloadService *gameDownloadService)
      {
        this->_gameDownloadService = gameDownloadService;
      }

      void PostHookBehavior::registerHook(const QString& serviceId, int preHookPriority, HookBase *hook)
      {
        this->_afterDownloadHookMap[serviceId].insert(-preHookPriority, hook);
      }

    }
  }
}