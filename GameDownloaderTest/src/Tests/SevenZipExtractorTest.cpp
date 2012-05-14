#include <GameDownloader/Extractor/SevenZipGameExtractor.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/HookBase.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service>
#include <Settings/Settings>

#include <UpdateSystem/UpdateFileInfo>

#include "LambdaHook.h"
#include "TestEventLoopFinisher.h"
#include "FakeCheckUpdateAdapter.h"
#include "FileUtils.h"

#include <gtest/gtest.h>

#include <QtCore/QObject>
#include <QtCore/QCoreApplication>
#include <QtCore/QTime>
#include <QtCore/QDirIterator>
#include <QtCore/QHash>
#include <QtCore/QDataStream>
#include <QtTest/QSignalSpy>

void scanDir(int removeLength, QHash<QString, QString> &result, QDir dir)
{
  dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

  QStringList fileList = dir.entryList();
  QString absolutePath = dir.absolutePath();

  for (int i=0; i<fileList.count(); i++)
  {
    QString file = QString("%1/%2").arg(absolutePath).arg(fileList[i]);
    result[file.right(file.length() - removeLength)] = file;
  }

  dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
  QStringList dirList = dir.entryList();
  for (int i=0; i<dirList.size(); ++i)
  {
    QString newPath = QString("%1/%2").arg(dir.absolutePath()).arg(dirList.at(i));
    scanDir(removeLength, result, QDir(newPath));
  }
}

void RecursionDirectoryScanTest(QHash<QString, QString> &result)
{
  QString str("D:/AIKA/AIKA3/");
  int len = str.length();
  QDir dir(str);
  scanDir(len, result, dir);
}

void QDirIteratorSpeedTest(QHash<QString, QString> &result)
{
  QString str("D:/AIKA/AIKA3/");
  int len = str.length();
  QDir dir(str);
  // Note that "if (!it.filinfo().isDir())" faster than setFilter
  //dir.setFilter(QDir::Files | QDir::NoSymLinks);

  QDirIterator it(dir, QDirIterator::IteratorFlag::Subdirectories);
  while (it.hasNext()) {
    QString file = it.next();
    if (!it.fileInfo().isDir()) 
    {
      result[file.right(file.length() - len)] = file;
    }
   
  }
}

TEST(dirScan, DISABLED_dirScan) 
{
  // Speed test for dirictory scan
  
  // QdirIterator
  QTime time1;
  time1.setHMS(0,0,0,0);
  time1.start();
  QHash<QString, QString> table2;
  QDirIteratorSpeedTest(table2);
  int result2 = time1.elapsed();


  QTime time;
  time.setHMS(0,0,0,0);
  time.start();
  QHash<QString, QString> table1;
  RecursionDirectoryScanTest(table1);
  int result1 = time.elapsed();
}

class SevenZipGameExtractorTest : public ::testing::Test{
public:
  GGS::GameDownloader::Extractor::SevenZipGameExtractor extractor;
  GGS::Core::Service service;
  GGS::GameDownloader::GameDownloadService gameDownloaderService;
  FakeCheckUpdateAdapter *checkUpdateAdapter;
  LambdaHook *hook;

  QSignalSpy *spy_extractFinished;
  QSignalSpy *spy_extractPaused;
  QSignalSpy *spy_extractFailed;
  QSignalSpy *spy_pauseRequest;
  QSignalSpy *spy_extractionProgressChanged;

  virtual void SetUp() {
    using namespace GGS::GameDownloader;
    using namespace GGS::GameDownloader::Extractor;
    using namespace GGS::Core;

    this->spy_extractFinished = new QSignalSpy(&this->extractor, SIGNAL(extractFinished(const GGS::Core::Service *)));
    this->spy_extractPaused = new QSignalSpy(&this->extractor, SIGNAL(extractPaused(const GGS::Core::Service *)));
    this->spy_extractFailed = new QSignalSpy(&this->extractor, SIGNAL(extractFailed(const GGS::Core::Service *)));
    this->spy_pauseRequest = new QSignalSpy(&this->extractor, SIGNAL(pauseRequest(const GGS::Core::Service *)));
    this->spy_extractionProgressChanged = new QSignalSpy(&this->extractor, SIGNAL(extractionProgressChanged(const QString&, qint8, qint64, qint64)));

    service.setTorrentUrl(QUrl("http://gnlupdate.tst.local/update/qgna_fixtures/"));
    service.setArea(Service::Live);
    QString root = QCoreApplication::applicationDirPath();
    QString fileArchivePath = root;
    fileArchivePath.append("/game/archive");
    QString fileGamePath = root;
    fileGamePath.append("/game/game/");

    service.setTorrentFilePath(fileArchivePath);
    service.setDownloadPath(fileArchivePath);
    service.setExtractionPath(fileGamePath);

    service.setId("300003010000000000");
    service.setExtractorType(extractor.extractorId());

    gameDownloaderService.init();
    checkUpdateAdapter = new FakeCheckUpdateAdapter(&this->gameDownloaderService);

    gameDownloaderService.registerExtractor(&extractor);

    DOWNLOADERHOOK(preHook) {
      return GGS::GameDownloader::HookBase::ExtractionStartPoint;
    };

    DOWNLOADERHOOK(postHook) {
      return GGS::GameDownloader::HookBase::Finish;
    };

    hook = new LambdaHook(1, preHook, postHook);
    gameDownloaderService.registerHook(service.id(), 0, 0, hook);

    FileUtils::removeDir(fileArchivePath);
    FileUtils::removeDir(fileGamePath);

    QHash<QString, GGS::UpdateSystem::UpdateFileInfo> emptyHash;
    this->saveUpdateInfo(&service, emptyHash);
  }

  virtual void TearDown() {
    delete this->checkUpdateAdapter;
    delete hook;

    delete spy_extractFinished;
    delete spy_extractPaused;
    delete spy_extractFailed;
    delete spy_pauseRequest;
    delete spy_extractionProgressChanged;
  }

  void resetSignals() 
  {
    spy_extractFinished->clear();
    spy_extractPaused->clear();
    spy_extractFailed->clear();
    spy_pauseRequest->clear();
    spy_extractionProgressChanged->clear();
  }

  QString getServiceAreaString(const GGS::Core::Service * service)
  {
    switch(service->area()) {
    case GGS::Core::Service::Live:;
      return QString("live");
    case GGS::Core::Service::Pts:
      return QString("pts");
    case GGS::Core::Service::Tst:
      return QString("tst");
    default:
      return QString("");
    }
  }

  void copyFromFixturesToArchiveDirectory(const QStringList &realtivePathNames)
  {
    Q_FOREACH(QString fileName, realtivePathNames) {
      this->copyFromFixturesToArchiveDirectory(fileName);
    }
  }

  void copyFromFixturesToArchiveDirectory(const QString &realtivePathName)
  {
    QString archivePath = QString("%1/%2/%3").arg(this->service.downloadPath(), 
      this->getServiceAreaString(&this->service),
      realtivePathName);

    QString cleanTargetFilePath = QDir::cleanPath(archivePath);

    int lastIndex = cleanTargetFilePath.lastIndexOf('/');
    QString targetDirectory;
    if(lastIndex != -1) {
      targetDirectory = cleanTargetFilePath.left(lastIndex + 1);
      QDir targetPath(cleanTargetFilePath);
      if(!targetPath.exists(targetDirectory)) {
        targetPath.mkpath(targetDirectory);
      }
    }

    QString fixturePath = QString("%1/fixtures/%2").arg(QCoreApplication::applicationDirPath(), realtivePathName);

    if (!QFile::exists(fixturePath)) {
      qCritical() << "unknwonw fixture " << realtivePathName;
      return;
    }

    QFile::copy(fixturePath, archivePath);
  }

  void saveUpdateInfo(const GGS::Core::Service * service, QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash)
  {
    QByteArray byteArray;
    QDataStream stream(&byteArray, QIODevice::WriteOnly);
    stream << resultHash;

    GGS::Settings::Settings settings;
    settings.beginGroup("GameDownloader");
    settings.beginGroup("SevenZipGameExtractor");
    settings.setValue(service->id(), byteArray);
  }

  void corruptGameFile(const QString& realtivePathName) 
  {
    QString filePath = QString("%1/%2/%3").arg(this->service.extractionPath(), 
      this->getServiceAreaString(&this->service),
      realtivePathName);
  
    if (!QFile::exists(filePath)) {
      qCritical() << "Can't corrupt file. File not found.";
      return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
      qCritical() << "Can't corrupt file. File is locked.";
      return;
    }

    QDataStream stream(&file);
    stream << "really wrong data. it can't be right. i'm sure." << QDateTime::currentMSecsSinceEpoch();
    file.close();
  }

  void removeGameFile(const QString& realtivePathName) 
  {
    QString filePath = QString("%1/%2/%3").arg(this->service.extractionPath(), 
      this->getServiceAreaString(&this->service),
      realtivePathName);

    if (!QFile::exists(filePath)) {
      qCritical() << "File not found.";
      return;
    }

    QFile::remove(filePath);
  }
};

TEST_F(SevenZipGameExtractorTest, AllFilesNotExists)
{
  using namespace GGS::GameDownloader;
  using namespace GGS::GameDownloader::Extractor;
  using namespace GGS::Core;
  // есть в фикстурках, если пригодиться можно поиспользовать
  //<< "update.crc.7z"
  //<< "3000.torrent.7z"

  QString rusfile1 = QString::fromUtf8(QByteArray::fromHex(QString("d09fd0b0d0bfd0bad0b0d181d184d0b0d0b9d0bbd0b0d0bcd0b85cd0bfd180d0bed187d182d0b85fd0bcd0b5d0bdd18f2ed182d0b5d0bad181d1822e377a").toLatin1()));
  QString rusfile2 = QString::fromUtf8(QByteArray::fromHex(QString("d09fd0b0d0bfd0bad0b0d181d184d0b0d0b9d0bbd0b0d0bcd0b85cd09fd0bed0b4d09fd0b0d0bfd0bad0b0d0a1d0a4d0b0d0b9d0bbd0b0d0bcd0b85cd09fd180d0bed187d182d0b85fd09cd0b5d0bdd18f5fd09fd0bed0b6d0b0d0bbd183d0b9d181d182d0b02ed182d0b5d0bad181d1822e377a").toLatin1()));

  QString rusfile2_without_7z = QString::fromUtf8(QByteArray::fromHex(QString("d09fd0b0d0bfd0bad0b0d181d184d0b0d0b9d0bbd0b0d0bcd0b85cd09fd0bed0b4d09fd0b0d0bfd0bad0b0d0a1d0a4d0b0d0b9d0bbd0b0d0bcd0b85cd09fd180d0bed187d182d0b85fd09cd0b5d0bdd18f5fd09fd0bed0b6d0b0d0bbd183d0b9d181d182d0b02ed182d0b5d0bad181d182").toLatin1()));
  QStringList fixtures;
  fixtures << "AirGarden.mp3.7z"
    << "Basilan.mp3.7z"
    << "test.txt.7z"
    << "SubDir\\readme.txt.7z"
    //<< QString::fromLocal8Bit("Папкасфайлами\\прочти_меня.текст.7z")
    << rusfile1
    //<< QString::fromLocal8Bit("Папкасфайлами\\ПодПапкаСФайлами\\Прочти_Меня_Пожалуйста.текст.7z");
    << rusfile2;

  this->copyFromFixturesToArchiveDirectory(fixtures);

  gameDownloaderService.start(&service, GGS::GameDownloader::StartType::Normal);

  QEventLoop loop;
  TestEventLoopFinisher loopKiller(&loop, 10000);
  QObject::connect(&gameDownloaderService, SIGNAL(finished(const GGS::Core::Service *)), &loopKiller, SLOT(terminateLoop()));

  loop.exec();
  ASSERT_FALSE(loopKiller.isKilledByTimeout());

  ASSERT_EQ(1, spy_extractFinished->count());
  ASSERT_EQ(0, spy_extractPaused->count());
  ASSERT_EQ(0, spy_extractFailed->count());
  ASSERT_EQ(0, spy_pauseRequest->count());

  ASSERT_EQ(2 + fixtures.count() * 2, spy_extractionProgressChanged->count());
  for (int i = 0; i < spy_extractionProgressChanged->count(); ++i) {
    ASSERT_EQ(this->service.id(), spy_extractionProgressChanged->at(i).at(0).toString());
  }
  
  ASSERT_EQ(0, spy_extractionProgressChanged->at(0).at(2).toInt());
  ASSERT_EQ(0, spy_extractionProgressChanged->at(0).at(3).toInt());

  ASSERT_EQ(0, spy_extractionProgressChanged->at(1 + fixtures.count()).at(2).toInt());
  ASSERT_EQ(fixtures.count(), spy_extractionProgressChanged->at(1 + fixtures.count()).at(3).toInt());

  for (int i = 2 + fixtures.count(); i < spy_extractionProgressChanged->count(); ++i) {
    ASSERT_EQ(i- 1 - fixtures.count(), spy_extractionProgressChanged->at(i).at(2).toInt());
    ASSERT_EQ(fixtures.count(), spy_extractionProgressChanged->at(i).at(3).toInt());
  }

  // Let's recheck files, we shouldn't get any extra progress event except first one
  this->resetSignals();

  gameDownloaderService.start(&service, GGS::GameDownloader::StartType::Force);
  QEventLoop loop1;
  TestEventLoopFinisher loopKiller1(&loop1, 10000);
  QObject::connect(&gameDownloaderService, SIGNAL(finished(const GGS::Core::Service *)), &loopKiller1, SLOT(terminateLoop()));

  loop1.exec();
  ASSERT_FALSE(loopKiller1.isKilledByTimeout());

  EXPECT_EQ(1, spy_extractFinished->count());
  EXPECT_EQ(0, spy_extractPaused->count());
  EXPECT_EQ(0, spy_extractFailed->count());
  EXPECT_EQ(0, spy_pauseRequest->count());
  EXPECT_EQ(1 + fixtures.count(), spy_extractionProgressChanged->count());

  // let's currupt two files: one with recheck flag and one without;
  // also remove one file without flag
  // We should get two extracted files and one skipped
  this->resetSignals();
  this->removeGameFile("AirGarden.mp3");
  this->corruptGameFile("SubDir/readme.txt");
  this->corruptGameFile(rusfile2_without_7z);

  gameDownloaderService.start(&service, GGS::GameDownloader::StartType::Force);
  QEventLoop loop2;
  TestEventLoopFinisher loopKiller2(&loop2, 5000);
  QObject::connect(&gameDownloaderService, SIGNAL(finished(const GGS::Core::Service *)), &loopKiller2, SLOT(terminateLoop()));
  loop2.exec();

  ASSERT_FALSE(loopKiller2.isKilledByTimeout());
  
  EXPECT_EQ(1, spy_extractFinished->count());
  EXPECT_EQ(0, spy_extractPaused->count());
  EXPECT_EQ(0, spy_extractFailed->count());
  EXPECT_EQ(0, spy_pauseRequest->count());
  EXPECT_EQ(4 +  fixtures.count(), spy_extractionProgressChanged->count());

  // let's corrupt two files(with and without flags) and start with force recheck
  this->resetSignals();
  this->corruptGameFile("SubDir/readme.txt");
  this->corruptGameFile(rusfile2_without_7z);

  gameDownloaderService.start(&service, GGS::GameDownloader::StartType::Recheck);
  QEventLoop loop3;
  TestEventLoopFinisher loopKiller3(&loop3, 5000);
  QObject::connect(&gameDownloaderService, SIGNAL(finished(const GGS::Core::Service *)), &loopKiller3, SLOT(terminateLoop()));
  loop3.exec();

  ASSERT_FALSE(loopKiller3.isKilledByTimeout());

  EXPECT_EQ(1, spy_extractFinished->count());
  EXPECT_EQ(0, spy_extractPaused->count());
  EXPECT_EQ(0, spy_extractFailed->count());
  EXPECT_EQ(0, spy_pauseRequest->count());
  EXPECT_EQ(4 + fixtures.count(), spy_extractionProgressChanged->count());
}