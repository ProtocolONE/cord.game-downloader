/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2015 - 2016, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/HookBase.h>

namespace GGS {
  namespace GameDownloader {
    class GameDownloadService;

    namespace Hooks {

      class DOWNLOADSERVICE_EXPORT RemoveFileHook : public HookBase
      {
        Q_OBJECT
      public:
        explicit RemoveFileHook(QObject *parent = 0);
        virtual ~RemoveFileHook();

        virtual HookResult beforeDownload(GameDownloadService *gameDownloader, ServiceState *state) override;
        virtual HookResult afterDownload(GameDownloadService *gameDownloader, ServiceState *state) override;

      private:
        bool shouldDelete(const QString& id, const QString& filePath);
        void saveDataInfo(const QString& id, const QString& filePath);
      };

    }
  }
}