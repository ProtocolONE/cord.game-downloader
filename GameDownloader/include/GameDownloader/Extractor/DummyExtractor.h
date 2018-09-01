#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/ExtractorBase.h>

namespace P1 {
  namespace GameDownloader {
    namespace Extractor {

      class DOWNLOADSERVICE_EXPORT DummyExtractor : public P1::GameDownloader::ExtractorBase
      {
        Q_OBJECT
      public:
        explicit DummyExtractor(QObject *parent = 0);
        ~DummyExtractor();

        virtual void extract(P1::GameDownloader::ServiceState* state, StartType startType) override;
        virtual void compress(P1::GameDownloader::ServiceState* state) override;
        virtual void setAllUnpacked(ServiceState* state) override;

      };

    }
  }
}
