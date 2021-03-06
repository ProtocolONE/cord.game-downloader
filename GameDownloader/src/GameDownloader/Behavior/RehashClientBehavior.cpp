#include <GameDownloader/Behavior/RehashClientBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <LibtorrentWrapper/TorrentConfig.h>
#include <LibtorrentWrapper/Wrapper.h>

#include <Core/Service.h>

#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFile>

using namespace P1::Libtorrent;
using P1::Libtorrent::EventArgs::ProgressEventArgs;


namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      RehashClientBehavior::RehashClientBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      RehashClientBehavior::~RehashClientBehavior()
      {
      }

      void RehashClientBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());
        Q_CHECK_PTR(this->_wrapper);

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        // ��� �������������� ������� ������ ������ �� ����� � ����������� �������� �������/����
        if (state->startType() == Recheck) {
          emit this->next(GameBroken, state);
          return;
        }

        QString path = CheckUpdateHelper::getTorrentPath(state);
        if (!QFile::exists(path)) {
          emit this->next(GameBroken, state);
          return;
        }

        QString msg = QObject::tr("REHASH_CURRENT_CLIENT");
        emit this->statusMessageChanged(state, msg);

        // We have to run this in new thread
        QtConcurrent::run(this, &RehashClientBehavior::syncStartTorrent, state);
      }

      void RehashClientBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
        // We have to run this in new thread
        QtConcurrent::run(this, &RehashClientBehavior::syncStopTorrent, state);
      }

      void RehashClientBehavior::setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper)
      {
        this->_wrapper = wrapper;

        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(torrentPaused(QString)), 
          this, SLOT(torrentPausedSlot(QString)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(torrentDownloadFinished(QString)), 
          this, SLOT(torrentDownloadFinishedSlot(QString)), Qt::QueuedConnection));

        // ������ �������� ������ ��������, ��� ������� ����� ���-���� ������������ ����������
        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(torrentError(QString)), 
          this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(fileError(QString, QString, int)), 
          this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection));
        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(startTorrentFailed(QString, int)), 
          this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(this->_wrapper, SIGNAL(torrentRehashed(QString, bool)), 
          this, SLOT(torrentDownloadRehashed(QString, bool)), Qt::QueuedConnection));

        SIGNAL_CONNECT_CHECK(QObject::connect(
          wrapper, 
          SIGNAL(progressChanged(P1::Libtorrent::EventArgs::ProgressEventArgs)), 
          this,
          SLOT(torrentProgress(P1::Libtorrent::EventArgs::ProgressEventArgs))));
      }

      void RehashClientBehavior::torrentPausedSlot(QString id)
      {
        ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->next(Paused, state);
      }

      void RehashClientBehavior::torrentDownloadFinishedSlot(QString id)
      {
        //ServiceState *state = this->state(id);
        //if (!state || state->currentBehavior() != this)
        //  return;

        //DEBUG_LOG << "Download finished slot on rehash " << id;
      }

      void RehashClientBehavior::torrentDownloadFailedSlot(QString id)
      {
        ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->next(GameBroken, state);
      }

      void RehashClientBehavior::torrentDownloadRehashed(QString id, bool isComplete)
      {
        ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        // UNDONE �������� ����� � �� ����� ��� ��������� ���������
        state->setIsGameClientComplete(isComplete);
        emit this->next(isComplete ? GameFine : GameBroken, state);
      }

      void RehashClientBehavior::setState(P1::GameDownloader::ServiceState *state)
      {
        QMutexLocker lock(&this->_mutex);
        this->_stateMap[state->id()] = state;
      }

      P1::GameDownloader::ServiceState* RehashClientBehavior::state(const QString& id)
      {
        QMutexLocker lock(&this->_mutex);
        if (!this->_stateMap.contains(id))
          return 0;

        return this->_stateMap[id];
      }

      void RehashClientBehavior::syncStartTorrent(P1::GameDownloader::ServiceState *state)
      {
        // UNDONE �������� - ����� ��������� ������ ���� � ����� ����� � ������ ������ - �������� GameBroken
        this->setState(state);

        TorrentConfig config;
        config.setPathToTorrentFile(CheckUpdateHelper::getTorrentPath(state));
        config.setDownloadPath(state->service()->downloadPath());
        
        config.setIsForceRehash(false);
        //config.setIsForceRehash(state->startType() == Recheck);

        config.setIsReloadRequired(false);
        config.setRehashOnly(true);
        this->_wrapper->start(state->id(), config);
      }

      void RehashClientBehavior::syncStopTorrent(P1::GameDownloader::ServiceState *state)
      {
        this->setState(state);
        this->_wrapper->stop(state->id());
      }

      void RehashClientBehavior::torrentProgress(P1::Libtorrent::EventArgs::ProgressEventArgs arg)
      {
        P1::GameDownloader::ServiceState *state = this->state(arg.id());
        if (!state || state->currentBehavior() != this)
          return;

        if (arg.status() != ProgressEventArgs::CheckingFiles) 
          return;
        
        qint8 res = static_cast<qint8>(100.0f * arg.progress());
        emit this->totalProgressChanged(state, res);
      }

    }
  }
}