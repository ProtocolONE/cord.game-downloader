#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QHash>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;
    class ExtractorBase;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT ExtractorBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1
        };

        explicit ExtractorBehavior(QObject *parent = 0);
        virtual ~ExtractorBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void registerExtractor(ExtractorBase *extractor);

      private slots:
        void extractionCompleted(P1::GameDownloader::ServiceState *state);
        void pauseRequestCompleted(P1::GameDownloader::ServiceState *state);
        void extractionFailed(P1::GameDownloader::ServiceState *state);
        void extractionProgressChanged(P1::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total);

      private:
        QHash<QString, ExtractorBase *> _extractorMap;

        ExtractorBase* getExtractorByType(const QString& type);
      };


    }
  }
}
