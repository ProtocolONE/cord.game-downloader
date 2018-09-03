#include <gtest/gtest.h>

#include <Core/Service.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/Extractor/DummyExtractor.h>

#include "SimpleContinueHook.h"
#include "TestEventLoopFinisher.h"
#include "FakeLibtorrentWrapperAdapter.h"
#include "FakeCheckUpdateAdapter.h"

#include "LambdaHook.h"
#include "SingleShotFunctor.h"
#include "SignalCounter.h"

#include <QtCore/QObject>
#include <QtCore/QEventLoop>
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtTest/QTest>

// for Sleep
#include <Windows.h>

using namespace P1::GameDownloader;
using namespace P1::GameDownloader::Extractor;

TEST(GameDownloadService, HookPriorityCheck)
{
  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeLibtorrentWrapperAdapter libtorrentWrapper(&downloadService);
  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 10000);
  QObject::connect(
    &downloadService, SIGNAL(postHooksCompleted(const P1::Core::Service*,P1::GameDownloader::HookBase::HookResult)), 
    &loopKiller, SLOT(terminateLoop()));

  QString id("someServiceId");

  QList<int> prelist;
  QList<int> postlist;

  DOWNLOADERHOOK(pre) {
    if (service->id() == id) {
      prelist.append(hookId);
    }

    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(post) {
    if (service->id() == id) {
      postlist.append(hookId);
    }

    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, post);
  LambdaHook lamda5(5, pre, post);
  LambdaHook lamda2(2, pre, post);
  LambdaHook lamda3(3, pre, post);
  LambdaHook lamda4(4, pre, post);
  
  downloadService.registerHook(id, 1, 4, &lamda1);
  downloadService.registerHook(id, 1, 4, &lamda5);
  downloadService.registerHook(id, 2, 3, &lamda2);
  downloadService.registerHook(id, 3, 2, &lamda3);
  downloadService.registerHook(id, 4, 1, &lamda4);

  P1::Core::Service service;
  service.setArea(P1::Core::Service::Live);
  service.setId(id);
  service.setTorrentUrl(QUrl("http://fs0.protocol.one/update/aika/"));
  service.setExtractorType(extractor.extractorId());

  checkUpdateAdapter.setUpdateTime(0);
  bool signalFine = false;
  checkUpdateAdapter.setPreHook([&] (const P1::Core::Service *s, P1::GameDownloader::CheckUpdateHelper::CheckUpdateType type) -> bool {
    signalFine = (&service == s);
    signalFine &= (type == P1::GameDownloader::CheckUpdateHelper::Normal);
    return true;
  });

  downloadService.start(&service, P1::GameDownloader::Normal);

  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(1, libtorrentWrapper.downloadEventCount());
  ASSERT_TRUE(signalFine);

  ASSERT_EQ(4, prelist[0]);
  ASSERT_EQ(3, prelist[1]);
  ASSERT_EQ(2, prelist[2]);
  ASSERT_EQ(5, prelist[3]);
  ASSERT_EQ(1, prelist[4]);

  ASSERT_EQ(5, postlist[0]);
  ASSERT_EQ(1, postlist[1]);
  ASSERT_EQ(2, postlist[2]);
  ASSERT_EQ(3, postlist[3]);
  ASSERT_EQ(4, postlist[4]);
}

TEST(GameDownloadService, PreHookResultCheck)
{
  QString id("someServiceId");
  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 10000);
  QObject::connect(
    &downloadService, SIGNAL(downloadRequest(const P1::Core::Service*, P1::GameDownloader::StartType, bool)), 
    &loopKiller, SLOT(terminateLoop()));
  int step = 0;
  int shouldBeZero = 0;
  DOWNLOADERHOOK(pre) {
    step++;
    switch(step) {
    case 1:
      return P1::GameDownloader::HookBase::PreHookStartPoint;
    case 2:
      return P1::GameDownloader::HookBase::PreHookEndPoint;
    };
    
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(post) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, post);
  downloadService.registerHook(id, 1, 4, &lamda1);

  P1::Core::Service service;
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  downloadService.start(&service, P1::GameDownloader::Normal);
  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(2, step);
  ASSERT_EQ(0, shouldBeZero);
}

TEST(GameDownloadService, PostHookResultCheck)
{
  QString id("someServiceId");

  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);
  FakeLibtorrentWrapperAdapter libtorrentWrapper(&downloadService);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 5000);
  QObject::connect(
    &downloadService, SIGNAL(finished(const P1::Core::Service*)), 
    &loopKiller, SLOT(terminateLoop()));
  int step = 0;
  DOWNLOADERHOOK(pre) {
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(post) {
    step++;
    switch(step) {
    case 1:
      return P1::GameDownloader::HookBase::PostHookStartPoint;
    case 2:
      return P1::GameDownloader::HookBase::PostHookEndPoint;
    };

    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, post);
  downloadService.registerHook(id, 1, 4, &lamda1);

  P1::Core::Service service;
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  downloadService.start(&service, P1::GameDownloader::Normal);
  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(2, step);
}

TEST(GameDownloadService, PreHookResultCheck_JumpToPostStart)
{
  QString id("someServiceId");

  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);
  FakeLibtorrentWrapperAdapter libtorrentWrapper(&downloadService);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 5000);
  QObject::connect(
    &downloadService, SIGNAL(finished(const P1::Core::Service*)), 
    &loopKiller, SLOT(terminateLoop()));
  int step = 0;
  int postCount = 0;
  int shouldBeZero = 0;
  DOWNLOADERHOOK(pre) {
    step++;
    if (step == 1) {
      return P1::GameDownloader::HookBase::PostHookStartPoint;
    }

    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(post) {
    postCount++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(preCantbeCalled) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, post);
  LambdaHook lamda2(2, preCantbeCalled, post);
  downloadService.registerHook(id, 100, 1, &lamda1);
  downloadService.registerHook(id, 1, 1, &lamda2);

  P1::Core::Service service;
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  downloadService.start(&service, P1::GameDownloader::Normal);
  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(1, step);
  ASSERT_EQ(0, shouldBeZero);
  ASSERT_EQ(2, postCount);
}

TEST(GameDownloadService, PreHookResultCheck_JumpToPostEnd)
{
  QString id("someServiceId");

  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);
  FakeLibtorrentWrapperAdapter libtorrentWrapper(&downloadService);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 5000);
  QObject::connect(
    &downloadService, SIGNAL(finished(const P1::Core::Service*)), 
    &loopKiller, SLOT(terminateLoop()));
  int step = 0;
  int postCount = 0;
  int shouldBeZero = 0;
  DOWNLOADERHOOK(pre) {
    step++;
    if (step == 1) {
      return P1::GameDownloader::HookBase::PostHookEndPoint;
    }

    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(post) {
    postCount++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(preCantbeCalled) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, post);
  LambdaHook lamda2(2, preCantbeCalled, post);
  downloadService.registerHook(id, 100, 1, &lamda1);
  downloadService.registerHook(id, 1, 1, &lamda2);

  P1::Core::Service service;
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  downloadService.start(&service, P1::GameDownloader::Normal);
  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(1, step);
  ASSERT_EQ(0, shouldBeZero);
  ASSERT_EQ(0, postCount);
}

TEST(GameDownloadService, SimpleStopOnPreHooks)
{
  QString id("someServiceId");
  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);
  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 5000);
  QObject::connect(
    &downloadService, SIGNAL(stopped(const P1::Core::Service*)), 
    &loopKiller, SLOT(terminateLoop()));

  int step = 0;
  int postCount = 0;
  int shouldBeZero = 0;
  DOWNLOADERHOOK(pre) {
    step++;
    Sleep(1000);
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(post) {
    postCount++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(preCantbeCalled) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, post);
  LambdaHook lamda2(2, preCantbeCalled, post);
  downloadService.registerHook(id, 100, 1, &lamda1);
  downloadService.registerHook(id, 1, 1, &lamda2);

  P1::Core::Service service;
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  downloadService.start(&service, P1::GameDownloader::Normal);

  SINGLESHOTFUNCTOR(stopCallFunction) {
    downloadService.stop(&service);
  };

  SingleShotFunctor stopCaller(100, stopCallFunction);

  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(1, step);
  ASSERT_EQ(0, shouldBeZero);
  ASSERT_EQ(0, postCount);
}

TEST(GameDownloadService, SimpleStopOnPostHooks)
{
  QString id("someServiceId");
  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeLibtorrentWrapperAdapter libtorrentWrapper(&downloadService);
  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 100000);
  QObject::connect(
    &downloadService, SIGNAL(stopped(const P1::Core::Service*)), 
    &loopKiller, SLOT(terminateLoop()));

  int preHookCount = 0;
  int postCount = 0;
  int shouldBeZero = 0;
  DOWNLOADERHOOK(pre) {
    preHookCount++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(post) {
    postCount++;
    Sleep(2000);
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(postCantbeCalled) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, post);
  LambdaHook lamda2(2, pre, postCantbeCalled);
  downloadService.registerHook(id, 100, 100, &lamda1);
  downloadService.registerHook(id, 1, 1, &lamda2);

  P1::Core::Service service;
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  downloadService.start(&service, P1::GameDownloader::Normal);

  SINGLESHOTFUNCTOR(stopCallFunction) {
    downloadService.stop(&service);
  };

  SingleShotFunctor stopCaller(500, stopCallFunction);

  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(2, preHookCount);
  ASSERT_EQ(1, libtorrentWrapper.downloadEventCount());
  ASSERT_EQ(0, shouldBeZero);
  ASSERT_EQ(1, postCount);
}

TEST(GameDownloadService, SimpleStopOnDownload)
{
  QString id("someServiceId");
  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeLibtorrentWrapperAdapter libtorrentWrapper(&downloadService);
  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);
  libtorrentWrapper.setDownloadTime(1000);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 5000);
  QObject::connect(
    &downloadService, SIGNAL(stopped(const P1::Core::Service*)), 
    &loopKiller, SLOT(terminateLoop()));

  SignalCounter downloadServiceStoppedCounter;
  QObject::connect(
    &downloadService, SIGNAL(stopped(const P1::Core::Service*)), 
    &downloadServiceStoppedCounter, SLOT(eventSlot()));

  int preHookCount = 0;
  int shouldBeZero = 0;

  DOWNLOADERHOOK(pre) {
    preHookCount++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(postCantbeCalled) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, postCantbeCalled);
  LambdaHook lamda2(2, pre, postCantbeCalled);
  downloadService.registerHook(id, 100, 100, &lamda1);
  downloadService.registerHook(id, 1, 1, &lamda2);

  P1::Core::Service service;
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  downloadService.start(&service, P1::GameDownloader::Normal);

  SINGLESHOTFUNCTOR(stopCallFunction) {
    downloadService.stop(&service);
  };

  SingleShotFunctor stopCaller(500, stopCallFunction);

  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(2, preHookCount);
  ASSERT_EQ(1, libtorrentWrapper.downloadEventCount());
  ASSERT_EQ(0, shouldBeZero);
  ASSERT_EQ(1, downloadServiceStoppedCounter.eventCount());
}

TEST(GameDownloadService, SimpleStopOnCheckUpdate)
{
  QString id("someServiceId");
  GameDownloadService downloadService;
  downloadService.init();
  DummyExtractor extractor;
  downloadService.registerExtractor(&extractor);

  FakeCheckUpdateAdapter checkUpdateAdapter(&downloadService);
  checkUpdateAdapter.setUpdateTime(2000);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 50000);
  QObject::connect(
    &downloadService, SIGNAL(stopped(const P1::Core::Service*)), 
    &loopKiller, SLOT(terminateLoop()));

  SignalCounter downloadServiceStoppedCounter;
  QObject::connect(
    &downloadService, SIGNAL(stopped(const P1::Core::Service*)), 
    &downloadServiceStoppedCounter, SLOT(eventSlot()));

  int shouldBeZero = 0;

  DOWNLOADERHOOK(pre) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  DOWNLOADERHOOK(postCantbeCalled) {
    shouldBeZero++;
    return P1::GameDownloader::HookBase::Continue;
  };

  LambdaHook lamda1(1, pre, postCantbeCalled);
  LambdaHook lamda2(2, pre, postCantbeCalled);
  downloadService.registerHook(id, 100, 100, &lamda1);
  downloadService.registerHook(id, 1, 1, &lamda2);

  P1::Core::Service service;
  service.setArea(P1::Core::Service::Live);
  service.setId(id);
  service.setExtractorType(extractor.extractorId());
  service.setTorrentUrl(QUrl("http://fs0.protocol.one/update/aika/"));

  downloadService.start(&service, P1::GameDownloader::Normal);

  SINGLESHOTFUNCTOR(stopCallFunction) {
    downloadService.stop(&service);
  };

  SingleShotFunctor stopCaller(500, stopCallFunction);

  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());
  ASSERT_EQ(0, shouldBeZero);
  ASSERT_EQ(1, downloadServiceStoppedCounter.eventCount());
}
