#include <GameDownloader/Behavior/TorrentDownloadBehavior.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/ServiceState.h>

#include <LibtorrentWrapper/TorrentConfig.h>
#include <LibtorrentWrapper/Wrapper.h>

#include <Core/Service.h>

#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <QtConcurrent/QtConcurrentRun>

using namespace P1::Libtorrent;
using P1::Libtorrent::EventArgs::ProgressEventArgs;

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      TorrentDownloadBehavior::TorrentDownloadBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      TorrentDownloadBehavior::~TorrentDownloadBehavior()
      {
      }

      void TorrentDownloadBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());
        Q_CHECK_PTR(this->_wrapper);

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        // We have to run this in new thread
        QtConcurrent::run(this, &TorrentDownloadBehavior::syncStartTorrent, state);
      }

      void TorrentDownloadBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        // We have to run this in new thread
        QtConcurrent::run(this, &TorrentDownloadBehavior::syncStopTorrent, state);
      }

      void TorrentDownloadBehavior::torrentPausedSlot(QString id)
      {
        P1::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->next(Paused, state);
      }

      void TorrentDownloadBehavior::torrentDownloadFinishedSlot(QString id)
      {
        P1::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->downloadFinished(state);
        emit this->next(Downloaded, state);
      }

      void TorrentDownloadBehavior::torrentDownloadFailedSlot(QString id)
      {
        P1::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->failed(state);
      }

      void TorrentDownloadBehavior::setState(P1::GameDownloader::ServiceState *state)
      {
        QMutexLocker lock(&this->_mutex);
        this->_stateMap[state->id()] = state;
        this->_installedStateMap[state->id()] = state->isInstalled();
      }

      P1::GameDownloader::ServiceState* TorrentDownloadBehavior::state(const QString& id)
      {
        QMutexLocker lock(&this->_mutex);
        if (!this->_stateMap.contains(id))
          return 0;

        return this->_stateMap[id];
      }

      bool TorrentDownloadBehavior::isInstalled(const QString& id)
      {
        QMutexLocker lock(&this->_mutex);
        if (!this->_installedStateMap.contains(id))
          return false;

        return this->_installedStateMap[id];
      }

      void TorrentDownloadBehavior::setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper)
      {
        this->_wrapper = wrapper;

        QObject::connect(this->_wrapper, &Wrapper::torrentPaused,
          this, &TorrentDownloadBehavior::torrentPausedSlot, Qt::QueuedConnection);

        QObject::connect(this->_wrapper, &Wrapper::torrentDownloadFinished,
          this, &TorrentDownloadBehavior::torrentDownloadFinishedSlot, Qt::QueuedConnection);

        // Просто реемитим ошибку безликую, при желании можно что-нить поконкретнее пробросить
        QObject::connect(this->_wrapper, &Wrapper::torrentError,
          this, &TorrentDownloadBehavior::torrentDownloadFailedSlot, Qt::QueuedConnection);
        QObject::connect(this->_wrapper, &Wrapper::fileError,
          this, &TorrentDownloadBehavior::torrentDownloadFailedSlot, Qt::QueuedConnection);
        QObject::connect(this->_wrapper, &Wrapper::startTorrentFailed,
          this, &TorrentDownloadBehavior::torrentDownloadFailedSlot, Qt::QueuedConnection);

        QObject::connect(this->_wrapper, &Wrapper::progressChanged,
          this, &TorrentDownloadBehavior::torrentProgress);
      }

      void TorrentDownloadBehavior::syncStartTorrent(P1::GameDownloader::ServiceState* state)
      {
        this->setState(state);

        bool needReload = state->isForceReload();

        TorrentConfig config;
        config.setPathToTorrentFile(CheckUpdateHelper::getTorrentPath(state));
        config.setDownloadPath(state->service()->downloadPath());
        config.setIsForceRehash(state->startType() == Recheck);
        config.setIsReloadRequired(state->startType() == Recheck || state->isFoundNewUpdate() || needReload);

        if (needReload)
          state->setForceReaload(!needReload);

        this->_wrapper->start(state->id(), config);
      }

      void TorrentDownloadBehavior::syncStopTorrent(P1::GameDownloader::ServiceState * state)
      {
        this->setState(state);
        this->_wrapper->stop(state->id());
      }

      void TorrentDownloadBehavior::torrentProgress(P1::Libtorrent::EventArgs::ProgressEventArgs arg)
      {
        P1::GameDownloader::ServiceState *state = this->state(arg.id());
        if (!state || state->currentBehavior() != this)
          return;

        bool installed = this->isInstalled(arg.id());

        float startPoint = 0.0f;
        float size = 0.0f;
        float total = 60.0f;

        if (arg.status() == ProgressEventArgs::CheckingFiles) {
          startPoint = 0;
          size = installed ? 20 : 1;
        } else if (arg.status() == ProgressEventArgs::Downloading) {
          startPoint = installed ? 20 : 1;
          size = installed ? 40 : 59;
        } else {
          return;
        }

        qint8 res = static_cast<qint8>(100.0f * (startPoint + arg.progress() * size) / total);

        emit this->downloadProgressChanged(state, res, arg);
      }

    }
  }
}