#include <TestEventLoopFinisher.h>
#include <FileUtils.h>
#include <GameDownloader/Behavior/BindiffBehavior.h>
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

QByteArray serialize(QStringList stringList)
{
    QByteArray byteArray;
    QDataStream out(&byteArray, QIODevice::WriteOnly);
    out << stringList;
    return byteArray;
}

void scanDir(int removeLength, QStringList &result, QDir dir)
{
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QStringList fileList = dir.entryList();
    QString absolutePath = dir.absolutePath();

    for (int i = 0; i < fileList.count(); i++)
    {
        QString file = QString("%1/%2").arg(absolutePath).arg(fileList[i]);
        result << file.right(file.length() - removeLength);
    }

    dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
    QStringList dirList = dir.entryList();
    for (int i = 0; i < dirList.size(); ++i)
    {
        QString newPath = QString("%1/%2").arg(dir.absolutePath()).arg(dirList.at(i));
        scanDir(removeLength, result, QDir(newPath));
    }
}

QStringList getFileList(const QString& directory) {
    QStringList result;

    scanDir(directory.size(), result, directory);
    return result;
}

TEST(BehaviorBindiff, BehaviorBindiffTest)
{
  GGS::GameDownloader::Behavior::BindiffBehavior bindiff;

  PREPAIR_WORK_SPACE(Bindiff, BehaviorBindiffTest);
  QString root = WORKSPACE_ROOT(Bindiff, BehaviorBindiffTest);
  QString patchVersion = "0-1";

  QStringList files = getFileList(root + "/OriginalFiles/patch/" + patchVersion + "/live");

  GGS::GameDownloader::ServiceState state;
  GGS::Core::Service service;
  service.setId("123");
  service.setInstallPath(root + "OriginalFiles");
  service.setDownloadPath(root + "OriginalFiles");
  service.setArea(GGS::Core::Service::Live);
  
  state.setState(GGS::GameDownloader::ServiceState::Started);
  state.setService(&service);
  state.setPatchFiles(files);
  state.setPatchVersion(patchVersion);

  bindiff.start(&state);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 60000);
  killer.setTerminateSignal(&bindiff, SIGNAL(next(int, GGS::GameDownloader::ServiceState *)));
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());

  Q_FOREACH(QString file, files) {
    QString originalFile = root + "OriginalFiles/live/" + file;
    QString patchFile = root + "TestFiles/" + file;
    
    assertFileEqual(originalFile, patchFile);
  }
  
}
