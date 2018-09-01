#pragma once

#include <GameDownloader/GameDownloader_global.h>

#include <QtCore/QObject>
#include <QtCore/QString>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT BaseBehavior : public QObject
      {
        Q_OBJECT
      public:
        explicit BaseBehavior(QObject *parent = 0);
        virtual ~BaseBehavior();

      public slots:
        virtual void start(P1::GameDownloader::ServiceState *state) = 0;
        virtual void stop(P1::GameDownloader::ServiceState *state) = 0;

      signals:
        void next(int result, P1::GameDownloader::ServiceState *state);

        void finished(P1::GameDownloader::ServiceState *state);
        void failed(P1::GameDownloader::ServiceState *state);

        void totalProgressChanged(P1::GameDownloader::ServiceState *state, qint8 progress);
        void statusMessageChanged(P1::GameDownloader::ServiceState *state, const QString& message);
      };

    }
  }
}