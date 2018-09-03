
#include "TestEventLoopFinisher.h"
#include "FileUtils.h"

#include <Core/Service>
#include <GameDownloader/CheckUpdateHelper.h>
#include <Settings/Settings>

#include <gtest/gtest.h>
#include <QtCore/QEventLoop>
#include <QtCore/QCoreApplication>
#include <QtCore/QTextStream>
#include <QtTest/QSignalSpy>

class CheckUpdateHookTest : public ::testing::Test {
  protected:
    void saveLastModifiedDate(const QString& date, const QString& id)
    {
      P1::Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(id);
      settings.setValue("LastModified", date, true);
    }

    QString loadLastModifiedDate(const QString& id)
    {
      P1::Settings::Settings settings; 
      settings.beginGroup("GameDownloader");
      settings.beginGroup("CheckUpdate");
      settings.beginGroup(id);
      return settings.value("LastModified", "").toString();
    }

    P1::Core::Service *buildService(QUrl &url) {
      P1::Core::Service *service = new P1::Core::Service();
      service->setTorrentUrl(url);
      QString root = QCoreApplication::applicationDirPath();
      filePath = root.append("/game");

      FileUtils::removeDir(filePath);

      service->setTorrentFilePath(filePath);
      service->setId("300002010000000000");
      service->setArea(P1::Core::Service::Live);

      this->saveLastModifiedDate("", "300002010000000000");
      return service;
    }

    QString filePath;
};

TEST_F(CheckUpdateHookTest, successTest)
{
  QEventLoop loop;
  P1::GameDownloader::CheckUpdateHelper hook;

  QScopedPointer<P1::Core::Service> service(buildService(QUrl("http://fs0.protocol.one/update/aika/")));

  QSignalSpy spy1(&hook, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const P1::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 10000);
  QObject::connect(&hook, SIGNAL(result(const P1::Core::Service *, bool)), &killer, SLOT(terminateLoop()));

  hook.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(1, spy1.count());
  ASSERT_EQ(0, spy2.count());

  QList<QVariant> arguments = spy1.takeFirst();
  ASSERT_EQ(service.data(), arguments.at(0).value<const P1::Core::Service *>());

  QString expectedTorrentFilePath = filePath;
  expectedTorrentFilePath.append("/live/300002010000000000.torrent");
  ASSERT_TRUE(arguments.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));
}

TEST_F(CheckUpdateHookTest, failHeadTest)
{
  QEventLoop loop;
  P1::GameDownloader::CheckUpdateHelper hook;

  QScopedPointer<P1::Core::Service> service(buildService(QUrl("http://fs0fake.protocol.one/update/aika/")));

  QSignalSpy spy1(&hook, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const P1::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 100000);
  QObject::connect(&hook, SIGNAL(error(const P1::Core::Service *)), &killer, SLOT(terminateLoop()));

  hook.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(0, spy1.count());
  ASSERT_EQ(1, spy2.count());

  QList<QVariant> arguments = spy2.takeFirst();
  ASSERT_EQ(service.data(), arguments.at(0).value<const P1::Core::Service *>());
}

TEST_F(CheckUpdateHookTest, checkUpdateWithSameDate)
{
  QEventLoop loop;
  P1::GameDownloader::CheckUpdateHelper hook;

  QScopedPointer<P1::Core::Service> service(buildService(QUrl("http://fs0.protocol.one/update/aika/")));

  QSignalSpy spy1(&hook, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const P1::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 10000);
  QObject::connect(&hook, SIGNAL(result(const P1::Core::Service *, bool)), &killer, SLOT(terminateLoop()));

  // Just download and save last modified date.
  hook.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(1, spy1.count());
  ASSERT_EQ(0, spy2.count());

  QList<QVariant> arguments = spy1.takeFirst();
  ASSERT_EQ(service.data(), arguments.at(0).value<const P1::Core::Service *>());

  QString expectedTorrentFilePath = filePath;
  expectedTorrentFilePath.append("/live/300002010000000000.torrent");
  ASSERT_TRUE(arguments.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  QString lastmodified1 = loadLastModifiedDate("300002010000000000");
  // Now lets check again and we should get same date and false in event.
  P1::GameDownloader::CheckUpdateHelper hook2;
  QSignalSpy spy3(&hook2, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy4(&hook2, SIGNAL(error(const P1::Core::Service *)));
  QEventLoop loop2;
  TestEventLoopFinisher killer2(&loop2, 10000);
  QObject::connect(&hook2, SIGNAL(result(const P1::Core::Service *, bool)), &killer2, SLOT(terminateLoop()));
  hook2.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::Normal);
  loop2.exec();
  ASSERT_FALSE(killer2.isKilledByTimeout());

  ASSERT_EQ(1, spy3.count());
  ASSERT_EQ(0, spy4.count());

  QList<QVariant> arguments2 = spy3.takeFirst();
  ASSERT_EQ(service.data(), arguments2.at(0).value<const P1::Core::Service *>());
  ASSERT_FALSE(arguments2.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  QString lastmodified2 = loadLastModifiedDate("300002010000000000");
  ASSERT_TRUE(lastmodified1 == lastmodified2);

  // let's now delete torrent and we should get true in event.
  QFile::remove(expectedTorrentFilePath);
  ASSERT_FALSE(QFile::exists(expectedTorrentFilePath));

  P1::GameDownloader::CheckUpdateHelper hook3;
  QSignalSpy spy5(&hook3, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy6(&hook3, SIGNAL(error(const P1::Core::Service *)));
  QEventLoop loop3;
  TestEventLoopFinisher killer3(&loop3, 10000);
  QObject::connect(&hook3, SIGNAL(result(const P1::Core::Service *, bool)), &killer3, SLOT(terminateLoop()));
  hook3.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::Normal);
  loop3.exec();
  ASSERT_FALSE(killer3.isKilledByTimeout());

  ASSERT_EQ(1, spy5.count());
  ASSERT_EQ(0, spy6.count());

  QList<QVariant> arguments3 = spy5.takeFirst();
  ASSERT_EQ(service.data(), arguments3.at(0).value<const P1::Core::Service *>());
  ASSERT_TRUE(arguments3.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  QString lastmodified3 = loadLastModifiedDate("300002010000000000");
  ASSERT_TRUE(lastmodified2 == lastmodified3);

  // and finally let's currupt torrent file.
  QFile curruptFile(expectedTorrentFilePath);
  ASSERT_TRUE(curruptFile.open(QIODevice::ReadWrite));
  QTextStream curruptFileStream(&curruptFile);
  curruptFileStream << "fake torrent file with some intresting info in it";
  curruptFile.close();

  P1::GameDownloader::CheckUpdateHelper hook4;
  QSignalSpy spy7(&hook4, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy8(&hook4, SIGNAL(error(const P1::Core::Service *)));
  QEventLoop loop4;
  TestEventLoopFinisher killer4(&loop4, 10000);
  QObject::connect(&hook4, SIGNAL(result(const P1::Core::Service *, bool)), &killer4, SLOT(terminateLoop()));
  hook4.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::Normal);
  loop4.exec();
  ASSERT_FALSE(killer4.isKilledByTimeout());

  ASSERT_EQ(1, spy7.count());
  ASSERT_EQ(0, spy8.count());

  QList<QVariant> arguments4 = spy7.takeFirst();
  ASSERT_EQ(service.data(), arguments4.at(0).value<const P1::Core::Service *>());
  ASSERT_TRUE(arguments4.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  QString lastmodified4 = loadLastModifiedDate("300002010000000000");
  ASSERT_TRUE(lastmodified2 == lastmodified4);
}

TEST_F(CheckUpdateHookTest, forceStartTest)
{
  QEventLoop loop;
  P1::GameDownloader::CheckUpdateHelper hook;

  QScopedPointer<P1::Core::Service> service(buildService(QUrl("http://fs0.protocol.one/update/aika/")));

  QSignalSpy spy1(&hook, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy2(&hook, SIGNAL(error(const P1::Core::Service *)));

  TestEventLoopFinisher killer(&loop, 10000);
  QObject::connect(&hook, SIGNAL(result(const P1::Core::Service *, bool)), &killer, SLOT(terminateLoop()));

  // Just download and save last modified date.
  hook.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::Normal);
  loop.exec();
  ASSERT_FALSE(killer.isKilledByTimeout());

  ASSERT_EQ(1, spy1.count());
  ASSERT_EQ(0, spy2.count());

  QList<QVariant> arguments = spy1.takeFirst();
  ASSERT_EQ(service.data(), arguments.at(0).value<const P1::Core::Service *>());

  QString expectedTorrentFilePath = filePath;
  expectedTorrentFilePath.append("/live/300002010000000000.torrent");
  ASSERT_TRUE(arguments.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));

  // Now lets check again with force download torrent and we should get same date and true in event.
  P1::GameDownloader::CheckUpdateHelper hook2;
  QSignalSpy spy3(&hook2, SIGNAL(result(const P1::Core::Service *, bool)));
  QSignalSpy spy4(&hook2, SIGNAL(error(const P1::Core::Service *)));
  QEventLoop loop2;
  TestEventLoopFinisher killer2(&loop2, 10000);
  QObject::connect(&hook2, SIGNAL(result(const P1::Core::Service *, bool)), &killer2, SLOT(terminateLoop()));
  hook2.startCheck(service.data(), P1::GameDownloader::CheckUpdateHelper::ForceDownloadTorrent);
  loop2.exec();
  ASSERT_FALSE(killer2.isKilledByTimeout());

  ASSERT_EQ(1, spy3.count());
  ASSERT_EQ(0, spy4.count());

  QList<QVariant> arguments2 = spy3.takeFirst();
  ASSERT_EQ(service.data(), arguments2.at(0).value<const P1::Core::Service *>());
  ASSERT_TRUE(arguments2.at(1).toBool());
  ASSERT_TRUE(QFile::exists(expectedTorrentFilePath));
}