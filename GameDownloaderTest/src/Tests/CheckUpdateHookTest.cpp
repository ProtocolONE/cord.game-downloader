
#include "TestEventLoopFinisher.h"
#include "FileUtils.h"

#include <Core/Service>
#include <GameDownloader/CheckUpdateHelper.h>
#include <Settings/Settings>

#include <gtest/gtest.h>
#include <QtCore/QEventLoop>
#include <QtCore/QCoreApplication>
#include <QtTest/QSignalSpy>


void saveLastModifiedDate(const QString& date, const QString& id)
{
  GGS::Settings::Settings settings; 
  QString groupName = QString("GameDownloader/CheckUpdate/%1").arg(id);
  settings.beginGroup(groupName);
  settings.setValue("LastModified", date, true);
}

QString loadLastModifiedDate(const QString& id)
{
  GGS::Settings::Settings settings; 
  QString groupName = QString("GameDownloader/CheckUpdate/%1").arg(id);
  settings.beginGroup(groupName);
  return settings.value("LastModified", "").toString();
}

TEST(CheckUpdateHookTest, simpleTest)
{
  QEventLoop loop;
  GGS::GameDownloader::CheckUpdateHelper hook;

  GGS::Core::Service service;
  service.setTorrentUrl(QUrl("http://fs0.gamenet.ru/update/aika/"));
  QString root = QCoreApplication::applicationDirPath();
  QString filePath = root.append("/game");

  FileUtils::removeDir(filePath);

  service.setTorrentFilePath(filePath);
  service.setId("300002010000000000");
  service.setArea(GGS::Core::Service::Live);

  saveLastModifiedDate("", "300002010000000000");

  QSignalSpy spy1(&hook, SIGNAL(result(const GGS::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const GGS::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 10000);
  QObject::connect(&hook, SIGNAL(result(const GGS::Core::Service *, bool)), &killer, SLOT(terminateLoop()));

  hook.startCheck(&service, GGS::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(1, spy1.count());
  ASSERT_EQ(0, spy2.count());

  QList<QVariant> arguments = spy1.takeFirst();
  ASSERT_EQ(&service, arguments.at(0).value<const GGS::Core::Service *>());

  QString expectedTorrentFilePath = filePath;
  expectedTorrentFilePath.append("/live/300002010000000000.torrent");
  ASSERT_TRUE(arguments.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));
}

TEST(CheckUpdateHookTest, failHeadTest)
{
  QEventLoop loop;
  GGS::GameDownloader::CheckUpdateHelper hook;

  GGS::Core::Service service;
  service.setTorrentUrl(QUrl("http://fs0fake.gamenet.ru/update/aika/"));
  QString root = QCoreApplication::applicationDirPath();
  QString filePath = root.append("/game");

  FileUtils::removeDir(filePath);

  service.setTorrentFilePath(filePath);
  service.setId("300002010000000000");
  service.setArea(GGS::Core::Service::Live);

  saveLastModifiedDate("", "300002010000000000");

  QSignalSpy spy1(&hook, SIGNAL(result(const GGS::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const GGS::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 100000);
  QObject::connect(&hook, SIGNAL(error(const GGS::Core::Service *)), &killer, SLOT(terminateLoop()));

  hook.startCheck(&service, GGS::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(0, spy1.count());
  ASSERT_EQ(1, spy2.count());

  QList<QVariant> arguments = spy2.takeFirst();
  ASSERT_EQ(&service, arguments.at(0).value<const GGS::Core::Service *>());
}

TEST(CheckUpdateHookTest, checkUpdateWithSameDate)
{
  QEventLoop loop;
  GGS::GameDownloader::CheckUpdateHelper hook;

  GGS::Core::Service service;
  service.setTorrentUrl(QUrl("http://fs0.gamenet.ru/update/aika/"));
  QString root = QCoreApplication::applicationDirPath();
  QString filePath = root.append("/game");

  FileUtils::removeDir(filePath);

  service.setTorrentFilePath(filePath);
  service.setId("300002010000000000");
  service.setArea(GGS::Core::Service::Live);

  saveLastModifiedDate("", "300002010000000000");

  QSignalSpy spy1(&hook, SIGNAL(result(const GGS::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const GGS::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 10000);
  QObject::connect(&hook, SIGNAL(result(const GGS::Core::Service *, bool)), &killer, SLOT(terminateLoop()));

  // Just download and save last modified date.
  hook.startCheck(&service, GGS::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(1, spy1.count());
  ASSERT_EQ(0, spy2.count());

  QList<QVariant> arguments = spy1.takeFirst();
  ASSERT_EQ(&service, arguments.at(0).value<const GGS::Core::Service *>());

  QString expectedTorrentFilePath = filePath;
  expectedTorrentFilePath.append("/live/300002010000000000.torrent");
  ASSERT_TRUE(arguments.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  QString lastmodified1 = loadLastModifiedDate("300002010000000000");
  // Now lets check again and we should get same date and false in event.
  GGS::GameDownloader::CheckUpdateHelper hook2;
  QSignalSpy spy3(&hook2, SIGNAL(result(const GGS::Core::Service *, bool)));
  QSignalSpy spy4(&hook2, SIGNAL(error(const GGS::Core::Service *)));
  QEventLoop loop2;
  TestEventLoopFinisher killer2(&loop2, 10000);
  QObject::connect(&hook2, SIGNAL(result(const GGS::Core::Service *, bool)), &killer2, SLOT(terminateLoop()));
  hook2.startCheck(&service, GGS::GameDownloader::CheckUpdateHelper::Normal);
  loop2.exec();
  ASSERT_FALSE(killer2.isKilledByTimeout());

  ASSERT_EQ(1, spy3.count());
  ASSERT_EQ(0, spy4.count());

  QList<QVariant> arguments2 = spy3.takeFirst();
  ASSERT_EQ(&service, arguments2.at(0).value<const GGS::Core::Service *>());
  ASSERT_FALSE(arguments2.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  QString lastmodified2 = loadLastModifiedDate("300002010000000000");
  ASSERT_TRUE(lastmodified1 == lastmodified2);

  // let's now delete torrent and we should get true in event.
  QFile::remove(expectedTorrentFilePath);
  ASSERT_FALSE(QFile::exists(expectedTorrentFilePath));

  GGS::GameDownloader::CheckUpdateHelper hook3;
  QSignalSpy spy5(&hook3, SIGNAL(result(const GGS::Core::Service *, bool)));
  QSignalSpy spy6(&hook3, SIGNAL(error(const GGS::Core::Service *)));
  QEventLoop loop3;
  TestEventLoopFinisher killer3(&loop3, 10000);
  QObject::connect(&hook3, SIGNAL(result(const GGS::Core::Service *, bool)), &killer3, SLOT(terminateLoop()));
  hook3.startCheck(&service, GGS::GameDownloader::CheckUpdateHelper::Normal);
  loop3.exec();
  ASSERT_FALSE(killer3.isKilledByTimeout());

  ASSERT_EQ(1, spy5.count());
  ASSERT_EQ(0, spy6.count());

  QList<QVariant> arguments3 = spy5.takeFirst();
  ASSERT_EQ(&service, arguments3.at(0).value<const GGS::Core::Service *>());
  ASSERT_TRUE(arguments3.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  QString lastmodified3 = loadLastModifiedDate("300002010000000000");
  ASSERT_TRUE(lastmodified2 == lastmodified3);
}

TEST(CheckUpdateHookTest, forceStartTest)
{
  QEventLoop loop;
  GGS::GameDownloader::CheckUpdateHelper hook;

  GGS::Core::Service service;
  service.setTorrentUrl(QUrl("http://fs0.gamenet.ru/update/aika/"));
  QString root = QCoreApplication::applicationDirPath();
  QString filePath = root.append("/game");

  FileUtils::removeDir(filePath);

  service.setTorrentFilePath(filePath);
  service.setId("300002010000000000");
  service.setArea(GGS::Core::Service::Live);

  saveLastModifiedDate("", "300002010000000000");

  QSignalSpy spy1(&hook, SIGNAL(result(const GGS::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const GGS::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 10000);
  QObject::connect(&hook, SIGNAL(result(const GGS::Core::Service *, bool)), &killer, SLOT(terminateLoop()));

  // Just download and save last modified date.
  hook.startCheck(&service, GGS::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(1, spy1.count());
  ASSERT_EQ(0, spy2.count());

  QList<QVariant> arguments = spy1.takeFirst();
  ASSERT_EQ(&service, arguments.at(0).value<const GGS::Core::Service *>());

  QString expectedTorrentFilePath = filePath;
  expectedTorrentFilePath.append("/live/300002010000000000.torrent");
  ASSERT_TRUE(arguments.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));


  // Now lets check again with force download torrent and we should get same date and true in event.
  GGS::GameDownloader::CheckUpdateHelper hook2;
  QSignalSpy spy3(&hook2, SIGNAL(result(const GGS::Core::Service *, bool)));
  QSignalSpy spy4(&hook2, SIGNAL(error(const GGS::Core::Service *)));
  QEventLoop loop2;
  TestEventLoopFinisher killer2(&loop2, 10000);
  QObject::connect(&hook2, SIGNAL(result(const GGS::Core::Service *, bool)), &killer2, SLOT(terminateLoop()));
  hook2.startCheck(&service, GGS::GameDownloader::CheckUpdateHelper::ForceDownloadTorrent);
  loop2.exec();
  ASSERT_FALSE(killer2.isKilledByTimeout());

  ASSERT_EQ(1, spy3.count());
  ASSERT_EQ(0, spy4.count());

  QList<QVariant> arguments2 = spy3.takeFirst();
  ASSERT_EQ(&service, arguments2.at(0).value<const GGS::Core::Service *>());
  ASSERT_TRUE(arguments2.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));
}