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
#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>

#include <GameDownloader/Behavior/private/BindiffBehaviorPrivate.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>

namespace GGS {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT BindiffBehavior : public BaseBehavior
      {
        Q_OBJECT
      public:
        enum Results : int
        {
          Paused = 0,
          Finished = 1,
          CRCFailed = 2,
        };

        explicit BindiffBehavior(QObject *parent = 0);
        ~BindiffBehavior();

        virtual void start(GGS::GameDownloader::ServiceState *state) override;
        virtual void stop(GGS::GameDownloader::ServiceState *state) override;

      private slots:
        void xdeltaFinished(BindiffBehaviorPrivate* data);

        void run(BindiffBehaviorPrivate* data);

      private:
        bool apply(const QString& file, BindiffBehaviorPrivate* data);
      };

    }
  }
}
