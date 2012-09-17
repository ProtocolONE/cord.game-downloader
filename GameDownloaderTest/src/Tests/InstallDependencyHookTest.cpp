
#include "FileUtils.h"

#include <GameDownloader/GameDownloadService>
#include <GameDownloader/Hooks/InstallDependency.h>

#include <Core/Service>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>

#include <gtest/gtest.h>

using namespace GGS::GameDownloader;
using namespace GGS::GameDownloader::Hooks;
using namespace GGS::Core;
class InstallDependencyHook : public ::testing::Test
{
public:
  QString redistr1;
  QString redistr2;
  QString noArgumentsString;

  GameDownloadService gameDownloader;
  InstallDependency hook;
  Service service;

  QString createDirectoryIfNotExist(const QString &targetFilePath)
  {
    QFileInfo info(targetFilePath);
    QDir targetPath = info.dir();
    QString path = info.absolutePath();
    if (!targetPath.exists())
      targetPath.mkpath(path);

    return path;
  }

  void copyFromFixturesToInstallDirectory(const QStringList &realtivePathNames)
  {
    Q_FOREACH(QString fileName, realtivePathNames) {
      this->copyFromFixturesToInstallDirectory(fileName);
    }
  }

  void copyFromFixturesToInstallDirectory(const QString &realtivePathName)
  {
    QString targetDirectory = QString("%1/%2/Dependency/%3").arg(this->service.installPath(), this->service.areaString(), realtivePathName);
    this->createDirectoryIfNotExist(targetDirectory);

    QString fixturePath = QString("%1/fixtures/%2").arg(QCoreApplication::applicationDirPath(), realtivePathName);

    if (!QFile::exists(fixturePath)) {
      qCritical() << "unknwonw fixture " << realtivePathName;
      return;
    }

    QFile::copy(fixturePath, targetDirectory);
  }

  void SetUp()
  {
    this->noArgumentsString = "\"NO ARGS\"";
    this->redistr1 = "redistr1.exe";
    this->redistr2 = "redistr2.exe";

    service.setInstallPath(QCoreApplication::applicationDirPath() + "/InstallDependencyHook/");
    service.setId("someTestId");
    QString targetDirectory = QString("%1/%2/Dependency/").arg(this->service.installPath(), this->service.areaString());
    FileUtils::removeDir(targetDirectory);
    QStringList fixtures;
    fixtures << this->redistr1 << this->redistr2;
    this->copyFromFixturesToInstallDirectory(fixtures);
  }

  QString readResult1()
  {
    return this->readResult(this->redistr1 + ".txt");
  }

  QString readResult2()
  {
    return this->readResult(this->redistr2 + ".txt");
  }

  QString readResult(const QString& fileName)
  {
    QString targetPath = QString("%1/%2/Dependency/%3").arg(this->service.installPath(), this->service.areaString(), fileName);
    if (!QFile::exists(targetPath))
      return QString();

    QFile file(targetPath);
    if (!file.open(QIODevice::ReadOnly))
      return QString();

    QString str(file.readAll());
    return str.trimmed();
  }

  bool isResultExist1()
  {
    return this->isResultExist(this->redistr1 + ".txt");
  }

  bool isResultExist2()
  {
    return this->isResultExist(this->redistr2 + ".txt");
  }

  bool isResultExist(const QString& fileName) {
    QString targetPath = QString("%1/%2/Dependency/%3").arg(this->service.installPath(), this->service.areaString(), fileName);
    return QFile::exists(targetPath);
  }
};

TEST_F(InstallDependencyHook, NothingExecuteTest)
{
 service.setExternalDependencyList("");
 hook.afterDownload(&gameDownloader, &service);
 ASSERT_FALSE(this->isResultExist1());
 ASSERT_FALSE(this->isResultExist2());
}

TEST_F(InstallDependencyHook, BothExecuteTest)
{
  QString dependency = QString("%1,,%2,").arg(redistr1, redistr2);
  service.setExternalDependencyList(dependency);
  hook.afterDownload(&gameDownloader, &service);
  ASSERT_TRUE(this->isResultExist1());
  ASSERT_TRUE(this->isResultExist2());
  ASSERT_EQ(noArgumentsString, this->readResult1());
  ASSERT_EQ(noArgumentsString, this->readResult2());
}

TEST_F(InstallDependencyHook, BothExecuteTrickTest)
{
  QString dependency = QString("%1,,%2").arg(redistr1, redistr2);
  service.setExternalDependencyList(dependency);
  hook.afterDownload(&gameDownloader, &service);
  ASSERT_TRUE(this->isResultExist1());
  ASSERT_TRUE(this->isResultExist2());
  ASSERT_EQ(noArgumentsString, this->readResult1());
  ASSERT_EQ(noArgumentsString, this->readResult2());
}

TEST_F(InstallDependencyHook, BothExecuteWithArgsTest)
{
  QString arg1("/q /passive");
  QString arg2("/no active");
  QString dependency = QString("%1,%2,%3,%4").arg(redistr1, arg1, redistr2, arg2);
  service.setExternalDependencyList(dependency);
  hook.afterDownload(&gameDownloader, &service);
  ASSERT_TRUE(this->isResultExist1());
  ASSERT_TRUE(this->isResultExist2());
  ASSERT_EQ(arg1, this->readResult1());
  ASSERT_EQ(arg2, this->readResult2());
}


TEST_F(InstallDependencyHook, ServiceInstalledTest)
{
  class TestClass : public GameDownloadService 
  { 
  public: using GameDownloadService::setIsInstalled;
  }; 

  TestClass testDownloader;
  testDownloader.setIsInstalled(service.id(), true);

  QString arg1("/q /passive");
  QString arg2("/no active");
  QString dependency = QString("%1,%2,%3,%4,fake.exe,asd,qwe").arg(redistr1, arg1, redistr2, arg2);
  service.setExternalDependencyList(dependency);
  hook.afterDownload(&gameDownloader, &service);
  ASSERT_FALSE(this->isResultExist1());
  ASSERT_FALSE(this->isResultExist2());
}

// Keep this for sample.

//TEST_F(InstallDependencyHook, ServiceInstalledTest2)
//{
//  class TestClass : public GameDownloadService 
//  { 
//    friend class InstallDependencyHook_ServiceInstalledTest2_Test; 
//  }; 
//
//  TestClass* pTC = reinterpret_cast<TestClass*> (&this->gameDownloader); 
//  pTC->setIsInstalled(service.id(), true);
//
//  QString arg1("/q /passive");
//  QString arg2("/no active");
//  QString dependency = QString("%1,%2,%3,%4").arg(redistr1, arg1, redistr2, arg2);
//  service.setExternalDependencyList(dependency);
//  hook.afterDownload(&gameDownloader, &service);
//  ASSERT_FALSE(this->isResultExist1());
//  ASSERT_FALSE(this->isResultExist2());
//}