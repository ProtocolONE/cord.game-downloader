#include <GameDownloader/Behavior/PostHookBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>

#include <QtConcurrent/QtConcurrentRun>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      PostHookBehavior::PostHookBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
        QObject::connect(this, SIGNAL(postHooksCompleted(P1::GameDownloader::ServiceState *, P1::GameDownloader::HookBase::HookResult)),
          SLOT(postHooksCompletedSlot(P1::GameDownloader::ServiceState *, P1::GameDownloader::HookBase::HookResult )));
      }

      PostHookBehavior::~PostHookBehavior(void)
      {
      }

      void PostHookBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        QtConcurrent::run(this, &PostHookBehavior::postHookLoop, state);
      }

      void PostHookBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
        emit this->stopping(state);
      }

      void PostHookBehavior::postHookLoop(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->postHooksCompleted(state, HookBase::Continue); // Походу надо будет урезать результаты хуков 
        }

        const P1::Core::Service *service = state->service();
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