/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (ñ) 2011 - 2017, Syncopate Limited and/or affiliates.
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
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT ReadOnlyBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1
        };

        explicit ReadOnlyBehavior(QObject *parent = 0);
        virtual ~ReadOnlyBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

        void keepCalmMessage(GGS::GameDownloader::ServiceState *state);
        void processFinished(int result, GGS::GameDownloader::ServiceState *state);

      signals:
        void scanFolders(const QStringList & folders, GGS::GameDownloader::ServiceState *state);
      };

    }
  }
}
