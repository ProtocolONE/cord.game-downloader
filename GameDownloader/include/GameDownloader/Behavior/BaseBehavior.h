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

#include <QtCore/QObject>
#include <QtCore/QString>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT BaseBehavior : public QObject
      {
        Q_OBJECT
      public:
        explicit BaseBehavior(QObject *parent = 0);
        ~BaseBehavior();

      public slots:
        virtual void start(GGS::GameDownloader::ServiceState *state) = 0;
        virtual void stop(GGS::GameDownloader::ServiceState *state) = 0;

      signals:
        void next(int result, GGS::GameDownloader::ServiceState *state);

        void finished(GGS::GameDownloader::ServiceState *state);
        void failed(GGS::GameDownloader::ServiceState *state);

        void totalProgressChanged(GGS::GameDownloader::ServiceState *state, qint8 progress);
        void statusMessageChanged(GGS::GameDownloader::ServiceState *state, const QString& message);
      };

    }
  }
}