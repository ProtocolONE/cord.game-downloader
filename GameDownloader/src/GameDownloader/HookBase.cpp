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

GGS::GameDownloader::HookBase::~HookBase()
{

}

GGS::GameDownloader::HookBase::HookBase(const QString& hookId, QObject *parent)
  : QObject(parent)
  , _hookId(hookId)
{
}

const QString& GGS::GameDownloader::HookBase::hookId()
{
  return this->_hookId;
}

quint8 GGS::GameDownloader::HookBase::beforeProgressWeight() const
{
  return this->_beforeProgressWeight;
}

void GGS::GameDownloader::HookBase::setBeforeProgressWeight(quint8 weight)
{
  this->_beforeProgressWeight = weight;
}

quint8 GGS::GameDownloader::HookBase::afterProgressWeight() const
{
  return this->_afterProgressWeight;
}

void GGS::GameDownloader::HookBase::setAfterProgressWeight(quint8 weight)
{
  this->_afterProgressWeight = weight;
}
