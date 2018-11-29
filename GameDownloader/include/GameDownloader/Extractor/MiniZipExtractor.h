#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/Extractor/GameExtractor.h>
#include <UpdateSystem/UpdateFileInfo.h>

namespace P1 {
  namespace GameDownloader {

    namespace Extractor {
      class DOWNLOADSERVICE_EXPORT MiniZipExtractor : public GameExtractor
      {
        Q_OBJECT
      public:
        explicit MiniZipExtractor(QObject *parent = 0);
        
        virtual void extract(P1::GameDownloader::ServiceState* state, StartType startType) override;
        virtual void compress(P1::GameDownloader::ServiceState* state) override;
        virtual void setAllUnpacked(ServiceState* state) override;
      };
    }
  }
}
