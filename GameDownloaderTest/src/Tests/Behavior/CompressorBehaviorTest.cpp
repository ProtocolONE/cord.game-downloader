#include <TestEventLoopFinisher.h>
#include <FileUtils.h>
#include <GameDownloader/Extractor/SevenZipExtractor.h>
#include <GameDownloader/Extractor/MiniZipExtractor.h>
#include <GameDownloader/Behavior/CompressorBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <UpdateSystem/Hasher/Md5FileHasher.h>
#include <Core/Service.h>

#include <gtest/gtest.h>
#include <QtCore/QStringList>
#include <Settings/Settings.h>
#include <QEventLoop>

#define assertFileEqual(f1, f2) { \
  P1::Hasher::Md5FileHasher hasher; \
  ASSERT_EQ(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

#define assertFileUnEqual(f1, f2) { \
  P1::Hasher::Md5FileHasher hasher; \
  ASSERT_NE(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

void scanDir(int removeLength, QStringList &result, QDir dir);
QStringList getFileList(const QString& directory);

TEST(SevenZipCompressorBindiff, SevenZipCompressorBindiffTest)
{
  P1::GameDownloader::Behavior::CompressorBehavior compressor;
  P1::GameDownloader::Extractor::SevenZipExtractor extractor;
  compressor.registerCompressor(&extractor);

  PREPAIR_WORK_SPACE(CompressorBindiff, CompressorBindiffTest);
  QString root = WORKSPACE_ROOT(CompressorBindiff, CompressorBindiffTest);

  QStringList files = getFileList(root + "/live");

  P1::GameDownloader::ServiceState state;
  P1::Core::Service service;
  service.setId("123");
  service.setInstallPath(root);
  service.setDownloadPath(root + "/dist");
  service.setArea(P1::Core::Service::Live);
  service.setExtractorType(extractor.extractorId());

  state.setState(P1::GameDownloader::ServiceState::Started);
  state.setService(&service);
  state.setPackingFiles(files);

  compressor.start(&state);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 16000);
  killer.setTerminateSignal(&compressor, SIGNAL(next(int, P1::GameDownloader::ServiceState *)));
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());

  Q_FOREACH(QString file, files) {
    QString arcFile = root + "dist/live/" + file + ".7z";
    ASSERT_TRUE(QFile::exists(arcFile));
  }  
}

TEST(MiniZipCompressorBindiff, MiniZipCompressorBindiffTest)
{
  P1::GameDownloader::Behavior::CompressorBehavior compressor;
  P1::GameDownloader::Extractor::MiniZipExtractor extractor;
  compressor.registerCompressor(&extractor);

  PREPAIR_WORK_SPACE(CompressorBindiff, CompressorBindiffTest);
  QString root = WORKSPACE_ROOT(CompressorBindiff, CompressorBindiffTest);

  QStringList files = getFileList(root + "/live");

  P1::GameDownloader::ServiceState state;
  P1::Core::Service service;
  service.setId("123");
  service.setInstallPath(root);
  service.setDownloadPath(root + "/dist");
  service.setArea(P1::Core::Service::Live);
  service.setExtractorType(extractor.extractorId());

  state.setState(P1::GameDownloader::ServiceState::Started);
  state.setService(&service);
  state.setPackingFiles(files);

  compressor.start(&state);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 16000);
  killer.setTerminateSignal(&compressor, SIGNAL(next(int, P1::GameDownloader::ServiceState *)));
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());

  Q_FOREACH(QString file, files) {
    QString arcFile = root + "dist/live/" + file + ".zip";
    ASSERT_TRUE(QFile::exists(arcFile));
  }
}