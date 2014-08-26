/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/TorrentDownloadBehavior.h>
#include <GameDownloader/CheckUpdateHelper>
#include <GameDownloader/ServiceState.h>

#include <LibtorrentWrapper/TorrentConfig>
#include <LibtorrentWrapper/Wrapper>

#include <Core/Service>
#include <Core/Marketing.h>

#include <QtCore/QMutexLocker>
#include <QtCore/QDateTime>
#include <QtConcurrent/QtConcurrentRun>

using namespace GGS::Libtorrent;
using GGS::Core::Marketing;
using GGS::Libtorrent::EventArgs::ProgressEventArgs;

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      TorrentDownloadBehavior::TorrentDownloadBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      TorrentDownloadBehavior::~TorrentDownloadBehavior()
      {
      }

      void TorrentDownloadBehavior::start(GGS::GameDownloader::ServiceState *state)
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

      void TorrentDownloadBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        // We have to run this in new thread
        QtConcurrent::run(this, &TorrentDownloadBehavior::syncStopTorrent, state);
      }

      void TorrentDownloadBehavior::torrentPausedSlot(QString id)
      {
        GGS::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->next(Paused, state);
      }

      void TorrentDownloadBehavior::torrentDownloadFinishedSlot(QString id)
      {
        GGS::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        Marketing::sendOnceByService(Marketing::FinishDownloadService, id);
        emit this->next(Downloaded, state);
      }

      void TorrentDownloadBehavior::torrentDownloadFailedSlot(QString id)
      {
        GGS::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->failed(state);
      }

      void TorrentDownloadBehavior::setState(GGS::GameDownloader::ServiceState *state)
      {
        QMutexLocker lock(&this->_mutex);
        this->_stateMap[state->id()] = state;
      }

      GGS::GameDownloader::ServiceState* TorrentDownloadBehavior::state(const QString& id)
      {
        QMutexLocker lock(&this->_mutex);
        if (!this->_stateMap.contains(id))
          return 0;

        return this->_stateMap[id];
      }

      void TorrentDownloadBehavior::setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper)
      {
        this->_wrapper = wrapper;
        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(torrentPaused(QString)), 
          this, SLOT(torrentPausedSlot(QString)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(torrentDownloadFinished(QString)), 
          this, SLOT(torrentDownloadFinishedSlot(QString)), Qt::QueuedConnection));

        // Просто реемитим ошибку безликую, при желании можно что-нить поконкретнее пробросить
        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(torrentError(QString)), 
          this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(fileError(QString, QString, int)), 
          this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(startTorrentFailed(QString, int)), 
          this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(
          wrapper, 
          SIGNAL(progressChanged(GGS::Libtorrent::EventArgs::ProgressEventArgs)), 
          this,
          SLOT(torrentProgress(GGS::Libtorrent::EventArgs::ProgressEventArgs))));
      }

      void TorrentDownloadBehavior::syncStartTorrent(GGS::GameDownloader::ServiceState* state)
      {
        this->setState(state);

        TorrentConfig config;
        config.setPathToTorrentFile(CheckUpdateHelper::getTorrentPath(state));
        config.setDownloadPath(state->service()->downloadPath());
        config.setIsForceRehash(state->startType() == Recheck);
        config.setIsReloadRequired(state->startType() == Recheck || state->isFoundNewUpdate());
        this->_wrapper->start(state->id(), config);
      }

      void TorrentDownloadBehavior::syncStopTorrent(GGS::GameDownloader::ServiceState * state)
      {
        this->setState(state);
        this->_wrapper->stop(state->id());
      }

      void TorrentDownloadBehavior::torrentProgress(GGS::Libtorrent::EventArgs::ProgressEventArgs arg)
      {
        GGS::GameDownloader::ServiceState *state = this->state(arg.id());
        if (!state || state->currentBehavior() != this)
          return;

        float startPoint = 0.0f;
        float size = 0.0f;
        float total = 60.0f;

        if (arg.status() == ProgressEventArgs::CheckingFiles) {
          startPoint = 0;
          size = 20;
        } else if (arg.status() == ProgressEventArgs::Downloading) {
          startPoint = 20;
          size = 40;
        } else {
          return;
        }

        qint8 res = static_cast<qint8>(100.0f * (startPoint + arg.progress() * size) / total);

        emit this->downloadProgressChanged(state, res, arg);
      }

    }
  }
}