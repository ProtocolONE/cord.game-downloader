/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2015, Syncopate Limited and/or affiliates.
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
  namespace Libtorrent {
    class Wrapper;
  }

  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT UninstallBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1,
          ContinueInstall = 2
        };

        explicit UninstallBehavior(QObject *parent = 0);
        virtual ~UninstallBehavior();

        void setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper);

        virtual void start(GGS::GameDownloader::ServiceState *state) override;  
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

      private:
        void uninstall(GGS::GameDownloader::ServiceState *state);
        QStringList getDirectoriesToRemove(const GGS::Core::Service *service);
        int getFilesCount(const QString &directory);

        GGS::Libtorrent::Wrapper *_wrapper;
      };
    }
  }
}
