#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace P1 {
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

        void setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper);

        virtual void start(P1::GameDownloader::ServiceState *state) override;  
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

      private:
        void uninstall(P1::GameDownloader::ServiceState *state);

        void removeEmptyFolders(const P1::Core::Service * service);

        QStringList getDirectoriesToRemove(const P1::Core::Service *service);
        int getFilesCount(const QString &directory);

        P1::Libtorrent::Wrapper *_wrapper;
      };
    }
  }
}
