#include <GameDownloader/Behavior/CheckUpdateBehavior.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service.h>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      CheckUpdateBehavior::CheckUpdateBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      CheckUpdateBehavior::~CheckUpdateBehavior()
      {
      }

      void CheckUpdateBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);

        CheckUpdateHelper *helper = new CheckUpdateHelper(this);
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(result(P1::GameDownloader::ServiceState *, bool)),
          this, SLOT(checkUpdateResult(P1::GameDownloader::ServiceState *, bool)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(P1::GameDownloader::ServiceState *)),
          this, SLOT(checkUpdateError(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(
          helper, 
          SIGNAL(checkUpdateProgressChanged(P1::GameDownloader::ServiceState *, quint8)),
          this, 
          SLOT(checkUpdateProgressChanged(P1::GameDownloader::ServiceState *, quint8))));

        helper->startCheck(state, CheckUpdateHelper::Normal);
      }

      void CheckUpdateBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
        // we can't stop right now.
        // let's just wait a little bit.
        // Or may be we must find a way to stop.
      }

      void CheckUpdateBehavior::checkUpdateResult(P1::GameDownloader::ServiceState *state, bool isUpdated)
      {
        CheckUpdateHelper *helper = qobject_cast<CheckUpdateHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        state->setIsFoundNewUpdate(isUpdated);
        emit this->next(Finished, state);
      }

      void CheckUpdateBehavior::checkUpdateError(P1::GameDownloader::ServiceState *state)
      {
        CheckUpdateHelper *helper = qobject_cast<CheckUpdateHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->failed(state);
      }

      void CheckUpdateBehavior::checkUpdateProgressChanged(P1::GameDownloader::ServiceState *state, quint8 progress)
      {
        emit this->totalProgressChanged(state, progress);
      }

    }
  }
}