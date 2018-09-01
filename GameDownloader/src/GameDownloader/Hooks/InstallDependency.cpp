#include <GameDownloader/Hooks/InstallDependency.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QDebug>

using P1::GameDownloader::HookBase;

namespace P1 {
  namespace GameDownloader {
    namespace Hooks {

      InstallDependency::InstallDependency(QObject *parent)
         : HookBase("B4910801-2FA4-455E-AEAE-A2BAA2D3E4CA", parent)
      {
      }

      InstallDependency::~InstallDependency()
      {
      }

      HookBase::HookResult InstallDependency::beforeDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        // nothing to do, really
        return P1::GameDownloader::HookBase::Continue;
      }

      HookBase::HookResult InstallDependency::afterDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        if (state->isInstalled() && state->startType() != Recheck)
            return HookBase::Continue;

        DEBUG_LOG << "Start installing dependency";

        const P1::Core::Service *service = state->service();

        QString dependencyList = service->externalDependencyList();
        QStringList fileNames = dependencyList.split(',', QString::KeepEmptyParts);

        QProcess process;
        while (!fileNames.isEmpty()) {
          QString dependency = fileNames.takeFirst();
          QString args("");
          if (!fileNames.isEmpty())
            args = fileNames.takeFirst();
          
          QString fullPath = QString("%1/%2/Dependency/%3").arg(service->installPath(), service->areaString(), dependency);
          DEBUG_LOG << "install" << fullPath;

          if (!QFile::exists(fullPath)) {
            WARNING_LOG << "Dependency not found. Service " << service->id() << " Dependency: " << dependency;
            return HookBase::Continue;
          }
          
          QStringList argumentList = args.split(' ', QString::SkipEmptyParts);
          process.start(fullPath, argumentList);
          process.waitForFinished(-1);
        }

        return HookBase::Continue;
      }

    }
  }
}