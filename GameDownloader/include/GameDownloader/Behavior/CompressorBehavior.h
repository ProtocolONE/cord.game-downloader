#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QHash>

namespace P1 {
  namespace GameDownloader {

    class ServiceState;
    class ExtractorBase;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT CompressorBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results: int
        {
          Paused = 0,
          Finished = 1
        };

        explicit CompressorBehavior(QObject *parent = 0);
        virtual ~CompressorBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void registerCompressor(ExtractorBase *extractor);

      private slots:
        void compressCompleted(P1::GameDownloader::ServiceState *state);
        void pauseRequestCompleted(P1::GameDownloader::ServiceState *state);
        void compressFailed(P1::GameDownloader::ServiceState *state);
        void compressProgressChanged(
          P1::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total);

      private:
        QHash<QString, ExtractorBase *> _extractorMap;

        ExtractorBase* getExtractorByType(const QString& type);
      };


    }
  }
}
