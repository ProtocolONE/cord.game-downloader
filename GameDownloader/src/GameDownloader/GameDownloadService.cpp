#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/ExtractorBase.h>

#include <Settings/Settings.h>

#include <Core/Service.h>

#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <QtConcurrent/QtConcurrentRun>
#include <QEventLoop>

namespace P1 {
  namespace GameDownloader {

    P1::Libtorrent::Wrapper::Profile fromGameDownloadProfile(GameDownloadService::TorrentProfile profile)
    {
      using P1::Libtorrent::Wrapper;

      if (profile == GameDownloadService::TorrentProfile::HIGH_PERFORMANCE_SEED) {
        return Wrapper::Profile::HIGH_PERFORMANCE_SEED;
      }
      else if (profile == GameDownloadService::TorrentProfile::MIN_MEMORY_USAGE) {
        return Wrapper::Profile::MIN_MEMORY_USAGE;
      }

      return Wrapper::Profile::DEFAULT_PROFILE;
    }

    GameDownloadService::GameDownloadService(QObject *parent)
      : QObject(parent)
      , _isShuttingDown(false)
      , _isShutdownFinished(false)
      , _wrapper(new P1::Libtorrent::Wrapper)
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

      SIGNAL_CONNECT_CHECK(QObject::connect(hook, SIGNAL(statusMessageChanged(P1::GameDownloader::ServiceState *, const QString&)), 
        this, SLOT(internalStatusMessageChanged(P1::GameDownloader::ServiceState *, const QString&))));
    }

    ServiceState* GameDownloadService::getStateById(const QString& id)
    { 
      if (!this->_stateMap.contains(id)) 
        return 0;

      return this->_stateMap[id];
    }

    void GameDownloadService::start(const P1::Core::Service *service, StartType startType)
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

      if (state->isDirectoryChanged()) {
        state->setIsDirectoryChanged(false);
        if (startType != GameDownloader::Force)
          startType = GameDownloader::Recheck;
      }

      state->setStartType(startType);

      this->_progressCalculator.resetProgress(state);

      if (this->_machine.start(state))
        emit this->started(service, startType);
    }

    void GameDownloadService::stop(const P1::Core::Service *service)
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

      this->_preHookBehavior.setGameDownloaderService(this);
      this->_postHook.setGameDownloaderService(this);

      SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(listeningPortChanged(unsigned short)),
        this, SIGNAL(listeningPortChanged(unsigned short))));

      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_machine, SIGNAL(finished(P1::GameDownloader::ServiceState *)),
        this, SLOT(internalFinished(P1::GameDownloader::ServiceState *))));
      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_machine, SIGNAL(failed(P1::GameDownloader::ServiceState *)),
        this, SLOT(internalFailed(P1::GameDownloader::ServiceState *))));
      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_machine, SIGNAL(stopped(P1::GameDownloader::ServiceState *)),
        this, SLOT(internalStopped(P1::GameDownloader::ServiceState *))));

      SIGNAL_CONNECT_CHECK(QObject::connect(&this->_progressCalculator, 
        SIGNAL(totalProgressChanged(P1::GameDownloader::ServiceState *, qint8)), 
        this, 
        SLOT(internalTotalProgressChanged(P1::GameDownloader::ServiceState *, qint8))));

      SIGNAL_CONNECT_CHECK(QObject::connect(
        &this->_progressCalculator, 
        SIGNAL(downloadProgressChanged(P1::GameDownloader::ServiceState *, qint8, P1::Libtorrent::EventArgs::ProgressEventArgs)), 
        this, 
        SLOT(internalDownloadProgressChanged(P1::GameDownloader::ServiceState *, qint8, P1::Libtorrent::EventArgs::ProgressEventArgs))));

      SIGNAL_CONNECT_CHECK(QObject::connect(
        &this->_preHookBehavior, 
        SIGNAL(statusMessageChanged(P1::GameDownloader::ServiceState *, const QString&)), 
        this, 
        SLOT(internalStatusMessageChanged(P1::GameDownloader::ServiceState *, const QString&))));

      SIGNAL_CONNECT_CHECK(QObject::connect(
        &this->_postHook, 
        SIGNAL(statusMessageChanged(P1::GameDownloader::ServiceState *, const QString&)), 
        this, 
        SLOT(internalStatusMessageChanged(P1::GameDownloader::ServiceState *, const QString&))));
    }

    // INFO 01.09.2014 Функция не вызывается нигде - надо удлаить в том числе с сигналом progressChanged
    void GameDownloadService::internalProgressChangedSlot(QString serviceId, qint8 progress)
    {
      if (!this->_stateLock.tryLock()) 
        return;

      ServiceState *state = this->getStateById(serviceId);
      if (state && state->state() == ServiceState::Started)
        emit this->progressChanged(serviceId, progress);

      this->_stateLock.unlock();
    }

    // INFO 01.09.2014 Функция не вызывается нигде - надо удлаить в том числе с сигналом progressDownloadChanged
    void GameDownloadService::internalProgressDownloadChangedSlot(QString serviceId, qint8 progress, P1::Libtorrent::EventArgs::ProgressEventArgs args)
    {
      if (!this->_stateLock.tryLock()) 
        return;

      ServiceState *state = this->getStateById(serviceId);
      if (state && state->state() == ServiceState::Started)
        emit this->progressDownloadChanged(serviceId, progress, args);

      this->_stateLock.unlock();
    }

    // INFO 01.09.2014 удалить - нигде не вызывается
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

    void GameDownloadService::directoryChanged(const P1::Core::Service *service)
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

    bool GameDownloadService::isInstalled(const P1::Core::Service *service)
    {
      Q_ASSERT(service);
      return this->isInstalled(service->id());
    }

    bool GameDownloadService::isInProgress(const P1::Core::Service *service)
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

    void GameDownloadService::internalFinished(P1::GameDownloader::ServiceState *state)
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown) {
        if (this->isAllStopped())
          this->internalShuttingDownComplete();

        return;
      }

      const P1::Core::Service *service = state->service();
      emit this->totalProgressChanged(service, 100);

      if (state->startType() == Uninstall) {
        state->setIsInstalled(false);
        emit this->serviceUninstalled(service);
      } else {
        if (!this->isInstalled(service)) {
          state->setIsInstalled(true);
          emit this->serviceInstalled(service);
        } else { 
          emit this->serviceUpdated(service); 
        }
      }

      emit this->finished(service);
    }

    void GameDownloadService::internalStopped(P1::GameDownloader::ServiceState *state)
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

    void GameDownloadService::internalFailed(P1::GameDownloader::ServiceState *state)
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
    
    void GameDownloadService::setTorrentProfile(TorrentProfile value)
    {
      this->_wrapper->setProfile(fromGameDownloadProfile(value));
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

    void GameDownloadService::setCredentials(const QString &userId, const QString &hash)
    {
      this->_wrapper->setCredentials(userId, hash);
    }

    void GameDownloadService::resetCredentials()
    {
      this->_wrapper->resetCredentials();
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

    void GameDownloadService::internalStatusMessageChanged(P1::GameDownloader::ServiceState *state, const QString& message)
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

    void GameDownloadService::internalTotalProgressChanged(P1::GameDownloader::ServiceState *state, qint8 progress)
    {
      emit this->totalProgressChanged(state->service(), progress);
    }

    void GameDownloadService::internalDownloadProgressChanged(
      P1::GameDownloader::ServiceState *state, 
      qint8 progress, 
      P1::Libtorrent::EventArgs::ProgressEventArgs args)
    {
      emit this->downloadProgressChanged(state->service(), progress, args);
    }

    void GameDownloadService::internalTorrentDownloadFinished(P1::GameDownloader::ServiceState *state)
    {
      QMutexLocker lock(&this->_stateLock);
      if (!state)
        return;

      emit this->finishedDownloading(state->service());
    }
  }
}
