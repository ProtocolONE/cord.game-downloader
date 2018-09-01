#include "FakeLibtorrentWrapperAdapter.h"

#include <functional>

// HACK считай не работает 
FakeLibtorrentWrapperAdapter::FakeLibtorrentWrapperAdapter(P1::GameDownloader::GameDownloadService *service)
  : _downloadService(service)
  , _downloadEventCount(0)
  , _pauseEventCount(0)
  , _downloadTimeInMs(0)
  , _downloadInProgress(0)
{
  this->_downloadEventCount;

  QObject::connect(&this->_timer, SIGNAL(timeout()), this, SLOT(downloadCompletedTimerTick()));
}


FakeLibtorrentWrapperAdapter::~FakeLibtorrentWrapperAdapter()
{
}

void FakeLibtorrentWrapperAdapter::downloadRequestCounter(const P1::Core::Service *service)
{
  QMutexLocker lock(&this->_mutex);
  this->_downloadEventCount++;
  if (this->_downloadInProgress)
    return;

  this->_downloadInProgress = true;
  this->_service = service;
  this->_timer.setSingleShot(true);
  this->_timer.start(this->_downloadTimeInMs);
}

int FakeLibtorrentWrapperAdapter::downloadEventCount() const
{
  return this->_downloadEventCount;
}

void FakeLibtorrentWrapperAdapter::setDownloadTime(int msec)
{
  this->_downloadTimeInMs = msec;
}

void FakeLibtorrentWrapperAdapter::pauseRequestCounter(const P1::Core::Service *service)
{
  //QMutexLocker lock(&this->_mutex);
  //this->_pauseEventCount++;
  //if (!this->_downloadInProgress)
  //  return;

  //this->_downloadInProgress = false;
  //this->_timer.stop();
  //this->_downloadService->pauseRequestCompleted(service);
}

int FakeLibtorrentWrapperAdapter::pauseEventCount() const
{
  return this->_pauseEventCount;
}

void FakeLibtorrentWrapperAdapter::downloadCompletedTimerTick()
{
//  this->_downloadService->downloadRequestCompleted(this->_service);  
}
