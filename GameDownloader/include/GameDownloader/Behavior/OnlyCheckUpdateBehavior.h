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
#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class OnlyCheckUpdateBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          UpdateFound = 1,
          UpdateNotFound = 2,
        };

        explicit OnlyCheckUpdateBehavior(QObject *parent = 0);
        ~OnlyCheckUpdateBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

      private slots:
        void checkUpdateResult(GGS::GameDownloader::ServiceState *state, bool isModified);
        void checkUpdateError(GGS::GameDownloader::ServiceState *state);
      };


    }
  }
}
