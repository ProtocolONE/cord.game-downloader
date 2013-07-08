/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Hooks/InstallDependency.h>
#include <GameDownloader/GameDownloadService>

#include <Core/Service.h>

#include <QtCore/QFile>
#include <QtCore/QStringList>
#include <QtCore/QProcess>
#include <QtCore/QDebug>

using GGS::GameDownloader::HookBase;

namespace GGS {
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
        return GGS::GameDownloader::HookBase::Continue;
      }

      HookBase::HookResult InstallDependency::afterDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        if (state->isInstalled() && state->startType() != Recheck)
            return HookBase::Continue;

        DEBUG_LOG << "Start installing dependency";

        const GGS::Core::Service *service = state->service();

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
          process.waitForFinished();
        }

        return HookBase::Continue;
      }

    }
  }
}