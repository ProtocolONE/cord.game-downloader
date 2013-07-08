#include <GameDownloader/StateMachine.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <StateMachine/TestBehavior.h>
#include <TestEventLoopFinisher.h>
#include <SingleShotFunctor.h>
#include <Core/Service.h>

#include <QtCore/QEventLoop>
#include <QtTest/QSignalSpy>
#include <gtest/gtest.h>
#include <functional>

using namespace GGS::GameDownloader;
using Behavior::BaseBehavior;

#define CHECK_RESULTS(finish, fail, stopped) ASSERT_EQ(finish, spyFinish.count()); \
ASSERT_EQ(fail, spyFail.count()); \
ASSERT_EQ(stopped, spyStopped.count()); \

class StateMachineTest : public ::testing::Test {
public:
  StateMachineTest() 
    : killer(&loop, 6000)
    , spyFinish(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)))
    , spyFail(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)))
    , spyStopped(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)))
  {
  }

  void SetUp()
  {
    service.setId("123");
    service.setInstallPath("tmp");
    state.setState(ServiceState::Stopped);
    state.setService(&service);
    state.setStartType(Normal);

    killer.setTerminateSignal(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)));
    killer.setTerminateSignal(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)));
    killer.setTerminateSignal(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)));
  }

  StateMachine machine;
  ServiceState state;
  QEventLoop loop;
  TestEventLoopFinisher killer;
  GGS::Core::Service service;

  QSignalSpy spyFinish;
  QSignalSpy spyFail;
  QSignalSpy spyStopped;

};

TEST_F(StateMachineTest, RouteTest)
{
  TestBehavior start;
  TestBehavior point1;
  TestBehavior point2;
  TestBehavior finish;

  start.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callNext(10, state);
  });

  point1.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callNext(0, state);
  });

  point2.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callNext(42, state);
  });

  finish.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callFinished(state);
  });

  machine.registerBehavior(&start);
  machine.registerBehavior(&point1);
  machine.registerBehavior(&point2);
  machine.registerBehavior(&finish);

  machine.setStartBehavior(&start);

  machine.setRoute(&start, 10, &point1);
  machine.setRoute(&point1, 0, &point2);
  machine.setRoute(&point2, 42, &finish);

  machine.start(&state);
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(1, 0, 0);
}

TEST_F(StateMachineTest, FailTest)
{
  TestBehavior start;

  start.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callFailed(state);
  });

  machine.registerBehavior(&start);
  machine.setStartBehavior(&start);

  machine.start(&state);
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(0, 1, 0);
}

TEST_F(StateMachineTest, FinishTest)
{
  TestBehavior start;

  start.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callFinished(state);
  });

  machine.registerBehavior(&start);
  machine.setStartBehavior(&start);

  machine.start(&state);
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(1, 0, 0);
}

TEST_F(StateMachineTest, PauseTest)
{
  TestBehavior start;
  
  start.setBody([](TestBehavior *sender, ServiceState *state) {
    SingleShotFunctor *timeout = new SingleShotFunctor(50, [sender, state]() {
      sender->callFinished(state);
    }, sender);
  });

  machine.registerBehavior(&start);
  machine.setStartBehavior(&start);

  machine.start(&state);
  machine.stop(&state);
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(0, 0, 1);
  ASSERT_EQ(1, start.bodyCalled);
  ASSERT_EQ(1, start.stopCalled);
}

TEST_F(StateMachineTest, PauseRouteTest)
{
  TestBehavior start;
  TestBehavior finish;

  start.setBody([](TestBehavior *sender, ServiceState *state) {
    SingleShotFunctor *timeout = new SingleShotFunctor(100, [sender, state]() {
      sender->callNext(0, state);
    }, sender);
  });

  finish.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callFinished(state);
  });
  
  machine.registerBehavior(&start);
  machine.registerBehavior(&finish);
  machine.setStartBehavior(&start);
  machine.setRoute(&start, 0, &finish);

  machine.start(&state);
  
  SingleShotFunctor pauseCaller(50, [this]() {
    this->machine.stop(&this->state);
  });

  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(0, 0, 1);
  ASSERT_EQ(1, start.bodyCalled);
  ASSERT_EQ(1, start.stopCalled);

  ASSERT_EQ(0, finish.bodyCalled);
  ASSERT_EQ(0, finish.stopCalled);
}


TEST_F(StateMachineTest, PauseContinueRouteTest)
{
  TestBehavior start;
  TestBehavior finish;

  start.setBody([](TestBehavior *sender, ServiceState *state) {
    SingleShotFunctor *timeout = new SingleShotFunctor(100, [sender, state]() {
      sender->callNext(0, state);
    }, sender);
  });

  finish.setBody([](TestBehavior *sender, ServiceState *state) {
    sender->callFinished(state);
  });

  machine.registerBehavior(&start);
  machine.registerBehavior(&finish);
  machine.setStartBehavior(&start);
  machine.setRoute(&start, 0, &finish);

  machine.start(&state);

  SingleShotFunctor pauseCaller(50, [this]() {
    this->machine.stop(&this->state);
  });

  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(0, 0, 1);
  ASSERT_EQ(1, start.bodyCalled);
  ASSERT_EQ(1, start.stopCalled);

  ASSERT_EQ(0, finish.bodyCalled);
  ASSERT_EQ(0, finish.stopCalled);

  QEventLoop loop2;
  TestEventLoopFinisher killer2(&loop2, 6000);
  QSignalSpy spyFinish2(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)));
  QSignalSpy spyFail2(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)));
  QSignalSpy spyStopped2(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)));

  SingleShotFunctor startCaller(150, [this]() {
    machine.start(&state);
  });
  
  loop2.exec();

  ASSERT_FALSE(killer2.isKilledByTimeout());
  ASSERT_EQ(1, spyFinish2.count());
  ASSERT_EQ(0, spyFail2.count());
  ASSERT_EQ(0, spyStopped2.count());

  ASSERT_EQ(2, start.bodyCalled);
  ASSERT_EQ(1, start.stopCalled);

  ASSERT_EQ(1, finish.bodyCalled);
  ASSERT_EQ(0, finish.stopCalled);
}

TEST_F(StateMachineTest, PauseContinueRouteTest2)
{
  return; // Временн оотключен

  TestBehavior start;
  start.setId(1);
  TestBehavior finish;
  finish.setId(2);

  start.setBody([](TestBehavior *sender, ServiceState *state) {
    SingleShotFunctor *timeout = new SingleShotFunctor(10, [sender, state]() {
      sender->callNext(0, state);
    }, sender);
  });

  finish.setBody([](TestBehavior *sender, ServiceState *state) {
    SingleShotFunctor *timeout = new SingleShotFunctor(100, [sender, state]() {
      sender->callFinished(state);
    }, sender);
  });

  machine.registerBehavior(&start);
  machine.registerBehavior(&finish);
  machine.setStartBehavior(&start);
  machine.setRoute(&start, 0, &finish);

  machine.start(&state);

  SingleShotFunctor pauseCaller(50, [this]() {
    this->machine.stop(&this->state);
  });

  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(0, 0, 1);
  ASSERT_EQ(1, start.bodyCalled);
  ASSERT_EQ(0, start.stopCalled);

  ASSERT_EQ(1, finish.bodyCalled);
  ASSERT_EQ(1, finish.stopCalled);

  QEventLoop loop2;
  TestEventLoopFinisher killer2(&loop2, 6000);
  QSignalSpy spyFinish2(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)));
  QSignalSpy spyFail2(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)));
  QSignalSpy spyStopped2(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)));

  SingleShotFunctor startCaller(150, [this]() {
    machine.start(&state);
  });

  loop2.exec();

  ASSERT_FALSE(killer2.isKilledByTimeout());
  ASSERT_EQ(1, spyFinish2.count());
  ASSERT_EQ(0, spyFail2.count());
  ASSERT_EQ(0, spyStopped2.count());

  ASSERT_EQ(1, start.bodyCalled);
  ASSERT_EQ(0, start.stopCalled);

  ASSERT_EQ(2, finish.bodyCalled);
  ASSERT_EQ(1, finish.stopCalled);
}


TEST_F(StateMachineTest, PauseContinueRouteTest3)
{
  machine.setTimeForResume(50);

  TestBehavior start;
  start.setId(1);
  TestBehavior finish;
  finish.setId(2);

  start.setBody([](TestBehavior *sender, ServiceState *state) {
    SingleShotFunctor *timeout = new SingleShotFunctor(10, [sender, state]() {
      sender->callNext(0, state);
    }, sender);
  });

  finish.setBody([](TestBehavior *sender, ServiceState *state) {
    SingleShotFunctor *timeout = new SingleShotFunctor(500, [sender, state]() {
      sender->callFinished(state);
    }, sender);
  });

  machine.registerBehavior(&start);
  machine.registerBehavior(&finish);
  machine.setStartBehavior(&start);
  machine.setRoute(&start, 0, &finish);

  machine.start(&state);

  SingleShotFunctor pauseCaller(200, [this]() {
    this->machine.stop(&this->state);
  });

  loop.exec();
  qDebug() << QDateTime::currentMSecsSinceEpoch() << "First loop killed";
  ASSERT_FALSE(killer.isKilledByTimeout());
  CHECK_RESULTS(0, 0, 1);
  ASSERT_EQ(1, start.bodyCalled);
  ASSERT_EQ(0, start.stopCalled);

  ASSERT_EQ(1, finish.bodyCalled);
  ASSERT_EQ(1, finish.stopCalled);

  QEventLoop loop2;
  TestEventLoopFinisher killer2(&loop2, 6000);
  QSignalSpy spyFinish2(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)));
  QSignalSpy spyFail2(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)));
  QSignalSpy spyStopped2(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)));
  killer2.setTerminateSignal(&machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)));

  SingleShotFunctor startCaller(150, [this]() {
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "Second start called";
    machine.start(&state);
  });

  loop2.exec();
  qDebug() << QDateTime::currentMSecsSinceEpoch() << "Second loop killed";
  ASSERT_FALSE(killer2.isKilledByTimeout());
  ASSERT_EQ(1, spyFinish2.count());
  ASSERT_EQ(0, spyFail2.count());
  ASSERT_EQ(0, spyStopped2.count());

  ASSERT_EQ(2, start.bodyCalled);
  ASSERT_EQ(0, start.stopCalled);

  ASSERT_EQ(2, finish.bodyCalled);
  ASSERT_EQ(1, finish.stopCalled);
}