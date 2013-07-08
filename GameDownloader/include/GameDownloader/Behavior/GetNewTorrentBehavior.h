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

#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/Behavior/BaseBehavior.h>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class GetNewTorrentBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Downloaded = 1
        };

        explicit GetNewTorrentBehavior(QObject *parent = 0);
        virtual ~GetNewTorrentBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

      signals:
        void progressChanged(const QString& serviceId, quint8 progress);

      private slots:
        void internalFinished(GGS::GameDownloader::ServiceState *state);
        void internalError(GGS::GameDownloader::ServiceState *state);

      };

    }
  }
}