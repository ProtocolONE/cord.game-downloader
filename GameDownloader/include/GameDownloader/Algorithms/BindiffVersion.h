#pragma once

#include <GameDownloader/GameDownloader_global.h>

#include <GameDownloader/Behavior/BaseBehavior.h>
#include <GameDownloader/Behavior/BindiffBehavior.h>

#include <GameDownloader/Behavior/TorrentDownloadBehavior.h>
#include <GameDownloader/Behavior/ExtractorBehavior.h>
#include <GameDownloader/Behavior/CompressorBehavior.h>
#include <GameDownloader/Behavior/BindiffBehavior.h>
#include <GameDownloader/Behavior/OnlyCheckUpdateBehavior.h>
#include <GameDownloader/Behavior/CompressorBehavior.h>
#include <GameDownloader/Behavior/RehashClientBehavior.h>
#include <GameDownloader/Behavior/GetNewTorrentBehavior.h>
#include <GameDownloader/Behavior/GetPatchVersionBehavior.h>
#include <GameDownloader/Behavior/BindiffFailedBehavior.h>
#include <GameDownloader/Behavior/DownloadBindiffBehavior.h>
#include <GameDownloader/Behavior/SetAllPackedBehavior.h>
#include <GameDownloader/Behavior/UninstallBehavior.h>
#include <GameDownloader/Behavior/FinishBehavior.h>
#include <GameDownloader/Behavior/CreateFastResumeBehavior.h>
#include <GameDownloader/Behavior/ReadOnlyBehavior.h>

#include <QtCore/QObject>

namespace P1 {
  namespace GameDownloader {
    class GameDownloadService;

    namespace Algorithms {

      class DOWNLOADSERVICE_EXPORT BindiffVersion : public QObject
      {
        Q_OBJECT
      public:
        explicit BindiffVersion(QObject *parent = 0);
        virtual ~BindiffVersion();

        void build(GameDownloadService* gameDownloader);

      private:
        GameDownloadService* _gameDownloader;
        
        Behavior::BindiffBehavior _bindiff1;
        Behavior::OnlyCheckUpdateBehavior _headT1;
        Behavior::CompressorBehavior _compressor1;
        Behavior::CompressorBehavior _compressor2;
        
        Behavior::ExtractorBehavior _extraction;
        Behavior::TorrentDownloadBehavior _torrentDownloadGame;
        Behavior::RehashClientBehavior _rehashClient;
        Behavior::GetNewTorrentBehavior _getNewTorrent;
        Behavior::GetPatchVersionBehavior _getPatchVersion;
        Behavior::BindiffFailedBehavior _bindDiffFailed;

        Behavior::DownloadBindiffBehavior _downloadBindiff;
        Behavior::SetAllPackedBehavior _setAllPacked;

        Behavior::BindiffBehavior _bindiff2;

        Behavior::UninstallBehavior _uninstall;

        Behavior::FinishBehavior _finish;
        Behavior::CreateFastResumeBehavior _createFastResume;
        Behavior::CreateFastResumeBehavior _createFastResume2;
        
        Behavior::ReadOnlyBehavior _readOnlyCheck;

        void registerExtractor(ExtractorBase *extractor);
        void setStartBehavior(Behavior::BaseBehavior *behavior);
        void registerBehavior(Behavior::BaseBehavior *behavior);
        void setProgress(Behavior::BaseBehavior *behavior, int size);

        void setupProgress();

        // Эта функция проверяет на этапе компиляции соответствие аргументов
        template <typename T>
        void setRoute(T *from, typename T::Results result, Behavior::BaseBehavior *to) {
          this->_gameDownloader->_machine.setRoute(from, result, to);
        }


      };

    }
  }
}