#include <GameDownloader/StateMachine.h>

#include <Core/Service.h>

#include <QtCore/QMutexLocker>
#include <QtCore/QMetaObject>
#include <QtCore/QtGlobal>
#include <QtCore/QDateTime>

#define CHECK_REGISTER_BEHAVIOR(X) \
  Q_ASSERT_X(this->_registeredBahavior.contains(X), __FUNCTION__ , X->metaObject()->className());

#define CHECK_NOT_REGISTER_BEHAVIOR(X) \
  Q_ASSERT_X(!this->_registeredBahavior.contains(X), __FUNCTION__ , X->metaObject()->className());

namespace P1 {
  namespace GameDownloader {
    using Behavior::BaseBehavior;

    // UNDONE в тесте не проверил что после любой остановки д олжен быть стейт STOPPED 
    StateMachine::StateMachine(QObject *parent)
      : QObject(parent)
      , _startBehavior(0)
      , _timeForResume(600000)
    {
    }

    StateMachine::~StateMachine()
    {
    }

    qint64 StateMachine::timeForResume() const
    {
      return this->_timeForResume;
    }

    void StateMachine::setTimeForResume(qint64 value)
    {
      this->_timeForResume = value;
    }

    bool StateMachine::start(ServiceState *state)
    {      
      Q_CHECK_PTR(this->_startBehavior);

      QMutexLocker lock(&this->_mutex);

      if (state->state() != ServiceState::Stopped) {
        DEBUG_LOG << "Service " << state->id() 
          << "in progress " << state->currentBehavior()->metaObject()->className();
        return false;
      }

      BaseBehavior *next = this->canContinue(state) ? state->currentBehavior() : this->_startBehavior;
      state->setState(ServiceState::Started);

      state->setCurrentBehavior(next);
      DEBUG_LOG << "Start " << state->id() << next->metaObject()->className();
      QMetaObject::invokeMethod(
        next, 
        "start", 
        Qt::QueuedConnection, 
        Q_ARG(P1::GameDownloader::ServiceState *, state));

      return true;
    }

    bool StateMachine::stop(ServiceState *state)
    {
      QMutexLocker lock(&this->_mutex);
      if (state->state() != ServiceState::Started) {
        DEBUG_LOG << "Service in not progress ";
        return false;
      }

      state->setState(ServiceState::Stopping);
      QMetaObject::invokeMethod(
        state->currentBehavior(), 
        "stop", 
        Qt::QueuedConnection, 
        Q_ARG(P1::GameDownloader::ServiceState *, state));

      return true;
    }

    void StateMachine::internalFinished(ServiceState *state)
    {
      QMutexLocker lock(&this->_mutex);
      BaseBehavior *sender = qobject_cast<BaseBehavior *>(QObject::sender());
      if (!sender || sender != state->currentBehavior())
        return;

      if (!this->checkState(state))
        return;

      DEBUG_LOG << "Finished " << state->id() << sender->metaObject()->className();
      state->setState(ServiceState::Stopped);
      state->resetLastDateStateChanged();
      emit this->finished(state);
    }

    void StateMachine::internalFailed(ServiceState *state)
    {
      QMutexLocker lock(&this->_mutex);
      BaseBehavior *sender = qobject_cast<BaseBehavior *>(QObject::sender());
      if (!sender || sender != state->currentBehavior())
        return;

      state->setDownloadSuccess(false);

      if (!this->checkState(state))
        return;

      CRITICAL_LOG << "Failed " << state->id() << sender->metaObject()->className();
      state->setState(ServiceState::Stopped);
      state->resetLastDateStateChanged();
      emit this->failed(state);
    }

    void StateMachine::internalNext(int result, ServiceState *state)
    {
      QMutexLocker lock(&this->_mutex);

      BaseBehavior *sender = qobject_cast<BaseBehavior *>(QObject::sender());
      if (!sender || sender != state->currentBehavior())
        return;

      if (!this->checkState(state))
        return;

      if (!this->_routes.contains(sender) || !this->_routes[sender].contains(result)) {
        CRITICAL_LOG << "No next route failed " << sender->metaObject()->className();
        state->setState(ServiceState::Stopped);
        emit this->failed(state);
        return;
      }

      BaseBehavior* next = this->_routes[sender][result];
      DEBUG_LOG << "Call " << state->id() << next->metaObject()->className();
      state->setCurrentBehavior(next);
      QMetaObject::invokeMethod(
        next, 
        "start", 
        Qt::QueuedConnection, 
        Q_ARG(P1::GameDownloader::ServiceState *, state));
    }

    void StateMachine::setRoute(Behavior::BaseBehavior *from, int result, Behavior::BaseBehavior *to)
    {
      Q_ASSERT(!this->_routes[from].contains(result));
      CHECK_REGISTER_BEHAVIOR(from);
      CHECK_REGISTER_BEHAVIOR(to);
      this->_routes[from][result] = to;
    }

    void StateMachine::registerBehavior(Behavior::BaseBehavior *behavior)
    {
      Q_CHECK_PTR(behavior);
      CHECK_NOT_REGISTER_BEHAVIOR(behavior);

      QObject::connect(behavior, &Behavior::BaseBehavior::finished, this, &StateMachine::internalFinished, Qt::QueuedConnection);
      QObject::connect(behavior, &Behavior::BaseBehavior::failed, this, &StateMachine::internalFailed, Qt::QueuedConnection);
      QObject::connect(behavior, &Behavior::BaseBehavior::next, this, &StateMachine::internalNext, Qt::QueuedConnection);
        
      this->_registeredBahavior.insert(behavior);
    }

    void StateMachine::setStartBehavior(Behavior::BaseBehavior *behavior)
    {
      CHECK_REGISTER_BEHAVIOR(behavior);
      this->_startBehavior = behavior;
    }

    bool StateMachine::checkState(P1::GameDownloader::ServiceState *state)
    {
      if (state->state() == ServiceState::Stopping) {
        state->setState(ServiceState::Stopped);
        emit this->stopped(state);
        return false;
      }

      return state->state() == ServiceState::Started;
    }

    bool StateMachine::canContinue(ServiceState * state)
    {
      if (!state->currentBehavior())
        return false;

      StartType startType = state->startType();
      if (startType == Force || startType == Recheck || startType == Uninstall)
        return false;

      qint64 elapsedTime = QDateTime::currentMSecsSinceEpoch() - state->lastDateStateChanged();
      return (elapsedTime < 0 || elapsedTime > this->_timeForResume) ? false : true;
    }
  }
}