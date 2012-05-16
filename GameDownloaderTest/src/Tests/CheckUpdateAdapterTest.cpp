#include "TestEventLoopFinisher.h"
#include "FileUtils.h"

#include <GameDownloader/CheckUpdateAdapter.h>
#include <Settings/Settings>
#include <Core/Service>

#include <gtest/gtest.h>
#include <QtTest/QSignalSpy>
#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QCoreApplication>

using namespace GGS::Settings;
using namespace GGS::GameDownloader;

class CheckUpdateAdapterTest : public ::testing::Test {
protected:
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

  GGS::Core::Service *buildService(QUrl &url) {
    GGS::Core::Service *service = new GGS::Core::Service();
    service->setTorrentUrl(url);
    QString root = QCoreApplication::applicationDirPath();
    filePath = root.append("/game");

    FileUtils::removeDir(filePath);

    service->setTorrentFilePath(filePath);
    service->setId("300002010000000000");
    service->setArea(GGS::Core::Service::Live);

    this->saveLastModifiedDate("", "300002010000000000");

    return service;
  }

  virtual void SetUp() {
    killer = new TestEventLoopFinisher(&this->loop, 10000);
    spyComplete = new QSignalSpy(&adapter, SIGNAL(checkUpdateCompleted(const GGS::Core::Service *, bool)));
    spyFailed = new QSignalSpy(&adapter, SIGNAL(checkUpdateFailed(const GGS::Core::Service *)));
  }

  virtual void TearDown() {
    ASSERT_FALSE(killer->isKilledByTimeout());
    delete killer;
  }

  QString filePath;
  QEventLoop loop;
  CheckUpdateAdapter adapter;
  TestEventLoopFinisher *killer;
  QSignalSpy *spyComplete;
  QSignalSpy *spyFailed;
};

TEST_F(CheckUpdateAdapterTest, NormalTest)
{
  QObject::connect(&adapter, SIGNAL(checkUpdateCompleted(const GGS::Core::Service *, bool)), killer, SLOT(terminateLoop()));

  QScopedPointer<GGS::Core::Service> service(buildService(QUrl("http://fs0.gamenet.ru/update/aika/")));
  adapter.checkUpdateRequest(service.data(), CheckUpdateHelper::Normal);

  loop.exec();
  
  ASSERT_EQ(1, spyComplete->count());
  ASSERT_EQ(0, spyFailed->count());

  QList<QVariant> arguments = spyComplete->takeFirst();
  ASSERT_EQ(service.data(), arguments.at(0).value<const GGS::Core::Service *>());
  ASSERT_TRUE(arguments.at(1).toBool());
}

TEST_F(CheckUpdateAdapterTest, FailTest)
{
  QObject::connect(&adapter, SIGNAL(checkUpdateFailed(const GGS::Core::Service *)), killer, SLOT(terminateLoop()));

  QScopedPointer<GGS::Core::Service> service(buildService(QUrl("http://fs0wfake.gamenetfake.ru/update/aika/")));
  adapter.checkUpdateRequest(service.data(), CheckUpdateHelper::Normal);

  loop.exec();

  ASSERT_EQ(0, spyComplete->count());
  ASSERT_EQ(1, spyFailed->count());

  QList<QVariant> arguments = spyFailed->takeFirst();
  ASSERT_EQ(service.data(), arguments.at(0).value<const GGS::Core::Service *>());
}