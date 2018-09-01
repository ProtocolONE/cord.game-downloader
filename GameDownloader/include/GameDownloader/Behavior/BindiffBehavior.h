#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>
#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>

#include <GameDownloader/Behavior/private/BindiffBehaviorPrivate.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>

namespace P1 {
  namespace Core {
    class Service;
  }

  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT BindiffBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1,
          CRCFailed = 2,
        };

        explicit BindiffBehavior(QObject *parent = 0);
        virtual ~BindiffBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void setTorrentWrapper(P1::Libtorrent::Wrapper * value);

      private slots:
        void xdeltaFinished(BindiffBehaviorPrivate* data);

        void run(BindiffBehaviorPrivate* data);

      private:
        bool apply(const QString& file, BindiffBehaviorPrivate* data);
        P1::Libtorrent::Wrapper *_wrapper;
      };

    }
  }
}
