#pragma once

#include <GameDownloader/GameDownloader_global.h>

#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QMutex>

namespace P1 {
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
      void finished(P1::GameDownloader::ServiceState *state);
      void failed(P1::GameDownloader::ServiceState *state);
      void stopped(P1::GameDownloader::ServiceState *state);

    private slots:
      void internalFinished(P1::GameDownloader::ServiceState *state);
      void internalFailed(P1::GameDownloader::ServiceState *state);
      void internalNext(int result, P1::GameDownloader::ServiceState *state);

    private:
      QSet<Behavior::BaseBehavior*> _registeredBahavior;
      QMap<Behavior::BaseBehavior*,  QMap<int, Behavior::BaseBehavior*> > _routes;
      Behavior::BaseBehavior *_startBehavior;
      QMutex _mutex;
      qint64 _timeForResume;
      
      bool checkState(P1::GameDownloader::ServiceState *state);
      bool canContinue(ServiceState * state);
    };

  }
}