/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (�) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QHash>

namespace GGS {
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
        ~ExtractorBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

        void registerExtractor(ExtractorBase *extractor);

      private slots:
        void extractionCompleted(GGS::GameDownloader::ServiceState *state);
        void pauseRequestCompleted(GGS::GameDownloader::ServiceState *state);
        void extractionFailed(GGS::GameDownloader::ServiceState *state);
        void extractionProgressChanged(GGS::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total);

      private:
        QHash<QString, ExtractorBase *> _extractorMap;

        ExtractorBase* getExtractorByType(const QString& type);
      };


    }
  }
}
