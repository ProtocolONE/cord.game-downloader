/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <UpdateSystem/UpdateInfoGetterResultInterface>
#include <QtCore/QObject>
#include <QtCore/QEventLoop>
#include <QtCore/QPointer>

namespace GGS {
  namespace GameDownloader {
    namespace Extractor {

      class UpdateInfoGetterResultEventLoopKiller : public QObject,
        public GGS::UpdateSystem::UpdateInfoGetterResultInterface
      {
        Q_OBJECT
      public:
        explicit UpdateInfoGetterResultEventLoopKiller(QObject *parent = 0);
        ~UpdateInfoGetterResultEventLoopKiller();

        void updateInfoCallback(GGS::UpdateSystem::UpdateInfoGetterResults error);
        void infoGetterUpdateProggress(quint64 current, quint64 total);
        
        void setEventLoop(QEventLoop *loop);
        GGS::UpdateSystem::UpdateInfoGetterResults result() const;

      private:
        QPointer<QEventLoop> _loop;
        GGS::UpdateSystem::UpdateInfoGetterResults _result;
      };

    }
  }
}
