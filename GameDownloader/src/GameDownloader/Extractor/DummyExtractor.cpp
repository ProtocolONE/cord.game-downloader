#include <GameDownloader/Extractor/DummyExtractor.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service.h>

namespace P1 {
  namespace GameDownloader {
    namespace Extractor {

      DummyExtractor::DummyExtractor(QObject *parent)
        : P1::GameDownloader::ExtractorBase("3A3AC78E-0332-45F4-A466-89C2B8E8BB9C", parent)
      {
      }

      DummyExtractor::~DummyExtractor()
      {
      }

      void DummyExtractor::extract(P1::GameDownloader::ServiceState* state, StartType startType)
      {
        Q_CHECK_PTR(state);
        emit this->extractionProgressChanged(state, 100, 0, 0);
        emit this->extractFinished(state);
      }

      void DummyExtractor::compress(P1::GameDownloader::ServiceState* state)
      {
        Q_CHECK_PTR(state);
        state->setPackingFiles(QStringList());
        emit this->compressProgressChanged(state, 100, 0, 0);
        emit this->compressFinished(state);
      }

      void DummyExtractor::setAllUnpacked(ServiceState* state)
      {
        emit this->unpackStateSaved(state);
      }

    }
  }
}