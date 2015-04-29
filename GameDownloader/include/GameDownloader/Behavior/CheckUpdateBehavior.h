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
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT CheckUpdateBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1
        };

        explicit CheckUpdateBehavior(QObject *parent = 0);
        virtual ~CheckUpdateBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;  
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

      private slots:
        void checkUpdateResult(GGS::GameDownloader::ServiceState *state, bool isUpdated);
        void checkUpdateError(GGS::GameDownloader::ServiceState *state);
        void checkUpdateProgressChanged(GGS::GameDownloader::ServiceState *state, quint8 progress);

      };
    }
  }
}
