#include <TestEventLoopFinisher.h>
#include <FileUtils.h>
#include <GameDownloader/Behavior/BindiffBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <UpdateSystem/Hasher/Md5FileHasher.h>
#include <Core/Service.h>
#include <LibtorrentWrapper/Wrapper.h>

#include <gtest/gtest.h>
#include <QtCore/QStringList>
#include <QtCore/QDataStream>
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

QStringList getFileList(const QString& directory) 
{
    QStringList result;

    scanDir(directory.size(), result, directory);
    return result;
}

TEST(BehaviorBindiff, BehaviorBindiffTest)
{
  P1::Libtorrent::Wrapper torrentWrapper;
  P1::GameDownloader::Behavior::BindiffBehavior bindiff;
  bindiff.setTorrentWrapper(&torrentWrapper);

  PREPAIR_WORK_SPACE(Bindiff, BehaviorBindiffTest);
  QString root = WORKSPACE_ROOT(Bindiff, BehaviorBindiffTest);
  QString patchVersion = "0-1";

  QStringList files = getFileList(root + "/OriginalFiles/patch/" + patchVersion + "/live");

  P1::GameDownloader::ServiceState state;
  P1::Core::Service service;
  service.setId("123");
  service.setInstallPath(root + "OriginalFiles");
  service.setDownloadPath(root + "OriginalFiles");
  service.setArea(P1::Core::Service::Live);
  
  state.setState(P1::GameDownloader::ServiceState::Started);
  state.setService(&service);
  state.setPatchFiles(files);
  state.setPatchVersion(patchVersion);

  bindiff.start(&state);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 60000);
  killer.setTerminateSignal(&bindiff, SIGNAL(next(int, P1::GameDownloader::ServiceState *)));
  loop.exec();

  ASSERT_FALSE(killer.isKilledByTimeout());

  Q_FOREACH(QString file, files) {
    QString originalFile = root + "OriginalFiles/live/" + file;
    QString patchFile = root + "TestFiles/" + file;
    
    assertFileEqual(originalFile, patchFile);
  }
  
}
