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
#include <GameDownloader/PauseRequestWatcher.h>

#include <Settings/Settings>

#include <Core/Service>
#include <Core/Marketing.h>

#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <QtCore/QtConcurrentRun>

using GGS::Core::Marketing;

namespace GGS {
  namespace GameDownloader {
    GameDownloadService::GameDownloadService(QObject *parent)
      : QObject(parent)
      , _timeForResume(600000)
      , _isShuttingDown(false)
    {
    }

    GameDownloadService::~GameDownloadService()
    {
    }

    void GameDownloadService::registerHook(const QString& serviceId, int preHookPriority, int postHookPriority, HookBase *hook)
    {
      this->_beforeDownloadHookMap[serviceId].insert(-preHookPriority, hook);
      this->_afterDownloadHookMap[serviceId].insert(-postHookPriority, hook);

      this->_progressCalculator.registerHook(serviceId, preHookPriority, postHookPriority, hook);

      if (_registredHooks.contains(hook))
        return;

      this->_registredHooks.insert(hook);

      SIGNAL_CONNECT_CHECK(QObject::connect(hook, SIGNAL(beforeProgressChanged(const QString&, const QString&, quint8)),
        &this->_progressCalculator, SLOT(preHookProgress(const QString&, const QString&, quint8))));

      SIGNAL_CONNECT_CHECK(QObject::connect(hook, SIGNAL(afterProgressChanged(const QString&, const QString&, quint8)),
        &this->_progressCalculator, SLOT(postHookProgress(const QString&, const QString&, quint8))));

      SIGNAL_CONNECT_CHECK(QObject::connect(this, SIGNAL(stopping(const GGS::Core::Service*)), 
        hook, SLOT(pauseRequestSlot(const GGS::Core::Service*)), Qt::DirectConnection));

      SIGNAL_CONNECT_CHECK(QObject::connect(hook, SIGNAL(statusMessageChanged(const GGS::Core::Service*, const QString&)), 
        this, SIGNAL(statusMessageChanged(const GGS::Core::Service*, const QString&))));
    }

    void GameDownloadService::start(const GGS::Core::Service *service, StartType startType)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return;

      ServiceState *state = this->getStateById(service->id());

      if (state) {
        if (state->state() == ServiceState::Started || state->state() == ServiceState::Stopping) {
          return;
        }

      } else {
        state = new ServiceState(this);
        state->setService(service);
        this->_stateMap[state->id()] = state;
      }

      emit this->started(service, startType);
      Marketing::send(Marketing::StartDownloadService, service->id());

      if (state->isDirectoryChanged()) {
        state->setIsDirectoryChanged(false);
        if (startType != GameDownloader::Force)
          startType = GameDownloader::Recheck;
      }

      state->setStartType(startType);

      if (startType == GameDownloader::Force || startType == GameDownloader::Recheck) {
        state->setState(ServiceState::Started);
        this->startCheckUpdate(service, state);
        return;
      }

      // UNDONE: Реализовать shadow режим (надо сначала обсудить, может его стоит делать не через даунллоад менеджер)
      qint64 elapsedTime = QDateTime::currentMSecsSinceEpoch() - state->lastDateStateChanged();
      state->setState(ServiceState::Started);
      if (elapsedTime < 0 || elapsedTime > this->_timeForResume) {
        this->startCheckUpdate(service, state);
        return;
      }

      switch(state->stage()) {
      case ServiceState::Nowhere: // Не спроста пропущен брейк - так надо
      case ServiceState::CheckUpdate:
        this->startCheckUpdate(service, state);
        break;
      case ServiceState::PreHook:
        this->startPreHooks(service, state);
        break;
      case ServiceState::Download:
        this->startDownload(service, state);
        break;
      case ServiceState::PostHook:
        this->startPostHooks(service, state);
        break;
      case ServiceState::Extraction:
        this->startExtract(service, state);
        break;
      case ServiceState::Finished:
        this->startFinish(service, state);
        break;

      default:
        CRITICAL_LOG << "unknown stage " << state->stage() << "for service" << service->id();
      }
    }

    ServiceState* GameDownloadService::getStateById(const QString& id)
    { 
      if (!this->_stateMap.contains(id)) 
        return 0;

      return this->_stateMap[id];
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

      if (state->state() != ServiceState::Started)
        return;

      this->startStopping(service, state);
    }

    void GameDownloadService::startCheckUpdate(const Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      DEBUG_LOG << "Check for update. Service: " << service->id();
      state->setStage(ServiceState::CheckUpdate);
      emit this->checkUpdateRequest(service, CheckUpdateHelper::Normal);
    }

    void GameDownloadService::startPreHooks(const Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      DEBUG_LOG << "Start pre hooks. Service: " << service->id();
      state->setStage(ServiceState::PreHook);
      QtConcurrent::run(this, &GameDownloadService::preHookLoop, service);
    }

    void GameDownloadService::startPostHooks(const Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      DEBUG_LOG << "Start post hooks. Service: " << service->id();
      state->setStage(ServiceState::PostHook);
      QtConcurrent::run(this, &GameDownloadService::postHookLoop, service);
    }

    void GameDownloadService::startDownload(const Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      DEBUG_LOG << "Start download. Service: " << service->id();
      state->setStage(ServiceState::Download);
      emit this->downloadRequest(service, state->startType(), state->isFoundNewUpdate());
    }

    void GameDownloadService::startExtract(const Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      DEBUG_LOG << "Start extraction. Service: " << service->id();
      state->setStage(ServiceState::Extraction);
      ExtractorBase *extractor = this->getExtractorByType(service->extractorType());
      if (!extractor) {
        CRITICAL_LOG << "unknown extractor " << service->extractorType() << "for service" << service->id();
        state->setStage(ServiceState::Nowhere);

        state->setState(ServiceState::Unknown);
        emit this->failed(service);
        return;
      }

      QtConcurrent::run(extractor, &ExtractorBase::extract, service, state->startType());
    }

    void GameDownloadService::startFinish(const GGS::Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      DEBUG_LOG << "Finishing. Service: " << service->id();
      state->setStage(ServiceState::Finished);
      state->setState(ServiceState::Unknown);
      this->setIsInstalled(service->id(), true);
      Marketing::sendOnceByService(Marketing::FinishInstallService, service->id());
      emit this->finished(service);
    }

    void GameDownloadService::startStopping(const GGS::Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      DEBUG_LOG << "Stopping. Service: " << service->id();
      state->setState(ServiceState::Stopping);
      if (state->stage() == ServiceState::Download)
        emit this->downloadStopRequest(service);

      emit this->stopping(service);
    }

    void GameDownloadService::preHookLoop(const Core::Service *service)
    {
      Q_ASSERT(service);
      PauseRequestWatcher watcher(service);
      connect(this, SIGNAL(stopping(const GGS::Core::Service*)), 
        &watcher, SLOT(pauseRequestSlot(const GGS::Core::Service*)), Qt::DirectConnection);

      if (this->isStoppedOrStopping(service)) {
        this->setStoppedService(service);
        return;
      }
      
      emit this->statusMessageChanged(service, QObject::tr("PRE_HOOK_DEFAULT_MESSAGE"));
      HookBase::HookResult result = HookBase::Continue;
      if(this->_beforeDownloadHookMap.contains(service->id())) {
        Q_FOREACH(HookBase *hook, this->_beforeDownloadHookMap[service->id()]) {
          if (watcher.isPaused()) {
            this->setStoppedService(service);
            return;
          }

          result = hook->beforeDownload(this, service);
          emit this->statusMessageChanged(service, QObject::tr("PRE_HOOK_DEFAULT_MESSAGE"));
          if (result != HookBase::Continue)
            break;
        }
      }
      
      emit this->preHooksCompleted(service, result);
    }

    void GameDownloadService::postHookLoop(const Core::Service *service)
    {
      Q_ASSERT(service);
      PauseRequestWatcher watcher(service);
      connect(this, SIGNAL(stopping(const GGS::Core::Service*)), 
        &watcher, SLOT(pauseRequestSlot(const GGS::Core::Service*)), Qt::DirectConnection);

      if (this->isStoppedOrStopping(service)) {
        this->setStoppedService(service);
        return;
      }

      emit this->statusMessageChanged(service, QObject::tr("POST_HOOK_DEFAULT_MESSAGE"));

      HookBase::HookResult result = HookBase::Continue;
      if (this->_afterDownloadHookMap.contains(service->id())) {
        Q_FOREACH(HookBase *hook, this->_afterDownloadHookMap[service->id()]) {
          if (watcher.isPaused()) {
            this->setStoppedService(service);
            return;
          }

          result = hook->afterDownload(this, service);
          emit this->statusMessageChanged(service, QObject::tr("POST_HOOK_DEFAULT_MESSAGE"));
          if (result != HookBase::Continue)
            break;
        }
      }

      emit this->postHooksCompleted(service, result);
    }

    void GameDownloadService::preHooksCompletedSlot(const GGS::Core::Service *service, HookBase::HookResult result)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());

      if (state->state() == ServiceState::Stopping) {
        this->setStoppedState(service, state);
        return;
      } 

      if (state->state() != ServiceState::Stopped) {
        if (result == HookBase::Continue)
          this->startDownload(service, state);
        else 
          this->hookResultRouter(service, state, result);
      }
    }

    void GameDownloadService::downloadRequestCompleted(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      Q_ASSERT(state);

      if (state->state() == ServiceState::Stopping) {
        this->setStoppedState(service, state);
        return;
      } 

      Marketing::sendOnceByService(Marketing::FinishDownloadService, service->id());
      if (state->state() != ServiceState::Stopped)
        this->startExtract(service, state);
    }

    void GameDownloadService::postHooksCompletedSlot(const GGS::Core::Service *service, GGS::GameDownloader::HookBase::HookResult result)
    {
      Q_ASSERT(service);
      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      Q_ASSERT(state);

      if (state->state() == ServiceState::Stopping) {
        this->setStoppedState(service, state);
        return;
      } 

      if (state->state() != ServiceState::Stopped) {
        if (result == HookBase::Continue)
          this->startFinish(service, state);
        else
          this->hookResultRouter(service, state, result);
      }
    }

    void GameDownloadService::hookResultRouter(const GGS::Core::Service *service, ServiceState *state, HookBase::HookResult result)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      // UNDONE: подумать нужно ли защиту от дурака на посдечет цикличных хуков.
      switch(result) {
      case HookBase::Continue:
        CRITICAL_LOG << "hook router can't decide where to continue. Serivce: " << service->id();
        break;
      case HookBase::Abort:
        this->setStoppedState(service, state);
        break;
      case HookBase::CheckUpdate:
        this->startCheckUpdate(service, state);
        break;
      case HookBase::PreHookStartPoint:
        this->startPreHooks(service, state);
        break;
      case HookBase::PreHookEndPoint:
        emit this->preHooksCompleted(service, HookBase::Continue);
        break;
      case HookBase::PostHookStartPoint:
        this->startPostHooks(service, state);
        break;
      case HookBase::PostHookEndPoint:
        emit this->postHooksCompleted(service, HookBase::Continue);
        break;
      case HookBase::DownloadStartPoint:
        this->startDownload(service, state);
        break;
      case HookBase::DownloadEndPoint:
        emit this->downloadRequestCompleted(service);
        break;
      case HookBase::ExtractionStartPoint:
        this->startExtract(service, state);
        break;
      case HookBase::ExtractionEndPoint: // break пропущен не случайно.
      case HookBase::Finish:
        this->startFinish(service, state);
        break;
      default:
        CRITICAL_LOG << "hookResultRouter Unknown result: " << result << "for service" << service->id();
      };
    }

    bool GameDownloadService::isStoppedOrStopping(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      if (!state)
        return true;

      return state->state() == ServiceState::Stopped || state->state() == ServiceState::Stopping;
    }

    void GameDownloadService::setStoppedService(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      DEBUG_LOG << "Stopped. Service: " << service->id();
      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      Q_ASSERT(state);
      this->setStoppedState(service, state);
    }

    void GameDownloadService::pauseRequestCompleted(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      Q_ASSERT(state);

      if ((state->state() == ServiceState::Started || state->state() == ServiceState::Stopping) &&
           (state->stage() == ServiceState::Download || state->stage() == ServiceState::Extraction))
        this->setStoppedState(service, state);
    }

    void GameDownloadService::checkUpdateRequestCompleted(const GGS::Core::Service *service, bool isUpdated)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      Q_ASSERT(state);

      state->setIsFoundNewUpdate(isUpdated);

      if (state->state() == ServiceState::Stopping) {
        this->setStoppedState(service, state);
        return;
      } 

      if (state->state() != ServiceState::Stopped)
        this->startPreHooks(service, state);
    }

    void GameDownloadService::init()
    {
      QObject::connect(this, 
        SIGNAL(preHooksCompleted(const GGS::Core::Service*,GGS::GameDownloader::HookBase::HookResult)), 
        SLOT(preHooksCompletedSlot(const GGS::Core::Service*,GGS::GameDownloader::HookBase::HookResult)), 
        Qt::QueuedConnection);

      QObject::connect(this, 
        SIGNAL(postHooksCompleted(const GGS::Core::Service*,GGS::GameDownloader::HookBase::HookResult)), 
        SLOT(postHooksCompletedSlot(const GGS::Core::Service*,GGS::GameDownloader::HookBase::HookResult)), 
        Qt::QueuedConnection);

      QObject::connect(&this->_progressCalculator, SIGNAL(progressChanged(QString, qint8)), 
      this, SLOT(internalProgressChangedSlot(QString, qint8)));
      QObject::connect(
        &this->_progressCalculator, 
        SIGNAL(progressDownloadChanged(QString, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)), 
        this, 
        SLOT(internalProgressDownloadChangedSlot(QString, qint8, GGS::Libtorrent::EventArgs::ProgressEventArgs)));
      QObject::connect(&this->_progressCalculator, SIGNAL(progressExtractionChanged(QString, qint8, qint64, qint64)), 
        this, SLOT(internalProgressExtractionChangedSlot(QString, qint8, qint64, qint64)));
    }

    void GameDownloadService::checkUpdateFailed(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      this->downloadServiceFailed(service);
    }

    void GameDownloadService::downloadServiceFailed(const Core::Service *service)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      state->setStage(ServiceState::Nowhere);

      state->setState(ServiceState::Unknown);
      emit this->failed(service);
    }

    void GameDownloadService::registerExtractor(ExtractorBase *extractor)
    {
      Q_ASSERT(extractor);

      QString type = extractor->extractorId();
      Q_ASSERT(!this->_extractorMap.contains(type));
      
      extractor->setGameDownloadService(this);
      this->_extractorMap[type] = extractor;
      QObject::connect(this, SIGNAL(stopping(const GGS::Core::Service*)), 
        extractor, SLOT(pauseRequestSlot(const GGS::Core::Service*)), Qt::QueuedConnection);

      QObject::connect(extractor, SIGNAL(extractFinished(const GGS::Core::Service *)), 
        this, SLOT(extractionCompleted(const GGS::Core::Service *)), Qt::QueuedConnection);
      QObject::connect(extractor, SIGNAL(extractPaused(const GGS::Core::Service *)), 
        this, SLOT(pauseRequestCompleted(const GGS::Core::Service *)), Qt::QueuedConnection);
      QObject::connect(extractor, SIGNAL(extractFailed(const GGS::Core::Service *)), 
        this, SLOT(extractionFailed(const GGS::Core::Service *)), Qt::QueuedConnection);
      QObject::connect(extractor, SIGNAL(extractionProgressChanged(const QString&, qint8, qint64, qint64)), 
        &this->_progressCalculator, SLOT(extractionProgress(const QString&, qint8, qint64, qint64)), 
        Qt::QueuedConnection);
    }

    ExtractorBase* GameDownloadService::getExtractorByType(const QString& type)
    {
        if (!this->_extractorMap.contains(type))
          return 0;

        return this->_extractorMap[type];
    }

    void GameDownloadService::extractionCompleted(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      ServiceState *state = this->getStateById(service->id());
      Q_ASSERT(state);

      if (state->state() == ServiceState::Stopping) {
        this->setStoppedState(service, state);
        return;
      } 

      if (state->state() != ServiceState::Stopped) 
        this->startPostHooks(service, state);
    }

    void GameDownloadService::extractionFailed(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      this->downloadServiceFailed(service);
    }

    StageProgressCalculator& GameDownloadService::progressCalculator()
    {
      return this->_progressCalculator;
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
      this->_timeForResume = seconds * 1000;
    }

    void GameDownloadService::downloadFailed(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      this->downloadServiceFailed(service);
    }

    void GameDownloadService::shutdown()
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown) {
        CRITICAL_LOG << "GameDownloadService : double shutdown request";
        return;
      }

      this->_isShuttingDown = true;

      Q_FOREACH(ServiceState *state, this->_stateMap) {
        if (state->state() != ServiceState::Started)
          continue;

        this->_serviceStopList.insert(state->id());
        this->startStopping(state->service(), state);
      }

      if (this->_serviceStopList.count() <= 0) {
        emit this->shutdownCompleted();
      }
    }

    void GameDownloadService::setStoppedState(const GGS::Core::Service *service, ServiceState *state)
    {
      Q_ASSERT(service);
      Q_ASSERT(state);
      state->setState(ServiceState::Stopped);
      emit this->stopped(service);

      if (this->_isShuttingDown && this->_serviceStopList.contains(service->id())) {
        this->_serviceStopList.remove(service->id());
        if (this->_serviceStopList.count() <= 0)
          emit this->shutdownCompleted();
      }
    }

    void GameDownloadService::directoryChanged(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);

      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return;

      ServiceState *state = this->getStateById(service-> id());
      if (!state) {
        return;
      }

      state->setIsDirectoryChanged(true);
      if (state->state() != ServiceState::Started)
        return;

      this->startStopping(service, state);
    }

    bool GameDownloadService::isInstalled(const QString& serviceId)
    {
      Q_ASSERT(!serviceId.isEmpty());
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup(serviceId);
      bool ok;
      int result = settings.value("isInstalled", 0).toInt(&ok);
      return ok && result == 1;
    }

    bool GameDownloadService::isInstalled(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      return this->isInstalled(service->id());
    }

    void GameDownloadService::setIsInstalled(const QString& serviceId, bool isInstalled)
    {
      Q_ASSERT(!serviceId.isEmpty());
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup(serviceId);
      settings.setValue("isInstalled", isInstalled ? 1 : 0);
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

      return !(state->state() == ServiceState::Stopped || state->state() == ServiceState::Unknown);
    }

    bool GameDownloadService::isAnyServiceInProgress()
    {
      QMutexLocker lock(&this->_stateLock);
      if (this->_isShuttingDown)
        return false;

      bool result = false;
      Q_FOREACH(ServiceState *state, this->_stateMap)
        result |= !(state->state() == ServiceState::Stopped || state->state() == ServiceState::Unknown);

      return result;
    }

  }
}
