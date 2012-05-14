#include <GameDownloader/StageProgressCalculator.h>
#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>

#include "SingleShotFunctor.h"
#include "TestEventLoopFinisher.h"
#include "SimpleContinueHook.h"

#include <QtCore/QObject>
#include <QtTest/QSignalSpy>

#include <gtest/gtest.h>
using namespace GGS::GameDownloader;

class StageProgressCalculatorTest : public ::testing::Test
{
public:
  GGS::GameDownloader::StageProgressCalculator calc;

  virtual void SetUp() {
    this->spy = new QSignalSpy(&calc, SIGNAL(progressChanged(QString, qint8)));
    this->spy1 = new QSignalSpy(&calc, SIGNAL(progressDownloadChanged(QString, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)));
    this->spy2 = new QSignalSpy(&calc, SIGNAL(progressExtractionChanged(QString, qint8, qint64, qint64)));

    this->calc.setCheckUpdateStageSize(5);
    this->calc.setPreHookStageSize(10);
    this->calc.setTorrentRehashingStageSize(10);
    this->calc.setTorrentDownloadinngStageSize(50);
    this->calc.setPostHookStageSize(10);
    this->calc.setExtractionStageSize(15);
  }

  virtual void TearDown() {
    delete this->spy;
    delete this->spy1;
    delete this->spy2;
  }

  QSignalSpy *spy;
  QSignalSpy *spy1;
  QSignalSpy *spy2;
  
};

TEST_F(StageProgressCalculatorTest, progressCheckUpdate)
{
  int progress[] = {0, 20, 40, 60, 80, 100};
  int i = 0;
  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 5000);
  GGS::GameDownloader::StageProgressCalculator *pcalc = &this->calc;
  QString id = "someServiceId";

  std::tr1::function<void ()> progressCaller = [id, &progress, &i, &pcalc, &loop]() mutable -> void {
    pcalc->checkUpdateProgress(id, progress[i]);
    i++;
    if (i == 6)
      loop.exit();
  };

  SingleShotFunctor functor1(1, progressCaller);
  SingleShotFunctor functor2(2, progressCaller);
  SingleShotFunctor functor3(3, progressCaller);
  SingleShotFunctor functor4(4, progressCaller);
  SingleShotFunctor functor5(5, progressCaller);
  SingleShotFunctor functor6(6, progressCaller);

  loop.exec();
  ASSERT_EQ(6, spy->count());
  ASSERT_EQ(0, spy1->count());
  ASSERT_EQ(0, spy2->count());
  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(id, spy->at(i).at(0).toString());

  int expectedResults[] = { 0, 1, 2, 3, 4, 5};

  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(expectedResults[i], spy->at(i).at(1).toInt());

}

TEST_F(StageProgressCalculatorTest, progressPreHook)
{
  QString id("someServiceId");
  SimpleContinueHook hook1(1, 0, 0);
  hook1.setBeforeProgressWeight(20);

  SimpleContinueHook hook2(2, 0, 0);
  hook2.setBeforeProgressWeight(60);

  SimpleContinueHook hook3(3, 0, 0);
  hook3.setBeforeProgressWeight(20);

  this->calc.registerHook(id, 1, 10, &hook3);
  this->calc.registerHook(id, 5, 5, &hook2);
  this->calc.registerHook(id, 10, 1, &hook1);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  
  int progress[] = {0, 20, 40, 60, 80, 100};
  GGS::GameDownloader::StageProgressCalculator *pcalc = &this->calc;
  int i = 0;
  std::tr1::function<void ()> progressCaller1 = [id, &progress, &i, &pcalc, &loop, &hook1]() mutable -> void {
    pcalc->preHookProgress(id, hook1.hookId(), progress[i]);
    i++;
    if (i == 6)
      i = 0;
  };

  std::tr1::function<void ()> progressCaller2 = [id, &progress, &i, &pcalc, &loop, &hook2]() mutable -> void {
    pcalc->preHookProgress(id, hook2.hookId(), progress[i]);
    i++;
    if (i == 6)
      i = 0;
  };

  std::tr1::function<void ()> progressCaller3 = [id, &progress, &i, &pcalc, &loop, &hook3]() mutable -> void {
    pcalc->preHookProgress(id, hook3.hookId(), progress[i]);
    i++;
    if (i == 6)
      loop.exit();
  };

  SingleShotFunctor functor11(1, progressCaller1);
  SingleShotFunctor functor12(2, progressCaller1);
  SingleShotFunctor functor13(3, progressCaller1);
  SingleShotFunctor functor14(4, progressCaller1);
  SingleShotFunctor functor15(5, progressCaller1);
  SingleShotFunctor functor16(6, progressCaller1);

  SingleShotFunctor functor21(21, progressCaller2);
  SingleShotFunctor functor22(22, progressCaller2);
  SingleShotFunctor functor23(23, progressCaller2);
  SingleShotFunctor functor24(24, progressCaller2);
  SingleShotFunctor functor25(25, progressCaller2);
  SingleShotFunctor functor26(26, progressCaller2);

  SingleShotFunctor functor31(41, progressCaller3);
  SingleShotFunctor functor32(42, progressCaller3);
  SingleShotFunctor functor33(43, progressCaller3);
  SingleShotFunctor functor34(44, progressCaller3);
  SingleShotFunctor functor35(45, progressCaller3);
  SingleShotFunctor functor36(46, progressCaller3);

  loop.exec();
  ASSERT_EQ(18, spy->count());
  ASSERT_EQ(0, spy1->count());
  ASSERT_EQ(0, spy2->count());

  for (i = 0; i < 18; ++i)
    ASSERT_EQ(id, spy->at(i).at(0).toString());

  int expectedResults[] = { 5, 5, 6, 6, 7, 7,
                            7, 8, 9, 11, 12, 13,
                            13, 13, 14, 14, 15, 15};

  for (i = 0; i < 18; ++i)
    ASSERT_EQ(expectedResults[i], spy->at(i).at(1).toInt());
}

TEST_F(StageProgressCalculatorTest, progressPostHook)
{
  QString id("someServiceId");
  SimpleContinueHook hook1(1, 0, 0);
  hook1.setAfterProgressWeight(20);

  SimpleContinueHook hook2(2, 0, 0);
  hook2.setAfterProgressWeight(60);

  SimpleContinueHook hook3(3, 0, 0);
  hook3.setAfterProgressWeight(20);

  this->calc.registerHook(id, 10, 1, &hook3);
  this->calc.registerHook(id, 5, 5, &hook2);
  this->calc.registerHook(id, 1, 10, &hook1);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);

  int progress[] = {0, 20, 40, 60, 80, 100};
  GGS::GameDownloader::StageProgressCalculator *pcalc = &this->calc;
  int i = 0;
  std::tr1::function<void ()> progressCaller1 = [id, &progress, &i, &pcalc, &loop, &hook1]() mutable -> void {
    pcalc->postHookProgress(id, hook1.hookId(), progress[i]);
    i++;
    if (i == 6)
      i = 0;
  };

  std::tr1::function<void ()> progressCaller2 = [id, &progress, &i, &pcalc, &loop, &hook2]() mutable -> void {
    pcalc->postHookProgress(id, hook2.hookId(), progress[i]);
    i++;
    if (i == 6)
      i = 0;
  };

  std::tr1::function<void ()> progressCaller3 = [id, &progress, &i, &pcalc, &loop, &hook3]() mutable -> void {
    pcalc->postHookProgress(id, hook3.hookId(), progress[i]);
    i++;
    if (i == 6)
      loop.exit();
  };

  SingleShotFunctor functor11(1, progressCaller1);
  SingleShotFunctor functor12(2, progressCaller1);
  SingleShotFunctor functor13(3, progressCaller1);
  SingleShotFunctor functor14(4, progressCaller1);
  SingleShotFunctor functor15(5, progressCaller1);
  SingleShotFunctor functor16(6, progressCaller1);

  SingleShotFunctor functor21(21, progressCaller2);
  SingleShotFunctor functor22(22, progressCaller2);
  SingleShotFunctor functor23(23, progressCaller2);
  SingleShotFunctor functor24(24, progressCaller2);
  SingleShotFunctor functor25(25, progressCaller2);
  SingleShotFunctor functor26(26, progressCaller2);

  SingleShotFunctor functor31(41, progressCaller3);
  SingleShotFunctor functor32(42, progressCaller3);
  SingleShotFunctor functor33(43, progressCaller3);
  SingleShotFunctor functor34(44, progressCaller3);
  SingleShotFunctor functor35(45, progressCaller3);
  SingleShotFunctor functor36(46, progressCaller3);

  loop.exec();
  ASSERT_EQ(18, spy->count());
  ASSERT_EQ(0, spy1->count());
  ASSERT_EQ(0, spy2->count());

  for (i = 0; i < 18; ++i)
    ASSERT_EQ(id, spy->at(i).at(0).toString());

  int expectedResults[] = { 5, 5, 6, 6, 7, 7,
    7, 8, 9, 11, 12, 13,
    13, 13, 14, 14, 15, 15};

  for (i = 0; i < 18; ++i)
    ASSERT_EQ(expectedResults[i] + 85, spy->at(i).at(1).toInt());
}

TEST_F(StageProgressCalculatorTest, progressZeroHook)
{
  QString id("someServiceId");
  SimpleContinueHook hook1(1, 0, 0);
  hook1.setAfterProgressWeight(0);
  hook1.setBeforeProgressWeight(0);

  SimpleContinueHook hook2(2, 0, 0);
  hook2.setAfterProgressWeight(0);
  hook2.setBeforeProgressWeight(0);

  SimpleContinueHook hook3(3, 0, 0);
  hook3.setAfterProgressWeight(0);
  hook3.setBeforeProgressWeight(0);

  this->calc.registerHook(id, 10, 1, &hook3);
  this->calc.registerHook(id, 5, 5, &hook2);
  this->calc.registerHook(id, 1, 10, &hook1);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);

  int progress[] = {0, 20, 40, 60, 80, 100};
  GGS::GameDownloader::StageProgressCalculator *pcalc = &this->calc;
  int i = 0;
  std::tr1::function<void ()> progressCaller1 = [id, &progress, &i, &pcalc, &loop, &hook1]() mutable -> void {
    pcalc->preHookProgress(id, hook1.hookId(), progress[i]);
    pcalc->postHookProgress(id, hook1.hookId(), progress[i]);
    i++;
    if (i == 6)
      i = 0;
  };

  std::tr1::function<void ()> progressCaller2 = [id, &progress, &i, &pcalc, &loop, &hook2]() mutable -> void {
    pcalc->preHookProgress(id, hook2.hookId(), progress[i]);
    pcalc->postHookProgress(id, hook2.hookId(), progress[i]);
    i++;
    if (i == 6)
      i = 0;
  };

  std::tr1::function<void ()> progressCaller3 = [id, &progress, &i, &pcalc, &loop, &hook3]() mutable -> void {
    pcalc->preHookProgress(id, hook3.hookId(), progress[i]);
    pcalc->postHookProgress(id, hook3.hookId(), progress[i]);
    i++;
    if (i == 6)
      loop.exit();
  };

  SingleShotFunctor functor11(1, progressCaller1);
  SingleShotFunctor functor12(2, progressCaller1);
  SingleShotFunctor functor13(3, progressCaller1);
  SingleShotFunctor functor14(4, progressCaller1);
  SingleShotFunctor functor15(5, progressCaller1);
  SingleShotFunctor functor16(6, progressCaller1);

  SingleShotFunctor functor21(21, progressCaller2);
  SingleShotFunctor functor22(22, progressCaller2);
  SingleShotFunctor functor23(23, progressCaller2);
  SingleShotFunctor functor24(24, progressCaller2);
  SingleShotFunctor functor25(25, progressCaller2);
  SingleShotFunctor functor26(26, progressCaller2);

  SingleShotFunctor functor31(41, progressCaller3);
  SingleShotFunctor functor32(42, progressCaller3);
  SingleShotFunctor functor33(43, progressCaller3);
  SingleShotFunctor functor34(44, progressCaller3);
  SingleShotFunctor functor35(45, progressCaller3);
  SingleShotFunctor functor36(46, progressCaller3);

  loop.exec();
  ASSERT_EQ(0, spy->count());
  ASSERT_EQ(0, spy1->count());
  ASSERT_EQ(0, spy2->count());

  //for (i = 0; i < 18; ++i)
  //  ASSERT_EQ(id, spy->at(i).at(0).toString());

  //int expectedResults[] = { 5, 5, 6, 6, 7, 7,
  //  7, 8, 9, 11, 12, 13,
  //  13, 13, 14, 14, 15, 15};

  //for (i = 0; i < 18; ++i)
  //  ASSERT_EQ(expectedResults[i] + 70, spy->at(i).at(1).toInt());
}

TEST_F(StageProgressCalculatorTest, progressTorrentRehashing)
{
  int progress[] = {0, 20, 40, 60, 80, 100};
  int i = 0;
  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 5000);
  GGS::GameDownloader::StageProgressCalculator *pcalc = &this->calc;
  QString id = "someServiceId";

  using GGS::Libtorrent::EventArgs::ProgressEventArgs;

  ProgressEventArgs args;
  args.setDownloadRate(100000);
  args.setUploadRate(200000);
  args.setId(id);
  args.setStatus(ProgressEventArgs::CheckingFiles);
  args.setTotalWanted(123);
  args.setTotalWantedDone(120);

  std::tr1::function<void ()> progressCaller = [id, &progress, &i, &pcalc, &loop, &args]() mutable -> void {
    qreal p = progress[i] / 100.0f;
    args.setProgress(p);
    pcalc->torrentProgress(args);
    i++;
    if (i == 6)
      loop.exit();
  };

  SingleShotFunctor functor1(1, progressCaller);
  SingleShotFunctor functor2(2, progressCaller);
  SingleShotFunctor functor3(3, progressCaller);
  SingleShotFunctor functor4(4, progressCaller);
  SingleShotFunctor functor5(5, progressCaller);
  SingleShotFunctor functor6(6, progressCaller);

  loop.exec();
  ASSERT_EQ(6, spy->count());
  ASSERT_EQ(0, spy1->count());
  ASSERT_EQ(0, spy2->count());
  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(id, spy->at(i).at(0).toString());

  int expectedResults[] = { 0, 2, 4, 6, 8, 10 };

  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(expectedResults[i] + 15, spy->at(i).at(1).toInt());

}

TEST_F(StageProgressCalculatorTest, progressTorrentDownloading)
{
  int progress[] = {0, 20, 40, 60, 80, 100};
  int i = 0;
  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 5000);
  GGS::GameDownloader::StageProgressCalculator *pcalc = &this->calc;
  QString id = "someServiceId";

  using GGS::Libtorrent::EventArgs::ProgressEventArgs;

  ProgressEventArgs args;
  args.setDownloadRate(100000);
  args.setUploadRate(200000);
  args.setId(id);
  args.setStatus(ProgressEventArgs::Downloading);
  args.setTotalWanted(123);
  args.setTotalWantedDone(120);

  std::tr1::function<void ()> progressCaller = [id, &progress, &i, &pcalc, &loop, &args]() mutable -> void {
    qreal p = progress[i] / 100.0f;
    args.setProgress(p);
    pcalc->torrentProgress(args);
    i++;
    if (i == 6)
      loop.exit();
  };

  SingleShotFunctor functor1(1, progressCaller);
  SingleShotFunctor functor2(2, progressCaller);
  SingleShotFunctor functor3(3, progressCaller);
  SingleShotFunctor functor4(4, progressCaller);
  SingleShotFunctor functor5(5, progressCaller);
  SingleShotFunctor functor6(6, progressCaller);

  loop.exec();
  ASSERT_EQ(0, spy->count());
  ASSERT_EQ(6, spy1->count());
  ASSERT_EQ(0, spy2->count());
  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(id, spy1->at(i).at(0).toString());

  int expectedResults[] = { 0, 10, 20, 30, 40, 50 };

  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(expectedResults[i] + 25, spy1->at(i).at(1).toInt());

}

TEST_F(StageProgressCalculatorTest, progressExtraction)
{
  int progress[] = {0, 20, 40, 60, 80, 100};
  int i = 0;
  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 50000);
  GGS::GameDownloader::StageProgressCalculator *pcalc = &this->calc;
  QString id = "someServiceId";
  int files[] = {0, 1, 2, 3, 4, 5};
  int total = 5;

  std::tr1::function<void ()> progressCaller = [id, &progress, &i, &pcalc, &loop, &files, total]() mutable -> void {
    pcalc->extractionProgress(id, progress[i], files[i], total);
    i++;
    if (i == 6)
      loop.exit();
  };

  SingleShotFunctor functor1(1, progressCaller);
  SingleShotFunctor functor2(2, progressCaller);
  SingleShotFunctor functor3(3, progressCaller);
  SingleShotFunctor functor4(4, progressCaller);
  SingleShotFunctor functor5(5, progressCaller);
  SingleShotFunctor functor6(6, progressCaller);

  loop.exec();
  ASSERT_EQ(0, spy->count());
  ASSERT_EQ(0, spy1->count());
  ASSERT_EQ(6, spy2->count());
  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(id, spy2->at(i).at(0).toString());

  int expectedResults[] = { 0, 3, 6, 9, 12, 15 };
  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(expectedResults[i] + 75, spy2->at(i).at(1).toInt());

  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(files[i], spy2->at(i).at(2).toInt());

  for (int i = 0; i < 6; ++i)
    ASSERT_EQ(total, spy2->at(i).at(3).toInt());


  
}
