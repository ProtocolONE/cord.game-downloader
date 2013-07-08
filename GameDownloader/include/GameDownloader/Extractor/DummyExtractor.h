/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/ExtractorBase.h>

namespace GGS {
  namespace GameDownloader {
    namespace Extractor {

      class DOWNLOADSERVICE_EXPORT DummyExtractor : public GGS::GameDownloader::ExtractorBase
      {
        Q_OBJECT
      public:
        explicit DummyExtractor(QObject *parent = 0);
        ~DummyExtractor();

        virtual void extract(GGS::GameDownloader::ServiceState* state, StartType startType) override;
        virtual void compress(GGS::GameDownloader::ServiceState* state) override;
        virtual void setAllUnpacked(ServiceState* state) override;

      };

    }
  }
}
