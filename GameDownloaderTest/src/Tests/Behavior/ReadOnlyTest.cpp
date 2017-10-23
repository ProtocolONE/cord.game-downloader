#include <TestEventLoopFinisher.h>
#include <FileUtils.h>
#include <GameDownloader/Behavior/ReadOnlyBehavior.h>

#include <GameDownloader/ServiceState>
#include <Core/Service>

#include <gtest/gtest.h>
#include <QtCore/QStringList>
#include <QEventLoop>

#include <Windows.h>

QStringList getFileList(const QString& directory);

TEST(ReadOnlyTester, ReadOnlyBehaviorTest)
{
  GGS::GameDownloader::Behavior::ReadOnlyBehavior flagDropper;  

  PREPAIR_WORK_SPACE(ReadOnlyDir, ReadOnlyDirTest);
  QString root = WORKSPACE_ROOT(ReadOnlyDir, ReadOnlyDirTest);

  QStringList files = getFileList(root);

  GGS::GameDownloader::ServiceState state;
  GGS::Core::Service service;
  service.setId("123");
  service.setInstallPath(root);
  service.setDownloadPath(root + "/dist");
  service.setArea(GGS::Core::Service::Live);
  
  state.setState(GGS::GameDownloader::ServiceState::Started);
  state.setService(&service);

  Q_FOREACH(QString file, files) {
    QString probeFile = root + file;
    std::wstring fName = probeFile.toStdWString();

    DWORD currAttr = GetFileAttributes(fName.c_str());
    currAttr |= FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;    
    SetFileAttributes(fName.c_str(), currAttr);
  }

  flagDropper.start(&state);

  QEventLoop loop;
  TestEventLoopFinisher killer(&loop, 1600000);
  killer.setTerminateSignal(&flagDropper, SIGNAL(next(int, GGS::GameDownloader::ServiceState *)));
  loop.exec();

  Q_FOREACH(QString file, files) {
    QString probeFile = root + file;
    std::wstring fName = probeFile.toStdWString();

    DWORD currAttr = GetFileAttributes(fName.c_str());
    DWORD probe = FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM;

    bool testFlag = (currAttr & probe) == 0;
    
    ASSERT_EQ(testFlag, true);
  }
}