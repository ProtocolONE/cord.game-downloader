#include <TestEventLoopFinisher.h>
#include <FileUtils.h>
#include <GameDownloader/Extractor/SevenZipGameExtractor.h>
#include <GameDownloader/Behavior/CompressorBehavior.h>
#include <GameDownloader/ServiceState>
#include <UpdateSystem/Hasher/Md5FileHasher.h>
#include <Core/Service>

#include <gtest/gtest.h>
#include <QtCore/QStringList>
#include <Settings/Settings>
#include <QEventLoop>

#define assertFileEqual(f1, f2) { \
  GGS::Hasher::Md5FileHasher hasher; \
  ASSERT_EQ(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

#define assertFileUnEqual(f1, f2) { \
  GGS::Hasher::Md5FileHasher hasher; \
  ASSERT_NE(hasher.getFileHash(f1), hasher.getFileHash(f2)); \
}

void scanDir(int removeLength, QStringList &result, QDir dir);
QStringList getFileList(const QString& directory);

TEST(CompressorBindiff, CompressorBindiffTest)
{
  GGS::GameDownloader::Behavior::CompressorBehavior compressor;
  GGS::GameDownloader::Extractor::SevenZipGameExtractor extractor;
  compressor.registerCompressor(&extractor);

  PREPAIR_WORK_SPACE(CompressorBindiff, CompressorBindiffTest);
  QString root = WORKSPACE_ROOT(CompressorBindiff, CompressorBindiffTest);

  QStringList files = getFileList(root + "/live");

  GGS::GameDownloader::ServiceState state;
  GGS::Core::Service service;
  service.setId("123");
  service.setInstallPath(root);
  service.setDownloadPath(root + "/dist");
  service.setArea(GGS::Core::Service::Live);
  service.setExtractorType(extractor.extractorId());

  state.setState(GGS::GameDownloader::ServiceState::Started);
  state.setService(&service);
  state.setPackingFiles(files);

  compressor.start(&state);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 6000);
  killer.setTerminateSignal(&compressor, SIGNAL(next(int, GGS::GameDownloader::ServiceState *)));
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());

  Q_FOREACH(QString file, files) {
    QString arcFile = root + "dist/live/" + file + ".7z";
    ASSERT_TRUE(QFile::exists(arcFile));
  }  
}