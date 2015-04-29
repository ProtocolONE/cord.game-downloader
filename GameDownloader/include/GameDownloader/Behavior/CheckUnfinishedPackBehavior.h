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

namespace GGS {
  namespace GameDownloader {

    class ServiceState;
    class ExtractorBase;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT CheckUnfinishedPackBehavior: public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results: int
        {
          AllPacked = 0,
          NotAllPacked = 1
        };

        explicit CheckUnfinishedPackBehavior(QObject *parent = 0);
        virtual ~CheckUnfinishedPackBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;
      };

    }
  }
}
