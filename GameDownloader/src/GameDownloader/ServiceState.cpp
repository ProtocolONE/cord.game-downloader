/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/ServiceState.h>
#include <Core/Service>

#include <QtCore/QDateTime>

namespace GGS {
  namespace GameDownloader {

    ServiceState::ServiceState(QObject *parent) 
      : QObject(parent)
      , _stage(Nowhere)
      , _state(Unknown)
      , _lastDateStateChanded(-1)
      , _isDirectoryChanged(false)
    {
    }

    ServiceState::~ServiceState()
    {
    }

    const QString& ServiceState::id() const
    {
      Q_ASSERT(this->_service);
      return this->_service->id();
    }

    void ServiceState::setState(const State state)
    {
      this->_state = state;
      this->_lastDateStateChanded = QDateTime::currentMSecsSinceEpoch();
    }

    GGS::GameDownloader::ServiceState::State ServiceState::state() const
    {
      return this->_state;
    }

    void ServiceState::setStage(const Stage stage)
    {
      this->_stage = stage;
    }

    GGS::GameDownloader::ServiceState::Stage ServiceState::stage() const
    {
      return this->_stage;
    }

    qint64 ServiceState::lastDateStateChanged() const
    {
      return this->_lastDateStateChanded;
    }

    void ServiceState::setStartType(StartType type)
    {
      this->_startType = type;
    }

    StartType ServiceState::startType() const
    {
      return this->_startType;
    }

    bool ServiceState::isFoundNewUpdate() const
    {
      return this->_isFoundNewUpdate;
    }

    void ServiceState::setIsFoundNewUpdate(bool isFoundNewUpdate)
    {
      this->_isFoundNewUpdate = isFoundNewUpdate;
    }

    void ServiceState::setService(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      this->_service = service;
    }

    const GGS::Core::Service* ServiceState::service() const
    {
      return this->_service;
    }

    bool ServiceState::isDirectoryChanged() const
    {
      return this->_isDirectoryChanged;
    }

    void ServiceState::setIsDirectoryChanged(bool isDirectoryChanged)
    {
      this->_isDirectoryChanged = isDirectoryChanged;
    }

  }
}