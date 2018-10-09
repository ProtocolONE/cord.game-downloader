/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (c) 2011 - 2015, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QObject>

namespace GGS {
  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT CreateFastResumeBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Created = 1
        };

        explicit CreateFastResumeBehavior(QObject *parent = 0);
        virtual ~CreateFastResumeBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

        void setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper);
      private:
        GGS::Libtorrent::Wrapper *_wrapper;
      };
    }
  }
}