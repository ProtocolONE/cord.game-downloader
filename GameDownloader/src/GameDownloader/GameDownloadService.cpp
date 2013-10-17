/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/ExtractorBase.h>

#include <Settings/Settings>

#include <Core/Service>
#include <Core/Marketing.h>

#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <QtCore/QtConcurrentRun>
#include <QEventLoop>

using GGS::Core::Marketing;

namespace GGS {
  namespace GameDownloader {
    GameDownloadService::GameDownloadService(QObject *parent)
      : QObject(parent)
      , _isShuttingDown(false)
      , _isShutdownFinished(false)
      , _wrapper(new GGS::Libtorrent::Wrapper)
    {
    }

    GameDownloadService::~GameDownloadService()
    {
      if (this->_isShuttingDown) {
        return;
      }

      this->shutdown();
    }

    void GameDownloadService::registerHook(const QString& serviceId, int preHookPriority, int postHookPriority, HookBase *hook)
    {
      this->_preHookBehavior.registerHook(serviceId, preHookPriority, hook);
      this->_postHook.registerHook(serviceId, postHookPriority, hook);

      if (_registredHooks.contains(hook))
        return;

      this->_registredHooks.insert(hook);

      SIGNAL_CONNECT_CHECK(QObject::connect(hook, SIGNAL(statusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&)), 
        this, SLOT(internalStatusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&))));
    }

    ServiceState* GameDownloadService::getStateById(const QString& id)
    { 
      if (!this->_stateMap.contains(id)) 
        return 0;

      return this->_stateMap[id];
    }

    void GameDownloadService::start(const GGS::Core::Service *service, StartType startType)
    {
      DEBUG_LOG << service->id();

      Q_CHECK_PTR(service);

      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return;

      ServiceState *state = this->getStateById(service->id());

      if (!state) {
        state = new ServiceState(this);
        state->setService(service);
        state->setState(ServiceState::Stopped);
        this->_stateMap[state->id()] = state;
      }

      state->setStartType(startType);

      if (state->isDirectoryChanged()) {
        state->setIsDirectoryChanged(false);
        if (startType != GameDownloader::Force)
          startType = GameDownloader::Recheck;
      }

      if (this->_machine.start(state)) {
        Marketing::send(Marketing::StartDownloadService, service->id());
        emit this->started(service, startType);
      }
    }

    void GameDownloadService::stop(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return;

      ServiceState *state = this->getStateById(service->id());
      if (!state) {
        CRITICAL_LOG << "can't stop unknown service " << service->id();
        return;
      }

      if (this->_machine.stop(state))
        emit this->stopping(service);
    }

    void GameDownloadService::init()
    {
      this->_wrapper->initEngine();
      this->_downloadAlgorithm.build(this);

      SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(listeningPortChanged(unsigned short)),
        this, SIGNAL(listeningPortChanged(unsigned short))));

      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_machine, SIGNAL(finished(GGS::GameDownloader::ServiceState *)),
        this, SLOT(internalFinished(GGS::GameDownloader::ServiceState *))));
      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_machine, SIGNAL(failed(GGS::GameDownloader::ServiceState *)),
        this, SLOT(internalFailed(GGS::GameDownloader::ServiceState *))));
      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_machine, SIGNAL(stopped(GGS::GameDownloader::ServiceState *)),
        this, SLOT(internalStopped(GGS::GameDownloader::ServiceState *))));

      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_progressCalculator, 
        SIGNAL(totalProgressChanged(GGS::GameDownloader::ServiceState *, qint8)), 
        this, 
        SLOT(internalTotalProgressChanged(GGS::GameDownloader::ServiceState *, qint8))));

      SIGNAL_CONNECT_CHECK(QObject::connect(
        &this->_progressCalculator, 
        SIGNAL(downloadProgressChanged(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)), 
        this, 
        SLOT(internalDownloadProgressChanged(GGS::GameDownloader::ServiceState *, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs))));

      SIGNAL_CONNECT_CHECK(QObject::connect(
        &this->_preHookBehavior, 
        SIGNAL(statusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&)), 
        this, 
        SLOT(internalStatusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&))));

      SIGNAL_CONNECT_CHECK(QObject::connect(
        &this->_postHook, 
        SIGNAL(statusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&)), 
        this, 
        SLOT(internalStatusMessageChanged(GGS::GameDownloader::ServiceState *, const QString&))));

    }

    void GameDownloadService::internalProgressChangedSlot(QString serviceId, qint8 progress)
    {
      if (!this->_stateLock.tryLock()) 
        return;

      ServiceState *state = this->getStateById(serviceId);
      if (state && state->state() == ServiceState::Started)
        emit this->progressChanged(serviceId, progress);

      this->_stateLock.unlock();
    }

    void GameDownloadService::internalProgressDownloadChangedSlot(QString serviceId, qint8 progress, GGS::Libtorrent::EventArgs::ProgressEventArgs args)
    {
      if (!this->_stateLock.tryLock()) 
        return;

      ServiceState *state = this->getStateById(serviceId);
      if (state && state->state() == ServiceState::Started)
        emit this->progressDownloadChanged(serviceId, progress, args);

      this->_stateLock.unlock();
    }

    void GameDownloadService::internalProgressExtractionChangedSlot(QString serviceId, qint8 progress, qint64 current, qint64 total)
    {
      if (!this->_stateLock.tryLock()) 
        return;

      ServiceState *state = this->getStateById(serviceId);
      if (state && state->state() == ServiceState::Started)
        emit this->progressExtractionChanged(serviceId, progress, current, total);

      this->_stateLock.unlock();
    }

    void GameDownloadService::setTimeoutForResume(quint32 seconds)
    {
      this->_machine.setTimeForResume(seconds * 1000);
    }

    void GameDownloadService::shutdown()
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown) {
        CRITICAL_LOG << "GameDownloadService : double shutdown request";
        return;
      }

      this->_isShuttingDown = true;

      bool shouldWait = false;

      Q_FOREACH(ServiceState *state, this->_stateMap) {
        if (state->state() == ServiceState::Stopped)
          continue;

        shouldWait = true;
        this->_machine.stop(state);
      }

      if (shouldWait)
        return;

      this->internalShuttingDownComplete();
    }

    void GameDownloadService::directoryChanged(const GGS::Core::Service *service)
    {
      Q_CHECK_PTR(service);

      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return;

      ServiceState *state = this->getStateById(service->id());
      if (!state)
        return;

      state->setIsDirectoryChanged(true);

      if (this->_machine.stop(state)) 
        emit this->stopping(service);
    }

    bool GameDownloadService::isInstalled(const QString& serviceId)
    {
      Q_ASSERT(!serviceId.isEmpty());
      return ServiceState::isInstalled(serviceId);
    }

    bool GameDownloadService::isInstalled(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      return this->isInstalled(service->id());
    }

    bool GameDownloadService::isInProgress(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return false;

      ServiceState *state = this->getStateById(service->id());
      if (!state)
        return false;

      return state->state() != ServiceState::Stopped;
    }

    bool GameDownloadService::isAnyServiceInProgress()
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return false;

      bool result = false;
      Q_FOREACH(ServiceState *state, this->_stateMap)
        result |= (state->state() != ServiceState::Stopped);

      return result;
    }

    void GameDownloadService::internalFinished(GGS::GameDownloader::ServiceState *state)
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown) {
        if (this->isAllStopped())
          this->internalShuttingDownComplete();

        return;
      }

      const GGS::Core::Service *service = state->service();
      emit this->totalProgressChanged(service, 100);

      if (!this->isInstalled(service)) {
        state->setIsInstalled(true);
        emit this->serviceInstalled(service);
      } else { 
        emit this->serviceUpdated(service); 
      }

      Marketing::sendOnceByService(Marketing::FinishInstallService, service->id());
      emit this->finished(service);
    }

    void GameDownloadService::internalStopped(GGS::GameDownloader::ServiceState *state)
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown) {
        if (this->isAllStopped())
          this->internalShuttingDownComplete();

        return;
      }

      emit this->stopped(state->service());
    }

    int GameDownloadService::getDiskFreeSpaceInMb(LPCWSTR drive)
    {
      ULARGE_INTEGER freeBytes;
      freeBytes.QuadPart = 0L;

      if (!GetDiskFreeSpaceEx(drive, &freeBytes, NULL, NULL))
        return 0;

      return freeBytes.QuadPart / 1048576;
    }

    void GameDownloadService::internalFailed(GGS::GameDownloader::ServiceState *state)
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown) {
        if (this->isAllStopped())
          this->internalShuttingDownComplete();

        return;
      }

      QString installDisk = state->service()->installPath().left(2);
      QString downloadDisk = state->service()->downloadPath().left(2);

      if (this->getDiskFreeSpaceInMb(installDisk.toStdWString().c_str()) < 5) {
        emit this->statusMessageChanged(state->service(), QObject::tr("NOT_ENOUGH_SPACE_ON_THE_DISK %1").arg(installDisk));
      }

      if (this->getDiskFreeSpaceInMb(downloadDisk.toStdWString().c_str()) < 5) {
        emit this->statusMessageChanged(state->service(), QObject::tr("NOT_ENOUGH_SPACE_ON_THE_DISK %1").arg(downloadDisk));
      }

      emit this->failed(state->service());
    }

    void GameDownloadService::setListeningPort(unsigned short port)
    {
      this->_wrapper->setListeningPort(port);
    }

    void GameDownloadService::setTorrentConfigDirectoryPath(const QString& path)
    {
      this->_wrapper->setTorrentConfigDirectoryPath(path);
    }

    void GameDownloadService::changeListeningPort(unsigned short port)
    {
      this->_wrapper->changeListeningPort(port);
    }

    void GameDownloadService::setUploadRateLimit(int bytesPerSecond)
    {
      this->_wrapper->setUploadRateLimit(bytesPerSecond);
    }

    void GameDownloadService::setDownloadRateLimit(int bytesPerSecond)
    {
      this->_wrapper->setDownloadRateLimit(bytesPerSecond);
    }

    void GameDownloadService::setMaxConnection(int maxConnection)
    {
      this->_wrapper->setMaxConnection(maxConnection);
    }

    void GameDownloadService::setSeedEnabled(bool value)
    {
      this->_wrapper->setSeedEnabled(value);
    }

    void GameDownloadService::internalShuttingDownComplete()
    {
      if (this->_isShutdownFinished)
        return;

      this->_isShutdownFinished = true;
      emit this->shutdownCompleted();
    }

    void GameDownloadService::release()
    {
      this->_wrapper->shutdown();
      delete _wrapper;
    }

    bool GameDownloadService::isAllStopped()
    {
      bool result = true;

      Q_FOREACH(ServiceState *state, this->_stateMap) {
        if (state->state() == ServiceState::Stopped)
          continue;

        result = false;
      }

      return result;
    }

    void GameDownloadService::internalStatusMessageChanged(GGS::GameDownloader::ServiceState *state, const QString& message)
    {
      if (state->state() != ServiceState::Started)
        return;

      // UNDONE подумать стоит ли тут проверить что все заепца
      emit this->statusMessageChanged(state->service(), message);
    }

    void GameDownloadService::pauseSession()
    {
      this->_wrapper->pauseSession();
    }

    void GameDownloadService::resumeSession()
    {
      this->_wrapper->resumeSession();
    }

    void GameDownloadService::internalTotalProgressChanged(GGS::GameDownloader::ServiceState *state, qint8 progress)
    {
      emit this->totalProgressChanged(state->service(), progress);
    }

    void GameDownloadService::internalDownloadProgressChanged(
      GGS::GameDownloader::ServiceState *state, 
      qint8 progress, 
      GGS::Libtorrent::EventArgs::ProgressEventArgs args)
    {
      emit this->downloadProgressChanged(state->service(), progress, args);
    }

  }
}
