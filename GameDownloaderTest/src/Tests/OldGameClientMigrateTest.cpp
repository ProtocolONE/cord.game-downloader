#include <GameDownloader/Hooks/OldGameClientMigrate.h>
#include <GameDownloader/GameDownloadService>
#include <Core/Service>
#include <Core/UI/Message>
#include <Core/UI/MessageAdapter>

#include <UpdateSystem/Hasher/Md5FileHasher>

#include <Settings/Settings>

#include <MessageAdapterMock.h>
#include <FileUtils.h>

#include <gtest/gtest.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QTextStream>
#include <QtCore/QFile>
#include <QtCore/QDebug>

#include <Windows.h>

using namespace P1::GameDownloader;
using namespace P1::GameDownloader::Hooks;
using namespace P1::Core;
using P1::Core::UI::Message;

class GameDownloadServicePublic : public GameDownloadService 
{ 
  public: using GameDownloadService::setIsInstalled;

  bool isStoppedOrStopping(const P1::Core::Service *service) {
    return false;
  }
}; 

class OldGameClientMigrateTest : public ::testing::Test
{
public:
  void SetUp()
  {
    Message::setAdapter(&_messageAdapter);

    QString root("C:/tmp/OldGameClientMigrateTest");
    destinationArchive = root + "/OldGameClientMigrate/";
    destinationInstall = root + "/OldGameClientMigrate2/";
    sourcePathToInstaller = root + "/OldGameClientMigrateSource";
    sourceInstallDirectory = root + "/OldGameClientMigrateSource2";
    FileUtils::removeDir(destinationInstall);
    FileUtils::removeDir(destinationArchive);
    FileUtils::removeDir(sourcePathToInstaller);
    FileUtils::removeDir(sourceInstallDirectory);

    QDir dir;
    dir.mkpath(destinationArchive);
    dir.mkpath(destinationInstall);
    dir.mkpath(sourcePathToInstaller);
    dir.mkpath(sourceInstallDirectory);

    service.setArea(Service::Live);
    service.setInstallPath(destinationInstall);
    service.setDownloadPath(destinationArchive);
    service.setId("fake300002010000000000");
    service.setIsDownloadable(true);
    service.setHashDownloadPath(true);
    service.setName("hehe");
    writeStringToRegGNA(service.id(), "PathToInstaller", sourcePathToInstaller);
    writeStringToRegGNA(service.id(), "INSTALL_DIRECTORY", sourceInstallDirectory);

    gameDownloader.setIsInstalled(service.id(), false);

    P1::Settings::Settings settings;
    settings.beginGroup("GameDownloader");
    settings.beginGroup(service.id());
    settings.beginGroup("OldGameClientMigrate");
    settings.setValue("migrateFinished", 0);
  }

  void writeFakeFile(const QString& filePath)
  {
    this->writeFakeFile(filePath, "fakedata");
  }

  void writeFakeFile(const QString& filePath, const QString& data)
  {
    QFileInfo info(filePath);
    QString dirName = info.absolutePath();
    QDir dir;
    dir.mkpath(dirName);

    QFile f(filePath);
    f.open(QIODevice::ReadWrite);
    QTextStream out(&f);
    out << data;
    f.close();
  }

  bool writeStringToRegGNA(const QString& id, const QString& name, const QString& value)
  {
    QString key = QString("SOFTWARE\\ProtocolOne\\Launcher\\%1").arg(id);

    wchar_t tmp[MAX_PATH] = {0};
    key.toWCharArray(tmp);

    HKEY hkey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, tmp, 0, KEY_WOW64_64KEY | KEY_READ | KEY_WRITE, &hkey) != ERROR_SUCCESS) {
      DWORD disposition;
      DWORD createResult = RegCreateKeyEx(
        HKEY_LOCAL_MACHINE, 
        tmp, 
        0, 
        0, 
        0, 
        KEY_WOW64_64KEY | KEY_READ | KEY_WRITE, 
        0, 
        &hkey, 
        &disposition);
      if (createResult != ERROR_SUCCESS)
        return false;
    }

    wchar_t tmp2[MAX_PATH] = {0};
    name.toWCharArray(tmp2);

    wchar_t tmp3[MAX_PATH*4] = {0};
    value.toWCharArray(tmp3);

    DWORD res = RegSetValueExW(hkey, tmp2, 0, REG_SZ, reinterpret_cast<LPBYTE>(tmp3), value.size()*2 + 2);
    RegCloseKey(hkey);
    return res == ERROR_SUCCESS;
  }

  bool checkFiles(const QString& file1, const QString& file2)
  {
    P1::Hasher::Md5FileHasher hasher;
    return hasher.getFileHash(file1) == hasher.getFileHash(file2);
  }

  MessageAdapterMock _messageAdapter;
  OldGameClientMigrate hook;
  Service service;
  GameDownloadServicePublic gameDownloader;
  QString destinationArchive;
  QString destinationInstall;
  QString sourcePathToInstaller;
  QString sourceInstallDirectory;
};

TEST_F(OldGameClientMigrateTest, SimpleMigrateCheck)
{
  QString filePath1 = QString("%1/test.txt").arg(sourcePathToInstaller);
  QString filePath2 = QString("%1/test1/test.txt").arg(sourcePathToInstaller);
  writeFakeFile(filePath1);
  writeFakeFile(filePath2);

  ASSERT_EQ(P1::GameDownloader::HookBase::CheckUpdate, hook.beforeDownload(&gameDownloader, &service));

  QString destFilePath1 = QString("%1/live/test.txt").arg(destinationArchive);
  QString destFilePath2 = QString("%1/live/test1/test.txt").arg(destinationArchive);
  ASSERT_TRUE(QFile::exists(destFilePath1));
  ASSERT_TRUE(QFile::exists(destFilePath2));
  ASSERT_TRUE(checkFiles(filePath1, destFilePath1));
  ASSERT_TRUE(checkFiles(filePath2, destFilePath2));
}

TEST_F(OldGameClientMigrateTest, SimpleMigrateCheckWithoutArchive)
{
  QString filePath1 = QString("%1/test.txt").arg(sourcePathToInstaller);
  QString filePath2 = QString("%1/test1/test.txt").arg(sourcePathToInstaller);
  writeFakeFile(filePath1);
  writeFakeFile(filePath2);

  QString noaFilePath1 = QString("%1/test.txt").arg(sourceInstallDirectory);
  QString noaFilePath2 = QString("%1/test1/test.txt").arg(sourceInstallDirectory);
  writeFakeFile(noaFilePath1, "unpacked file data");
  writeFakeFile(noaFilePath2, "unpacked file data");
  
  service.setHashDownloadPath(false);
  service.setDownloadPath(destinationInstall);
  ASSERT_EQ(P1::GameDownloader::HookBase::CheckUpdate, hook.beforeDownload(&gameDownloader, &service));

  QString destFilePath1 = QString("%1/live/test.txt").arg(destinationArchive);
  QString destFilePath2 = QString("%1/live/test1/test.txt").arg(destinationArchive);
  ASSERT_FALSE(QFile::exists(destFilePath1));
  ASSERT_FALSE(QFile::exists(destFilePath2));

  QString qdestFilePath1 = QString("%1/live/test.txt").arg(destinationInstall);
  QString qdestFilePath2 = QString("%1/live/test1/test.txt").arg(destinationInstall);
  ASSERT_TRUE(QFile::exists(qdestFilePath1));
  ASSERT_TRUE(QFile::exists(qdestFilePath2));

  ASSERT_TRUE(checkFiles(noaFilePath1, qdestFilePath1));
  ASSERT_TRUE(checkFiles(noaFilePath2, qdestFilePath2));
}

TEST_F(OldGameClientMigrateTest, OverwriteMigrateCheck)
{
  QString filePath1 = QString("%1/test.txt").arg(sourcePathToInstaller);
  QString filePath2 = QString("%1/test1/test.txt").arg(sourcePathToInstaller);
  writeFakeFile(filePath1);
  writeFakeFile(filePath2);

  QString destFilePath1 = QString("%1/live/test.txt").arg(destinationArchive);
  QString destFilePath2 = QString("%1/live/test1/test.txt").arg(destinationArchive);

  writeFakeFile(destFilePath1, "wrongfakedata");
  ASSERT_FALSE(checkFiles(filePath1, destFilePath1));

  ASSERT_EQ(P1::GameDownloader::HookBase::CheckUpdate, hook.beforeDownload(&gameDownloader, &service));

  ASSERT_TRUE(QFile::exists(destFilePath1));
  ASSERT_TRUE(QFile::exists(destFilePath2));

  ASSERT_TRUE(checkFiles(filePath1, destFilePath1));
  ASSERT_TRUE(checkFiles(filePath2, destFilePath2));
}

TEST_F(OldGameClientMigrateTest, SecondMigrateCheck)
{
  QString filePath1 = QString("%1/test.txt").arg(sourcePathToInstaller);
  QString filePath2 = QString("%1/test1/test.txt").arg(sourcePathToInstaller);
  writeFakeFile(filePath1);
  writeFakeFile(filePath2);

  QString destFilePath1 = QString("%1/live/test.txt").arg(destinationArchive);
  QString destFilePath2 = QString("%1/live/test1/test.txt").arg(destinationArchive);

  ASSERT_EQ(P1::GameDownloader::HookBase::CheckUpdate, hook.beforeDownload(&gameDownloader, &service));

  ASSERT_TRUE(QFile::exists(destFilePath1));
  ASSERT_TRUE(QFile::exists(destFilePath2));

  ASSERT_TRUE(checkFiles(filePath1, destFilePath1));
  ASSERT_TRUE(checkFiles(filePath2, destFilePath2));

  writeFakeFile(destFilePath1, "wrongfakedata");
  ASSERT_FALSE(checkFiles(filePath1, destFilePath1));

  writeFakeFile(destFilePath2, "wrongfakedata");
  ASSERT_FALSE(checkFiles(filePath2, destFilePath2));

  ASSERT_EQ(P1::GameDownloader::HookBase::Continue, hook.beforeDownload(&gameDownloader, &service));

  ASSERT_FALSE(checkFiles(filePath1, destFilePath1));
  ASSERT_FALSE(checkFiles(filePath2, destFilePath2));
}