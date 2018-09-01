#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/HookBase.h>

namespace P1 {
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