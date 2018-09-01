#include <GameDownloader/Hooks/PreventWinXpDownload.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>
#include <Core/UI/Message.h>

#include <QtCore/QSysInfo>

using P1::GameDownloader::HookBase;

namespace P1 {
  namespace GameDownloader {
    namespace Hooks {

      PreventWinXpDownload::PreventWinXpDownload(QObject *parent)
        : HookBase("F9FD8276-2FEA-4F99-A2AA-1B37627216F5", parent)
      {
      }

      PreventWinXpDownload::~PreventWinXpDownload()
      {
      }

      HookBase::HookResult PreventWinXpDownload::beforeDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        if (QSysInfo::WV_XP == QSysInfo::WindowsVersion) {
          Core::UI::Message::warning(
            QObject::tr("TITLE_ATTENTION"),
            QObject::tr("PREVENT_XP_LAUNCH_WARNING").arg(state->service()->displayName())
          );          

          return HookBase::Abort;
        }

        return HookBase::Continue;
      }

      HookBase::HookResult PreventWinXpDownload::afterDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        return HookBase::Continue;
      }
    }
  }
}