/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Builder.h>

//#include <GameDownloader/CheckUpdateAdapter.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/StartType.h>

#include <GameDownloader/Extractor/DummyExtractor.h>
#include <GameDownloader/Extractor/SevenZipGameExtractor.h>

namespace GGS {
  namespace GameDownloader {

    Builder::Builder(QObject *parent)
      : QObject(parent)
    {
    }

    Builder::~Builder()
    {
    }

    void Builder::build()
    {
      //this->_libtorrentAdapter.setTorrentWrapper(&this->_wrapper);
      this->_gameDownloader.init();
      this->_gameDownloader._downloadBehavior.setTorrentWrapper(&this->_wrapper);

      //CheckUpdateAdapter *checkUpdateAdapter = new CheckUpdateAdapter(&this->_gameDownloader);
      //checkUpdateAdapter->setGameDownloaderService(&this->_gameDownloader);

      //QObject::connect(checkUpdateAdapter, SIGNAL(checkUpdateCompleted(const GGS::Core::Service *, bool)),
      //  &this->_gameDownloader, SLOT(checkUpdateRequestCompleted(const GGS::Core::Service *, bool)), Qt::QueuedConnection);

      //QObject::connect(checkUpdateAdapter, SIGNAL(checkUpdateFailed(const GGS::Core::Service *)), 
      //  &this->_gameDownloader, SLOT(checkUpdateFailed(const GGS::Core::Service *)), Qt::QueuedConnection);

      //QObject::connect(
      //  &this->_gameDownloader, 
      //  SIGNAL(checkUpdateRequest(const GGS::Core::Service *, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType)),
      //  checkUpdateAdapter, 
      //  SLOT(checkUpdateRequest(const GGS::Core::Service *, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType)), 
      //  Qt::QueuedConnection);

      //QObject::connect(&this->_gameDownloader, 
      //  SIGNAL(downloadRequest(const GGS::Core::Service *, GGS::GameDownloader::StartType, bool)),
      //  &this->_libtorrentAdapter, 
      //  SLOT(downloadRequest(const GGS::Core::Service *, GGS::GameDownloader::StartType, bool)));

      //QObject::connect(&this->_gameDownloader, SIGNAL(downloadStopRequest(const GGS::Core::Service *)),
      //  &this->_libtorrentAdapter, SLOT(pauseRequest(const GGS::Core::Service *)));

      //QObject::connect(
      //  &this->_libtorrentAdapter, SIGNAL(torrentPaused(const GGS::Core::Service *)), 
      //  &this->_gameDownloader, SLOT(pauseRequestCompleted(const GGS::Core::Service *)));
      //QObject::connect(
      //  &this->_libtorrentAdapter, SIGNAL(torrentDownloadFinished(const GGS::Core::Service *)), 
      //  &this->_gameDownloader, SLOT(downloadRequestCompleted(const GGS::Core::Service *)));
      //QObject::connect(
      //  &this->_libtorrentAdapter, SIGNAL(torrentDownloadFailed(const GGS::Core::Service *)), 
      //  &this->_gameDownloader, SLOT(downloadFailed(const GGS::Core::Service *)));

      QObject::connect(&this->_wrapper, SIGNAL(progressChanged(GGS::Libtorrent::EventArgs::ProgressEventArgs)), 
        &this->_gameDownloader.progressCalculator(), SLOT(torrentProgress(GGS::Libtorrent::EventArgs::ProgressEventArgs)));

      //QObject::connect(checkUpdateAdapter, SIGNAL(checkUpdateProgressChanged(const QString&, quint8)), 
      //  &this->_gameDownloader.progressCalculator(), SLOT(checkUpdateProgress(const QString&, quint8)));

      Extractor::DummyExtractor *dummyExtractor = new Extractor::DummyExtractor(&this->_gameDownloader);
      this->_gameDownloader.registerExtractor(dummyExtractor);      
      Extractor::SevenZipGameExtractor *sevenZipExtractor = new Extractor::SevenZipGameExtractor(&this->_gameDownloader);
      this->_gameDownloader.registerExtractor(sevenZipExtractor);
    }

    GGS::Libtorrent::Wrapper& Builder::torrentWrapper()
    {
      return this->_wrapper;
    }

    GameDownloadService& Builder::gameDownloader()
    {
      return this->_gameDownloader;
    }

  }
}