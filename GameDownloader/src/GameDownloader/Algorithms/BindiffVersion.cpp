#include <GameDownloader/Algorithms/BindiffVersion.h>

#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/StartType.h>

#include <GameDownloader/Extractor/DummyExtractor.h>
#include <GameDownloader/Extractor/GameExtractor.h>

using namespace P1::GameDownloader::Behavior;

namespace P1 {
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

        this->_rehashClient.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_torrentDownloadGame.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_getPatchVersion.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_downloadBindiff.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_uninstall.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_bindiff1.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_bindiff2.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_createFastResume.setTorrentWrapper(this->_gameDownloader->_wrapper);
        this->_createFastResume2.setTorrentWrapper(this->_gameDownloader->_wrapper);

        this->registerExtractor(new Extractor::DummyExtractor(this->_gameDownloader));
        this->registerExtractor(new Extractor::GameExtractor(this->_gameDownloader));

        this->registerBehavior(&this->_readOnlyCheck);
        this->registerBehavior(&this->_uninstall);
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
        this->registerBehavior(&this->_createFastResume);
        this->registerBehavior(&this->_createFastResume2);

        this->setStartBehavior(&this->_gameDownloader->_preHookBehavior);

        this->setRoute(&this->_gameDownloader->_preHookBehavior, PreHookBehavior::Finished, &this->_readOnlyCheck);

        this->setRoute(&this->_readOnlyCheck, ReadOnlyBehavior::Finished, &this->_uninstall);
        this->setRoute(&this->_uninstall, UninstallBehavior::ContinueInstall, &this->_bindiff1);
        this->setRoute(&this->_uninstall, UninstallBehavior::Finished, &this->_gameDownloader->_postHook);

        this->setRoute(&this->_bindiff1, BindiffBehavior::CRCFailed, &this->_bindDiffFailed);
        this->setRoute(&this->_bindiff1, BindiffBehavior::Finished, &this->_compressor1);

        this->setRoute(&this->_compressor1, CompressorBehavior::Finished, &this->_createFastResume);
        this->setRoute(&this->_createFastResume, CreateFastResumeBehavior::Created, &this->_headT1);

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

        this->setRoute(&this->_compressor2, CompressorBehavior::Finished, &this->_createFastResume2);
        this->setRoute(&this->_createFastResume2, CreateFastResumeBehavior::Created, &this->_gameDownloader->_postHook);

        this->setRoute(&this->_gameDownloader->_postHook, PostHookBehavior::Finished, &this->_finish);
        this->setRoute(&this->_gameDownloader->_postHook, PostHookBehavior::ReturnToStart, &this->_gameDownloader->_preHookBehavior);

        this->setRoute(&this->_bindDiffFailed, BindiffFailedBehavior::Finished, &this->_getNewTorrent);

        this->setupProgress();

        QObject::connect(&this->_torrentDownloadGame, &TorrentDownloadBehavior::downloadProgressChanged,
          &this->_gameDownloader->_progressCalculator, &ProgressCalculator::downloadSlot);

        QObject::connect(&this->_downloadBindiff, &DownloadBindiffBehavior::downloadProgressChanged,
          &this->_gameDownloader->_progressCalculator, &ProgressCalculator::downloadSlot);

        QObject::connect(&this->_rehashClient, &RehashClientBehavior::downloadProgressChanged,
          &this->_gameDownloader->_progressCalculator, &ProgressCalculator::downloadSlot);

        QObject::connect(&this->_torrentDownloadGame, &Behavior::TorrentDownloadBehavior::downloadFinished,
          this->_gameDownloader, &GameDownloadService::internalTorrentDownloadFinished, Qt::QueuedConnection);
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

        QObject::connect(behavior, &BaseBehavior::statusMessageChanged,
          this->_gameDownloader, &GameDownloadService::internalStatusMessageChanged, Qt::QueuedConnection);
      }

      void BindiffVersion::setStartBehavior(BaseBehavior *behavior)
      {
        this->_gameDownloader->_machine.setStartBehavior(behavior);
      }

      void BindiffVersion::setProgress(Behavior::BaseBehavior *behavior, int size)
      {
        this->_gameDownloader->_progressCalculator.registerBehavior(behavior);
      }

      void BindiffVersion::setupProgress()
      {
        ProgressCalculator& calc(this->_gameDownloader->_progressCalculator);
        calc.registerBehavior(&this->_gameDownloader->_preHookBehavior);
        calc.registerBehavior(&this->_bindiff1);
        calc.registerBehavior(&this->_compressor1);
        calc.registerBehavior(&this->_rehashClient);

        calc.registerBehavior(&this->_downloadBindiff);
        calc.registerBehavior(&this->_bindiff2);
        calc.registerBehavior(&this->_compressor2);

        calc.registerBehavior(&this->_torrentDownloadGame);
        calc.registerBehavior(&this->_extraction);
        calc.registerBehavior(&this->_gameDownloader->_postHook);

        calc.registerBehavior(&this->_uninstall);

        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_gameDownloader->_preHookBehavior, 1);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_bindiff1, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_compressor1, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_rehashClient, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_downloadBindiff, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_bindiff2, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_compressor2, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_torrentDownloadGame, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_extraction, 10);
        calc.setBehaviorValue(ProgressCalculator::UpdateGame, &this->_gameDownloader->_postHook, 1);

        calc.setBehaviorValue(ProgressCalculator::InstallGameWithArchive, &this->_gameDownloader->_preHookBehavior, 1);
        calc.setBehaviorValue(ProgressCalculator::InstallGameWithArchive, &this->_torrentDownloadGame, 50);
        calc.setBehaviorValue(ProgressCalculator::InstallGameWithArchive, &this->_extraction, 40);
        calc.setBehaviorValue(ProgressCalculator::InstallGameWithArchive, &this->_gameDownloader->_postHook, 1);

        calc.setBehaviorValue(ProgressCalculator::InstallGameWithoutArchive, &this->_gameDownloader->_preHookBehavior, 1);
        calc.setBehaviorValue(ProgressCalculator::InstallGameWithoutArchive, &this->_torrentDownloadGame, 90);
        calc.setBehaviorValue(ProgressCalculator::InstallGameWithoutArchive, &this->_gameDownloader->_postHook, 1);

        calc.setBehaviorValue(ProgressCalculator::UninstallGame, &this->_uninstall, 100);

      }

    }
  }
}
