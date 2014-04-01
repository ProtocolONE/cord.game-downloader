/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/PauseRequestWatcher.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service.h>

#include <QtCore/QDebug>

namespace GGS {
  namespace GameDownloader {

    PauseRequestWatcher::PauseRequestWatcher(const GGS::Core::Service *service)
    {
      Q_CHECK_PTR(service);

      this->_id = service->id();
      this->_isPaused = false;
    }

    PauseRequestWatcher::~PauseRequestWatcher()
    {
    }

    void PauseRequestWatcher::pauseRequestSlot(const GGS::Core::Service *service)
    {
      Q_CHECK_PTR(service);

      if (this->_id != service->id())
        return;

      this->_isPaused = true;
    }

    bool PauseRequestWatcher::isPaused() const
    {
      return this->_isPaused;
    }

    void PauseRequestWatcher::pauseRequestSlot2(ServiceState *state)
    {
      Q_CHECK_PTR(state);
      Q_CHECK_PTR(state->service());

      if (this->_id != state->service()->id())
        return;

      this->_isPaused = true;
    }

  }
}