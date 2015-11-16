#include <GameDownloader/GameDownloadService>
#include <GameDownloader/Hooks/RemoveFileHook.h>

#include <Core/Service.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>


#include <TestEventLoopFinisher.h>
#include <FileUtils.h>
#include <ExternalToolHelper.h>

#include <gtest/gtest.h>

using namespace GGS::GameDownloader;
using namespace GGS::GameDownloader::Hooks;
using namespace GGS::Core;

class RemoveFileHookTest : public ::testing::Test
{
public:

  RemoveFileHookTest()
  {
    serviceState.setService(&service);
  }
  GameDownloadService gameDownloader;
  RemoveFileHook hook;
  Service service;
  ServiceState serviceState;


};

TEST_F(RemoveFileHookTest, SuccessDelete)
{
  PREPAIR_WORK_SPACE(RemoveFileHookTest, SuccessDelete);
  QString root = WORKSPACE_ROOT(RemoveFileHookTest, SuccessDelete);

  service.setInstallPath(root);

  RemoveFileHook hook;
  hook.afterDownload(&gameDownloader, &serviceState); 
  // UNDONE 

}