/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2015, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Hooks/PreventWin32Download.h>
#include <GameDownloader/GameDownloadService>

#include <Core/Service.h>
#include <Core/UI/Message>

#include <QtCore/QSysInfo>

using GGS::GameDownloader::HookBase;

namespace GGS {
  namespace GameDownloader {
    namespace Hooks {

      PreventWin32Download::PreventWin32Download(QObject *parent)
        : HookBase("1A097DFD-7660-4619-8B1D-8A1FE5440300", parent)
      {
      }

      PreventWin32Download::~PreventWin32Download()
      {
      }

      bool PreventWin32Download::isWindows64bit() const
      {
        static BOOL isWindows64bit = -1;
        if (-1 == isWindows64bit) {
#if _WIN64
          isWindows64bit = TRUE;
#elif _WIN32
          decltype(IsWow64Process)* fnIsWow64Process = (decltype(IsWow64Process)*)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");
          if (fnIsWow64Process)
            fnIsWow64Process(::GetCurrentProcess(), &isWindows64bit);
          else
            isWindows64bit = FALSE;
#endif
        }
        return static_cast<bool>(isWindows64bit);
      }

      HookBase::HookResult PreventWin32Download::beforeDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        if (!this->isWindows64bit()) {
          Core::UI::Message::warning(
            QObject::tr("TITLE_ATTENTION"),
            QObject::tr("PREVENT_WIN32_LAUNCH_WARNING").arg(state->service()->displayName())
            );

          return HookBase::Abort;
        }

        return HookBase::Continue;
      }

      HookBase::HookResult PreventWin32Download::afterDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        return HookBase::Continue;
      }
    }
  }
}