/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Extractor/DummyExtractor.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service>

namespace GGS {
  namespace GameDownloader {
    namespace Extractor {

      DummyExtractor::DummyExtractor(QObject *parent)
        : GGS::GameDownloader::ExtractorBase("3A3AC78E-0332-45F4-A466-89C2B8E8BB9C", parent)
      {
      }

      DummyExtractor::~DummyExtractor()
      {
      }

      void DummyExtractor::extract(GGS::GameDownloader::ServiceState* state, StartType startType)
      {
        Q_CHECK_PTR(state);
        emit this->extractionProgressChanged(state, 100, 0, 0);
        emit this->extractFinished(state);
      }

      void DummyExtractor::compress(GGS::GameDownloader::ServiceState* state)
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