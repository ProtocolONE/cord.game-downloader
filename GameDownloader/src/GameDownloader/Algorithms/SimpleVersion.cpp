#include <GameDownloader/Algorithms/SimpleVersion.h>

#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/StartType.h>

#include <GameDownloader/Extractor/DummyExtractor.h>
#include <GameDownloader/Extractor/MiniZipExtractor.h>
#include <GameDownloader/Extractor/SevenZipExtractor.h>

using namespace P1::GameDownloader::Behavior;

namespace P1 {
  namespace GameDownloader {
    namespace Algorithms {

      SimpleVersion::SimpleVersion(QObject *parent)
        : QObject(parent)
      {
      }

      SimpleVersion::~SimpleVersion()
      {
      }

      void SimpleVersion::build(GameDownloadService* gameDownloader)
      {
        this->_gameDownloader = gameDownloader;
        this->_downloadBehavior.setTorrentWrapper(this->_gameDownloader->_wrapper);

        this->registerExtractor(new Extractor::DummyExtractor(this->_gameDownloader));
        this->registerExtractor(new Extractor::SevenZipExtractor(this->_gameDownloader));
        this->registerExtractor(new Extractor::MiniZipExtractor(this->_gameDownloader));

        this->registerBehavior(&this->_checkUpdateBehavior);
        this->registerBehavior(&this->_gameDownloader->_preHookBehavior);
        this->registerBehavior(&this->_gameDownloader->_postHook);
        this->registerBehavior(&this->_downloadBehavior);
        this->registerBehavior(&this->_extractionBehavior);
        this->registerBehavior(&this->_finishBehavior);

        this->setProgress(&this->_gameDownloader->_preHookBehavior, 4);
        this->setProgress(&this->_checkUpdateBehavior, 1);
        this->setProgress(&this->_downloadBehavior, 60);
        this->setProgress(&this->_extractionBehavior, 25);
        this->setProgress(&this->_gameDownloader->_postHook, 10);

        SIGNAL_CONNECT_CHECK(QObject::connect(
          &this->_downloadBehavior, 
          SIGNAL(downloadProgressChanged(P1::GameDownloader::ServiceState *, qint8, P1::Libtorrent::EventArgs::ProgressEventArgs)),
          &this->_gameDownloader->_progressCalculator, 
          SLOT(downloadSlot(P1::GameDownloader::ServiceState *, qint8, P1::Libtorrent::EventArgs::ProgressEventArgs))));

        this->setStartBehavior(&this->_checkUpdateBehavior);

        this->setRoute(&this->_checkUpdateBehavior, CheckUpdateBehavior::Finished , &this->_gameDownloader->_preHookBehavior);
        this->setRoute(&this->_gameDownloader->_preHookBehavior, PreHookBehavior::Finished, &this->_downloadBehavior);
        this->setRoute(&this->_downloadBehavior, TorrentDownloadBehavior::Downloaded, &this->_extractionBehavior);
        this->setRoute(&this->_extractionBehavior, ExtractorBehavior::Finished, &this->_gameDownloader->_postHook);
        this->setRoute(&this->_gameDownloader->_postHook, PostHookBehavior::Finished, &this->_finishBehavior);
        this->setRoute(&this->_gameDownloader->_postHook, PostHookBehavior::ReturnToStart, &this->_checkUpdateBehavior);
      }

      void SimpleVersion::registerExtractor(ExtractorBase *extractor)
      {
        this->_extractionBehavior.registerExtractor(extractor);
      }

      void SimpleVersion::setStartBehavior(BaseBehavior *behavior)
      {
        this->_gameDownloader->_machine.setStartBehavior(behavior);
      }

      void SimpleVersion::registerBehavior(BaseBehavior *behavior)
      {
        this->_gameDownloader->_machine.registerBehavior(behavior);
      }

      void SimpleVersion::setProgress(Behavior::BaseBehavior *behavior, int size)
      {
        this->_gameDownloader->_progressCalculator.registerBehavior(behavior);
        this->_gameDownloader->_progressCalculator.setBehaviorValue(ProgressCalculator::UpdateGame, behavior, static_cast<float>(size));
        this->_gameDownloader->_progressCalculator.setBehaviorValue(ProgressCalculator::InstallGameWithArchive, behavior, static_cast<float>(size));
        this->_gameDownloader->_progressCalculator.setBehaviorValue(ProgressCalculator::InstallGameWithoutArchive, behavior, static_cast<float>(size));
      }

    }
  }
}