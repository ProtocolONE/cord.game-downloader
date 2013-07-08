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
  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT GetPatchVersionBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          PatchFound = 1,
          PatchNotFound = 2
        };

        explicit GetPatchVersionBehavior(QObject *parent = 0);
        virtual ~GetPatchVersionBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;
        void setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper);

      private slots:
        void internalFinished(GGS::GameDownloader::ServiceState *state);
        void internalError(GGS::GameDownloader::ServiceState *state);

        void internalPatchFinished(GGS::GameDownloader::ServiceState *state);
        void internalPatchError(GGS::GameDownloader::ServiceState *state);

        void checkPatchFound(GGS::GameDownloader::ServiceState *state);
        void checkPatchNotFound(GGS::GameDownloader::ServiceState *state);

      private:
        GGS::Libtorrent::Wrapper *_wrapper;
        
        void checkPatchExist(GGS::GameDownloader::ServiceState *state);
        QString getServicePatchUrl(GGS::GameDownloader::ServiceState *state);
      };

    }
  }
}