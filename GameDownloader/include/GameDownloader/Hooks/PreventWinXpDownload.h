/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2015, Syncopate Limited and/or affiliates.
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
			class DOWNLOADSERVICE_EXPORT PreventWinXpDownload : public HookBase
			{
				Q_OBJECT

			public:
				explicit PreventWinXpDownload(QObject *parent = 0);
				virtual ~PreventWinXpDownload();

				virtual HookResult beforeDownload(GameDownloadService *gameDownloader, ServiceState *state) override;
				virtual HookResult afterDownload(GameDownloadService *gameDownloader, ServiceState *state) override;
			};
		}
	}
}