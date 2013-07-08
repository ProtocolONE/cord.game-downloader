/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/CheckUpdateBehavior.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      CheckUpdateBehavior::CheckUpdateBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      CheckUpdateBehavior::~CheckUpdateBehavior()
      {
      }

      void CheckUpdateBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);

        CheckUpdateHelper *helper = new CheckUpdateHelper(this);
        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(result(GGS::GameDownloader::ServiceState *, bool)),
          this, SLOT(checkUpdateResult(GGS::GameDownloader::ServiceState *, bool)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(helper, SIGNAL(error(GGS::GameDownloader::ServiceState *)),
          this, SLOT(checkUpdateError(GGS::GameDownloader::ServiceState *)),
          Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(
          helper, 
          SIGNAL(checkUpdateProgressChanged(GGS::GameDownloader::ServiceState *, quint8)),
          this, 
          SLOT(checkUpdateProgressChanged(GGS::GameDownloader::ServiceState *, quint8))));

        helper->startCheck(state, CheckUpdateHelper::Normal);
      }

      void CheckUpdateBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
        // we can't stop right now.
        // let's just wait a little bit.
        // Or may be we must find a way to stop.
      }

      void CheckUpdateBehavior::checkUpdateResult(GGS::GameDownloader::ServiceState *state, bool isUpdated)
      {
        CheckUpdateHelper *helper = qobject_cast<CheckUpdateHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        state->setIsFoundNewUpdate(isUpdated);
        emit this->next(Finished, state);
      }

      void CheckUpdateBehavior::checkUpdateError(GGS::GameDownloader::ServiceState *state)
      {
        CheckUpdateHelper *helper = qobject_cast<CheckUpdateHelper *>(QObject::sender());
        if (!helper)
          return;

        helper->deleteLater();
        emit this->failed(state);
      }

      void CheckUpdateBehavior::checkUpdateProgressChanged(GGS::GameDownloader::ServiceState *state, quint8 progress)
      {
        emit this->totalProgressChanged(state, progress);
      }

    }
  }
}