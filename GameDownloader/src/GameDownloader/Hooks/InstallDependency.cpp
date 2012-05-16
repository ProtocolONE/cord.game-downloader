#include <GameDownloader/Hooks/InstallDependency.h>
#include <GameDownloader/GameDownloadService>

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QDebug>

namespace GGS {
  namespace GameDownloader {
    namespace Hooks {
      InstallDependency::InstallDependency()
         : HookBase("B4910801-2FA4-455E-AEAE-A2BAA2D3E4CA")
      {
      }

      InstallDependency::~InstallDependency()
      {
      }

      GGS::GameDownloader::HookBase::HookResult InstallDependency::beforeDownload(GameDownloadService *gameDownloader, const GGS::Core::Service *service)
      {
        // nothing to do, really
        return GGS::GameDownloader::HookBase::Continue;
      }

      GGS::GameDownloader::HookBase::HookResult InstallDependency::afterDownload(GameDownloadService *gameDownloader, const GGS::Core::Service *service)
      {
        if (gameDownloader->isInstalled(service))
          return GGS::GameDownloader::HookBase::Continue;

        QString dependencyList = service->externalDependencyList();
        QStringList fileNames = dependencyList.split(',', QString::KeepEmptyParts);

        QProcess process;
        while (!fileNames.isEmpty()) {
          QString dependency = fileNames.takeFirst();
          QString args("");
          if (!fileNames.isEmpty())
            args = fileNames.takeFirst();
          
          QString fullPath = QString("%1/Dependency/%2").arg(service->installPath(), dependency);
          if (!QFile::exists(fullPath)) {
            WARNING_LOG << "Dependency not found. Service " << service->id() << " Dependency: " << dependency;
            return GGS::GameDownloader::HookBase::Continue;
          }
          
          QStringList argumentList = args.split(' ', QString::SkipEmptyParts);
          process.start(fullPath, argumentList);
          process.waitForFinished();
        }

        return GGS::GameDownloader::HookBase::Continue;
      }

    }
  }
}