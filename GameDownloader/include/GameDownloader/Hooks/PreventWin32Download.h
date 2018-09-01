#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/HookBase.h>

namespace P1 {
  namespace GameDownloader {
    class GameDownloadService;

    namespace Hooks {
      class DOWNLOADSERVICE_EXPORT PreventWin32Download : public HookBase
      {
        Q_OBJECT

      public:
        explicit PreventWin32Download(QObject *parent = 0);
        virtual ~PreventWin32Download();

        virtual HookResult beforeDownload(GameDownloadService *gameDownloader, ServiceState *state) override;
        virtual HookResult afterDownload(GameDownloadService *gameDownloader, ServiceState *state) override;

      private:
        bool isWindows64bit() const;
      };
    }
  }
}