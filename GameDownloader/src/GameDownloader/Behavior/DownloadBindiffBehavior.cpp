#include <GameDownloader/Behavior/DownloadBindiffBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <LibtorrentWrapper/TorrentConfig.h>
#include <LibtorrentWrapper/Wrapper.h>

#include <Settings/Settings.h>
#include <Core/Service.h>

#include <QtCore/QMutexLocker>
#include <QtConcurrent/QtConcurrentRun>
#include <QtCore/QFile>

#include <UpdateSystem/Hasher/Md5FileHasher.h>

using namespace P1::Libtorrent;
using P1::Libtorrent::EventArgs::ProgressEventArgs;

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      DownloadBindiffBehavior::DownloadBindiffBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      DownloadBindiffBehavior::~DownloadBindiffBehavior()
      {
      }

      void DownloadBindiffBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());
        Q_CHECK_PTR(this->_wrapper);

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        // We have to run this in new thread
        QtConcurrent::run(this, &DownloadBindiffBehavior::syncStartTorrent, state);
      }

      void DownloadBindiffBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        // We have to run this in new thread
        QtConcurrent::run(this, &DownloadBindiffBehavior::syncStopTorrent, state);
      }

      void DownloadBindiffBehavior::setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper)
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

        SIGNAL_CONNECT_CHECK(QObject::connect(
          wrapper, 
          SIGNAL(progressChanged(P1::Libtorrent::EventArgs::ProgressEventArgs)), 
          this,
          SLOT(torrentProgress(P1::Libtorrent::EventArgs::ProgressEventArgs))));
      }

      void DownloadBindiffBehavior::torrentPausedSlot(QString id)
      {
        P1::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->next(Paused, state);
      }

      void DownloadBindiffBehavior::torrentDownloadFinishedSlot(QString id)
      {
        P1::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        QString torrentFilePath = this->getPatchTorrentPath(state);
        QList<QString> fileList;
        bool result = this->_wrapper->getFileList(torrentFilePath, fileList);

        if (!result) {
          emit this->failed(state);
          return;
        }

        state->setPatchFiles(fileList);
        
        QString removeStr = ".diff";
        QStringList packFiles;

        Q_FOREACH(QString fileName, fileList) {
          packFiles << fileName.left(fileName.length() - removeStr.length());
        }

        state->setPackingFiles(packFiles);
        this->moveT1toT0(state);

        emit this->next(Downloaded, state);
      }

      void DownloadBindiffBehavior::moveT1toT0(P1::GameDownloader::ServiceState *state)
      {
        QString t0 = CheckUpdateHelper::getTorrentPath(state);
        QString t1 = QString("%1/patch/%3/%4.torrent")
          .arg(state->service()->torrentFilePath())
          .arg(state->service()->areaString())
          .arg(state->id());

        QFile::rename(t0, t0 + ".old");
        QFile::rename(t1, t0);
        QFile::remove(t0 + ".old");
        
        P1::Hasher::Md5FileHasher hasher;
        QString hash = hasher.getFileHash(CheckUpdateHelper::getTorrentPath(state));

        CheckUpdateHelper::saveTorrenthash(hash, state);
        CheckUpdateHelper::saveLastModifiedDate(state->torrentLastModifedDate(), state);
      }

      void DownloadBindiffBehavior::torrentDownloadFailedSlot(QString id)
      {
        P1::GameDownloader::ServiceState *state = this->state(id);
        if (!state || state->currentBehavior() != this)
          return;

        emit this->failed(state);
      }

      void DownloadBindiffBehavior::setState(P1::GameDownloader::ServiceState *state)
      {
        QMutexLocker lock(&this->_mutex);
        this->_stateMap[this->getPatchId(state)] = state;
      }

      ServiceState* DownloadBindiffBehavior::state(const QString& id)
      {
        QMutexLocker lock(&this->_mutex);
        if (!this->_stateMap.contains(id))
          return 0;

        return this->_stateMap[id];
      }

      void DownloadBindiffBehavior::syncStartTorrent(P1::GameDownloader::ServiceState *state)
      {
        // UNDONE
        this->setState(state);

        const P1::Core::Service *service = state->service();

        QString torrentFilePath = this->getPatchTorrentPath(state);

        QString path = QString("%1/patch/%2/")
          .arg(service->torrentFilePath())
          .arg(state->patchVersion());

        TorrentConfig config;
        config.setPathToTorrentFile(torrentFilePath);
        config.setDownloadPath(path);
        config.setIsForceRehash(false);
        config.setIsReloadRequired(false);  // UNDONE - �� ����� ������ ��� ���� ��������� � � ���� ������ ����������
        config.setIsSeedEnable(false);

        this->_wrapper->start(this->getPatchId(state), config);
      }

      void DownloadBindiffBehavior::syncStopTorrent(P1::GameDownloader::ServiceState *state)
      {
        this->setState(state);
        this->_wrapper->stop(this->getPatchId(state));
      }

      QString DownloadBindiffBehavior::getPatchTorrentPath(P1::GameDownloader::ServiceState *state)
      {
        const P1::Core::Service* service = state->service();
        QString torrentFilePath = QString("%1/patch/%2/%3/%4.torrent")
          .arg(service->torrentFilePath())
          .arg(state->patchVersion())
          .arg(service->areaString())
          .arg(service->id());	
        return torrentFilePath;
      }

      void DownloadBindiffBehavior::torrentProgress(P1::Libtorrent::EventArgs::ProgressEventArgs arg)
      {
        P1::GameDownloader::ServiceState *state = this->state(arg.id());
        if (!state || state->currentBehavior() != this)
          return;

        float startPoint = 0.0f;
        float size = 0.0f;
        float total = 60.0f;

        bool isRehashing;
        if (arg.status() == ProgressEventArgs::CheckingFiles) {
          startPoint = 0;
          size = 20;
          isRehashing = true;
        } else if (arg.status() == ProgressEventArgs::Downloading) {
          startPoint = 20;
          size = 40;
          isRehashing = false;
        } else {
          return;
        }

        qint8 res = static_cast<qint8>(100.0f * (startPoint + arg.progress() * size) / total);
        // UNDONE ��� �� ���� �������� ���������, �� �������� ����������
        if (isRehashing)
          emit this->totalProgressChanged(state, res);
        else
          emit this->downloadProgressChanged(state, res, arg);
      }

      QString DownloadBindiffBehavior::getPatchId(const P1::GameDownloader::ServiceState* state)
      {
        return "Patch_" + state->id();
      }

    }
  }
}