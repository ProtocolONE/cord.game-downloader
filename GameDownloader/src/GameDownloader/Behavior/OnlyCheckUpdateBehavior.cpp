#include <GameDownloader/Behavior/OnlyCheckUpdateBehavior.h>
#include <GameDownloader/ServiceState.h>

#include <GameDownloader/Behavior/Private/CheckTorrentModify.h>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      OnlyCheckUpdateBehavior::OnlyCheckUpdateBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      OnlyCheckUpdateBehavior::~OnlyCheckUpdateBehavior()
      {
      }

      void OnlyCheckUpdateBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);

        CheckTorrentModify * helper = new CheckTorrentModify(this);
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(result(P1::GameDownloader::ServiceState *, bool)),
          this, SLOT(checkUpdateResult(P1::GameDownloader::ServiceState *, bool)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(P1::GameDownloader::ServiceState *)),
          this, SLOT(checkUpdateError(P1::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        helper->start(state);
      }

      void OnlyCheckUpdateBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
      }

      void OnlyCheckUpdateBehavior::checkUpdateResult(P1::GameDownloader::ServiceState *state, bool isModified)
      {
        CheckTorrentModify *helper = qobject_cast<CheckTorrentModify *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        state->setIsFoundNewUpdate(isModified);
        state->setTorrentLastModifedDate(helper->lastModified());

        emit this->next(isModified ? UpdateFound : UpdateNotFound, state);
      }

      void OnlyCheckUpdateBehavior::checkUpdateError(P1::GameDownloader::ServiceState *state)
      {
        CheckTorrentModify *helper = qobject_cast<CheckTorrentModify *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->failed(state);
      }

    }
  }
}