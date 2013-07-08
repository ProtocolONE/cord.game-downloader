/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Algorithms/SimpleVersion.h>

#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/StartType.h>

#include <GameDownloader/Extractor/DummyExtractor.h>
#include <GameDownloader/Extractor/SevenZipGameExtractor.h>

using namespace GGS::GameDownloader::Behavior;

namespace GGS {
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
        this->_downloadBehavior.setTorrentWrapper(&this->_gameDownloader->_wrapper);

        this->registerExtractor(new Extractor::DummyExtractor(this->_gameDownloader));
        this->registerExtractor(new Extractor::SevenZipGameExtractor(this->_gameDownloader));

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
          SIGNAL(downloadProgressChanged(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)),
          &this->_gameDownloader->_progressCalculator, 
          SLOT(downloadSlot(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs))));

        this->setStartBehavior(&this->_checkUpdateBehavior);

        this->setRoute(&this->_checkUpdateBehavior, CheckUpdateBehavior::Finished , &this->_gameDownloader->_preHookBehavior);
        this->setRoute(&this->_gameDownloader->_preHookBehavior, PreHookBehavior::Finished, &this->_downloadBehavior);
        this->setRoute(&this->_downloadBehavior, TorrentDownloadBehavior::Downloaded, &this->_extractionBehavior);
        this->setRoute(&this->_extractionBehavior, ExtractorBehavior::Finished, &this->_gameDownloader->_postHook);
        this->setRoute(&this->_gameDownloader->_postHook, PostHookBehavior::Finished, &this->_finishBehavior);
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
        this->_gameDownloader->_progressCalculator.registerBehavior(behavior, static_cast<float>(size));
      }

    }
  }
}