/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2015, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Hooks/PreventWinXpDownload.h>
#include <GameDownloader/GameDownloadService>

#include <Core/Service.h>
#include <Core/UI/Message>

#include <QtCore/QSysInfo>

using GGS::GameDownloader::HookBase;

namespace GGS {
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