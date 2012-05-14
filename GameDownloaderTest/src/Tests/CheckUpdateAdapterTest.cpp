#include "TestEventLoopFinisher.h"
#include "FileUtils.h"

#include <GameDownloader/CheckUpdateAdapter.h>
#include <Settings/Settings>

#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QCoreApplication>

using namespace GGS::Settings;

// defined in CheckUpdateHookTest.cpp
void saveLastModifiedDate(const QString& date, const QString& id);
QString loadLastModifiedDate(const QString& id);

using namespace GGS::GameDownloader;
TEST(CheckUpdateAdapterTest, NormalTest)
{
  CheckUpdateAdapter adapter;
  QSignalSpy spy1(&adapter, SIGNAL(checkUpdateCompleted(const GGS::Core::Service *, bool)));
  QSignalSpy spy2(&adapter, SIGNAL(checkUpdateFailed(const GGS::Core::Service *)));

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 5000);
  QObject::connect(&adapter, SIGNAL(checkUpdateCompleted(const GGS::Core::Service *, bool)), &killer, SLOT(terminateLoop()));

  saveLastModifiedDate("", "300002010000000000");

  GGS::Core::Service service;
  service.setTorrentUrl(QUrl("http://fs0.gamenet.ru/update/aika/"));
  QString root = QCoreApplication::applicationDirPath();
  QString filePath = root.append("/game");

  FileUtils::removeDir(filePath);

  service.setTorrentFilePath(filePath);
  service.setId("300002010000000000");
  service.setArea(GGS::Core::Service::Live);

  adapter.checkUpdateRequest(&service, CheckUpdateHelper::Normal);

  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(1, spy1.count());
  ASSERT_EQ(0, spy2.count());

  QList<QVariant> arguments = spy1.takeFirst();
  ASSERT_EQ(&service, arguments.at(0).value<const GGS::Core::Service *>());
  ASSERT_TRUE(arguments.at(1).toBool());
}

TEST(CheckUpdateAdapterTest, FailTest)
{
  CheckUpdateAdapter adapter;
  QSignalSpy spy1(&adapter, SIGNAL(checkUpdateCompleted(const GGS::Core::Service *, bool)));
  QSignalSpy spy2(&adapter, SIGNAL(checkUpdateFailed(const GGS::Core::Service *)));

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 5000);
  QObject::connect(&adapter, SIGNAL(checkUpdateFailed(const GGS::Core::Service *)), &killer, SLOT(terminateLoop()));

  saveLastModifiedDate("", "300002010000000000");

  GGS::Core::Service service;
  service.setTorrentUrl(QUrl("http://fs0wfake.gamenetfake.ru/update/aika/"));
  QString root = QCoreApplication::applicationDirPath();
  QString filePath = root.append("/game");

  FileUtils::removeDir(filePath);

  service.setTorrentFilePath(filePath);
  service.setId("300002010000000000");
  service.setArea(GGS::Core::Service::Live);

  adapter.checkUpdateRequest(&service, CheckUpdateHelper::Normal);

  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(0, spy1.count());
  ASSERT_EQ(1, spy2.count());

  QList<QVariant> arguments = spy2.takeFirst();
  ASSERT_EQ(&service, arguments.at(0).value<const GGS::Core::Service *>());
}