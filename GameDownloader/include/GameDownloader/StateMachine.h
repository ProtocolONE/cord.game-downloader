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

#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QMutex>

namespace GGS {
  namespace GameDownloader {

    class DOWNLOADSERVICE_EXPORT StateMachine : public QObject
    {
      Q_OBJECT
    public:
      explicit StateMachine(QObject *parent = 0);
      ~StateMachine();

      bool start(ServiceState *state);
      bool stop(ServiceState *state);

      void setStartBehavior(Behavior::BaseBehavior *behavior);
      void registerBehavior(Behavior::BaseBehavior *behavior);
      void setRoute(Behavior::BaseBehavior *from, int result, Behavior::BaseBehavior *to);

      qint64 timeForResume() const;
      void setTimeForResume(qint64 value);

    signals:
      void finished(GGS::GameDownloader::ServiceState *state);
      void failed(GGS::GameDownloader::ServiceState *state);
      void stopped(GGS::GameDownloader::ServiceState *state);

    private slots:
      void internalFinished(GGS::GameDownloader::ServiceState *state);
      void internalFailed(GGS::GameDownloader::ServiceState *state);
      void internalNext(int result, GGS::GameDownloader::ServiceState *state);

    private:
      QSet<Behavior::BaseBehavior*> _registeredBahavior;
      QMap<Behavior::BaseBehavior*,  QMap<int, Behavior::BaseBehavior*> > _routes;
      Behavior::BaseBehavior *_startBehavior;
      QMutex _mutex;
      qint64 _timeForResume;
      
      bool checkState(GGS::GameDownloader::ServiceState *state);
      bool canContinue(ServiceState * state);
    };

  }
}