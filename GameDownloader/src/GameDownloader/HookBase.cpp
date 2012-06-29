/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/HookBase.h>
#include <GameDownloader/GameDownloadService>

namespace GGS {
  namespace GameDownloader {
    HookBase::~HookBase()
    {
    }

    HookBase::HookBase(const QString& hookId, QObject *parent)
      : QObject(parent)
      , _hookId(hookId)
      , _beforeProgressWeight(0)
      , _afterProgressWeight(0)
    {
    }

    const QString& HookBase::hookId()
    {
      return this->_hookId;
    }

    quint8 HookBase::beforeProgressWeight() const
    {
      return this->_beforeProgressWeight;
    }

    void HookBase::setBeforeProgressWeight(quint8 weight)
    {
      this->_beforeProgressWeight = weight;
    }

    quint8 HookBase::afterProgressWeight() const
    {
      return this->_afterProgressWeight;
    }

    void HookBase::setAfterProgressWeight(quint8 weight)
    {
      this->_afterProgressWeight = weight;
    }

    void HookBase::pauseRequestSlot(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      emit this->pauseRequest(service);
    }

  }
}