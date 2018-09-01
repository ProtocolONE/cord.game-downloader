#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QHash>

namespace P1 {
  namespace GameDownloader {

    class ServiceState;
    class ExtractorBase;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT SetAllPackedBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results: int
        {
          Paused = 0,
          Finished = 1
        };

        explicit SetAllPackedBehavior(QObject *parent = 0);
        ~SetAllPackedBehavior();

        virtual void start(P1::GameDownloader::ServiceState *state) override;
        virtual void stop(P1::GameDownloader::ServiceState *state) override;

        void registerExtractor(ExtractorBase *extractor);

      private slots:
        void internalUnpackStateSaved(P1::GameDownloader::ServiceState* state);
        void internalUnpackStateSaveFailed(P1::GameDownloader::ServiceState* state);

      private:
        QHash<QString, ExtractorBase *> _extractorMap;
        ExtractorBase* getExtractorByType(const QString& type);
      };

    }
  }
}
