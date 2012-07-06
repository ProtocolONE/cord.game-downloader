/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_HOOKS_OLDGAMECLIENTMIGRATE_H_
#define _GGS_GAMEDOWNLOADER_HOOKS_OLDGAMECLIENTMIGRATE_H_

#include <GameDownloader/HookBase.h>

namespace GGS {
  namespace GameDownloader {
    class GameDownloadService;

    namespace Hooks {
      class DOWNLOADSERVICE_EXPORT OldGameClientMigrate : public HookBase
      {
      public:
        OldGameClientMigrate();
        virtual ~OldGameClientMigrate();

        virtual HookResult beforeDownload(GameDownloadService *gameDownloader, const GGS::Core::Service *service);
        virtual HookResult afterDownload(GameDownloadService *gameDownloader, const GGS::Core::Service *service);
      
      private:
        bool isFirstRun(const GGS::Core::Service *service);
        void saveHookFinished(const GGS::Core::Service *service);
        bool migrate(GameDownloadService *gameDownloader, const GGS::Core::Service *service, const QString& sourceDirectory);
        bool readString(const QString& key, const QString& paramName, QString& result);
        bool checkAndGetSourceDirectory(const GGS::Core::Service *service, QString& sourceDirectory);

        quint64 getDriveFreeSpace(const QString& drive);
        quint64 getDirectorySize(const QString& directory);
        QString getDriveName(const QString& path);
      };
    }
  }
}

#endif // _GGS_GAMEDOWNLOADER_HOOKS_OLDGAMECLIENTMIGRATE_H_