#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/HookBase.h>

namespace P1 {
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