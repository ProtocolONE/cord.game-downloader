/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Algorithms/BindiffVersion.h>

#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/StartType.h>

#include <GameDownloader/Extractor/DummyExtractor.h>
#include <GameDownloader/Extractor/SevenZipGameExtractor.h>

using namespace GGS::GameDownloader::Behavior;

namespace GGS {
  namespace GameDownloader {
    namespace Algorithms {

      BindiffVersion::BindiffVersion(QObject *parent)
        : QObject(parent)
      {
      }

      BindiffVersion::~BindiffVersion()
      {
      }

      // Пока собираю синхронную запаковку
      void BindiffVersion::build(GameDownloadService* gameDownloader)
      {
        this->_gameDownloader = gameDownloader;

        this->_rehashClient.setTorrentWrapper(&this->_gameDownloader->_wrapper);
        this->_torrentDownloadGame.setTorrentWrapper(&this->_gameDownloader->_wrapper);
        this->_getPatchVersion.setTorrentWrapper(&this->_gameDownloader->_wrapper);
        this->_downloadBindiff.setTorrentWrapper(&this->_gameDownloader->_wrapper);

        this->registerExtractor(new Extractor::DummyExtractor(this->_gameDownloader));
        this->registerExtractor(new Extractor::SevenZipGameExtractor(this->_gameDownloader));

        this->registerBehavior(&this->_gameDownloader->_preHookBehavior);
        this->registerBehavior(&this->_gameDownloader->_postHook);
        this->registerBehavior(&this->_bindiff1);
        this->registerBehavior(&this->_headT1);
        this->registerBehavior(&this->_compressor1);
        this->registerBehavior(&this->_compressor2);
        this->registerBehavior(&this->_torrentDownloadGame);
        this->registerBehavior(&this->_finish);
        this->registerBehavior(&this->_extraction);
        this->registerBehavior(&this->_getNewTorrent);
        this->registerBehavior(&this->_getPatchVersion);
        this->registerBehavior(&this->_bindDiffFailed);
        this->registerBehavior(&this->_downloadBindiff);
        this->registerBehavior(&this->_bindiff2);
        this->registerBehavior(&this->_rehashClient);
        this->registerBehavior(&this->_setAllPacked);

        this->setStartBehavior(&this->_gameDownloader->_preHookBehavior);

        this->setRoute(&this->_gameDownloader->_preHookBehavior, PreHookBehavior::Finished, &this->_bindiff1);

        this->setRoute(&this->_bindiff1, BindiffBehavior::CRCFailed, &this->_bindDiffFailed);
        this->setRoute(&this->_bindiff1, BindiffBehavior::Finished, &this->_compressor1);

        this->setRoute(&this->_compressor1, CompressorBehavior::Finished, &this->_headT1);

        // Обновлений не найден, проходимя по торренту(начинаем сидировать), 
        // распаковываем недораспакованной и на финиш
        this->setRoute(&this->_headT1, OnlyCheckUpdateBehavior::UpdateNotFound, &this->_torrentDownloadGame);
        this->setRoute(&this->_torrentDownloadGame, TorrentDownloadBehavior::Downloaded, &this->_extraction);
        this->setRoute(&this->_extraction, ExtractorBehavior::Finished, &this->_gameDownloader->_postHook);

        // OnlyCheckUpdateBehavior::UpdateFound
        this->setRoute(&this->_headT1, OnlyCheckUpdateBehavior::UpdateFound, &this->_getPatchVersion);

        this->setRoute(&this->_getPatchVersion, GetPatchVersionBehavior::PatchNotFound, &this->_getNewTorrent);
        this->setRoute(&this->_getPatchVersion, GetPatchVersionBehavior::PatchFound, &this->_rehashClient);

        this->setRoute(&this->_rehashClient, RehashClientBehavior::GameBroken, &this->_getNewTorrent);
        this->setRoute(&this->_rehashClient, RehashClientBehavior::GameFine, &this->_downloadBindiff);
        
        this->setRoute(&this->_getNewTorrent, GetNewTorrentBehavior::Downloaded, &this->_torrentDownloadGame);
        
        this->setRoute(&this->_downloadBindiff, DownloadBindiffBehavior::Downloaded, &this->_setAllPacked);
        this->setRoute(&this->_setAllPacked, SetAllPackedBehavior::Finished, &this->_bindiff2);

        this->setRoute(&this->_bindiff2, BindiffBehavior::CRCFailed, &this->_bindDiffFailed);
        this->setRoute(&this->_bindiff2, BindiffBehavior::Finished, &this->_compressor2);

        this->setRoute(&this->_compressor2, CompressorBehavior::Finished, &this->_gameDownloader->_postHook);

        this->setRoute(&this->_gameDownloader->_postHook, PostHookBehavior::Finished, &this->_finish);

        this->setRoute(&this->_bindDiffFailed, BindiffFailedBehavior::Finished, &this->_getNewTorrent);

        // Setup progress
        this->setProgress(&this->_gameDownloader->_preHookBehavior, 1);
        this->setProgress(&this->_bindiff1, 10);
        this->setProgress(&this->_compressor1, 10);
        this->setProgress(&this->_rehashClient, 10);

        this->setProgress(&this->_downloadBindiff, 10);
        this->setProgress(&this->_bindiff2, 10);
        this->setProgress(&this->_compressor2, 10);

        this->setProgress(&this->_torrentDownloadGame, 10);
        this->setProgress(&this->_extraction, 10);
        this->setProgress(&this->_gameDownloader->_postHook, 1);

        SIGNAL_CONNECT_CHECK(QObject::connect(
          &this->_torrentDownloadGame, 
          SIGNAL(downloadProgressChanged(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)),
          &this->_gameDownloader->_progressCalculator, 
          SLOT(downloadSlot(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs))));
        
        SIGNAL_CONNECT_CHECK(QObject::connect(
          &this->_downloadBindiff, 
          SIGNAL(downloadProgressChanged(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)),
          &this->_gameDownloader->_progressCalculator, 
          SLOT(downloadSlot(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs))));

        SIGNAL_CONNECT_CHECK(QObject::connect(
          &this->_rehashClient, 
          SIGNAL(downloadProgressChanged(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)),
          &this->_gameDownloader->_progressCalculator, 
          SLOT(downloadSlot(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs))));
      }

      void BindiffVersion::registerExtractor(ExtractorBase *extractor)
      {
        this->_extraction.registerExtractor(extractor);

        this->_compressor1.registerCompressor(extractor);
        this->_compressor2.registerCompressor(extractor);

        this->_setAllPacked.registerExtractor(extractor);
      }

      void BindiffVersion::registerBehavior(BaseBehavior *behavior)
      {
        this->_gameDownloader->_machine.registerBehavior(behavior);

        SIGNAL_CONNECT_CHECK(QObject::connect(
          behavior, 
          SIGNAL(statusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&)), 
          this->_gameDownloader, 
          SLOT(internalStatusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&)), 
          Qt::QueuedConnection));
      }

      void BindiffVersion::setStartBehavior(BaseBehavior *behavior)
      {
        this->_gameDownloader->_machine.setStartBehavior(behavior);
      }

      void BindiffVersion::setProgress(Behavior::BaseBehavior *behavior, int size)
      {
        this->_gameDownloader->_progressCalculator.registerBehavior(behavior, static_cast<float>(size));
      }

    }
  }
}