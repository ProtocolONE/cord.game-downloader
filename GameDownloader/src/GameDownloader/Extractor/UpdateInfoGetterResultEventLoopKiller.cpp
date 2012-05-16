/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Extractor/UpdateInfoGetterResultEventLoopKiller.h>

namespace GGS {
  namespace GameDownloader {
    namespace Extractor {

      UpdateInfoGetterResultEventLoopKiller::UpdateInfoGetterResultEventLoopKiller(QObject *parent)
        : QObject(parent)
        , _loop(0)
      {
      }

      UpdateInfoGetterResultEventLoopKiller::~UpdateInfoGetterResultEventLoopKiller()
      {
      }

      void UpdateInfoGetterResultEventLoopKiller::updateInfoCallback(GGS::UpdateSystem::UpdateInfoGetterResults error)
      {
        Q_ASSERT(this->_loop != 0);
        this->_result = error;
        this->_loop->exit();
      }

      void UpdateInfoGetterResultEventLoopKiller::infoGetterUpdateProggress(quint64 current, quint64 total)
      {
      }

      void UpdateInfoGetterResultEventLoopKiller::setEventLoop(QEventLoop *loop)
      {
        this->_loop = loop;
      }

      GGS::UpdateSystem::UpdateInfoGetterResults UpdateInfoGetterResultEventLoopKiller::result() const
      {
        return this->_result;
      }

    }
  }
}