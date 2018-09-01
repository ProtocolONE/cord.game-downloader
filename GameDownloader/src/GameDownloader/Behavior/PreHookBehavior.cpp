#include <GameDownloader/Behavior/PreHookBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>

#include <QtConcurrent/QtConcurrentRun>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      PreHookBehavior::PreHookBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
        QObject::connect(this, SIGNAL(preHooksCompleted(P1::GameDownloader::ServiceState *, P1::GameDownloader::HookBase::HookResult)),
          SLOT(preHooksCompletedSlot(P1::GameDownloader::ServiceState *, P1::GameDownloader::HookBase::HookResult )));
      }

      PreHookBehavior::~PreHookBehavior(void)
      {
      }

      void PreHookBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        QtConcurrent::run(this, &PreHookBehavior::preHookLoop, state);
      }

      void PreHookBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
        emit this->stopping(state);
      }

      void PreHookBehavior::preHookLoop(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->preHooksCompleted(state, HookBase::Continue); // Походу надо будет урезать результаты хуков 
        }

        const P1::Core::Service *service = state->service();
        emit this->statusMessageChanged(state, QObject::tr("PRE_HOOK_DEFAULT_MESSAGE"));

        HookBase::HookResult result = HookBase::Continue;
        if (this->_beforeDownloadHookMap.contains(service->id())) {
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

      void PreHookBehavior::preHooksCompletedSlot(
        P1::GameDownloader::ServiceState *state, 
        P1::GameDownloader::HookBase::HookResult result)
      {
        // INFO Этот случай используется для интеграции сторонней системы скачивания
        // Вся логика по скачивания реализуется в PRE-хуке в том числе прогресс и статусы. 
        if (result == HookBase::Finish) {
          emit this->finished(state);
          return;
        }

        // INFO Помечаем сервис на остановку, и стейт машина остановить работу.
        if (result == HookBase::Abort) {
          state->setState(P1::GameDownloader::ServiceState::Stopping);
          emit this->next(Paused, state);
        }

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

        if (this->_registredHooks.contains(hook))
          return;

        this->_registredHooks.insert(hook);
        QObject::connect(this, &PreHookBehavior::stopping, hook, &HookBase::pauseRequestSlot);
      }

    }
  }
}