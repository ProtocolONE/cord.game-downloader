#ifndef _GGS_GAMEDOWNLOADER_HOOKS_INSTALLDEPENDENCY_H_
#define _GGS_GAMEDOWNLOADER_HOOKS_INSTALLDEPENDENCY_H_

#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/HookBase.h>

namespace GGS {
  namespace GameDownloader {
    class GameDownloadService;

    namespace Hooks {
      class DOWNLOADSERVICE_EXPORT InstallDependency : public HookBase
      {
      public:
        InstallDependency();
        virtual ~InstallDependency();

        virtual HookResult beforeDownload(GameDownloadService *gameDownloader, const GGS::Core::Service *service);
        virtual HookResult afterDownload(GameDownloadService *gameDownloader, const GGS::Core::Service *service);
      };
    }
  }
}
#endif // _GGS_GAMEDOWNLOADER_HOOKS_INSTALLDEPENDENCY_H_