/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/CheckUnfinishedPackBehavior.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service.h>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      CheckUnfinishedPackBehavior::CheckUnfinishedPackBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      CheckUnfinishedPackBehavior::~CheckUnfinishedPackBehavior()
      {
      }

      void CheckUnfinishedPackBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());
        
        if (state->packingFiles().isEmpty()) {
          emit this->next(AllPacked, state);
          return;
        }

        emit this->next(NotAllPacked, state);
      }

      void CheckUnfinishedPackBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
      }
    }
  }
}