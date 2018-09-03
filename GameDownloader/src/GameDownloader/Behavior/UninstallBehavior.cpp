#include <GameDownloader/Behavior/UninstallBehavior.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service.h>

#include <LibtorrentWrapper/Wrapper.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QSettings>
#include <QtCore/QStringList>
#include <QtConcurrent/QtConcurrentRun>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      UninstallBehavior::UninstallBehavior(QObject *parent)
        : BaseBehavior(parent)
        , _wrapper(nullptr)
      {
      }

      UninstallBehavior::~UninstallBehavior()
      {
      }


      void UninstallBehavior::setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper)
      {
        Q_ASSERT(wrapper);
        this->_wrapper = wrapper;
      }

      void UninstallBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->next(Paused, state);
          return;
        }

        if (state->startType() != Uninstall) {
          emit this->next(ContinueInstall, state);
          return;
        }

        QtConcurrent::run(this, &UninstallBehavior::uninstall, state);
      }

      void UninstallBehavior::stop(P1::GameDownloader::ServiceState *state)
      {

      }

      void UninstallBehavior::uninstall(P1::GameDownloader::ServiceState *state)
      {
        Q_ASSERT(this->_wrapper);
        const P1::Core::Service *service = state->service();
        Q_ASSERT(service);
        
        qint8 progressValue = 0;
        int filesTotal = 0;
        int filesDeleted = 0;

        QStringList dirsToDelete = this->getDirectoriesToRemove(service);
        if (dirsToDelete.isEmpty()) {
          emit this->failed(state);
          return;
        }
        
        emit statusMessageChanged(state, QObject::tr("STATE_PREPARE_UNINSTALL").arg(service->displayName()));
        emit this->totalProgressChanged(state, 0);

        foreach(QString dirName, dirsToDelete) {
          filesTotal += this->getFilesCount(dirName);
        }
                
        emit statusMessageChanged(state, QObject::tr("STATUS_INPROGESS").arg(service->displayName()));

        QString resumeFileName = this->_wrapper->getFastResumeFilePath(service->id());
        if (QFile::exists(resumeFileName))
          QFile::remove(resumeFileName);

        foreach(QString dirName, dirsToDelete) {
          QDirIterator it(dirName, QDir::Files, QDirIterator::Subdirectories);
          while (it.hasNext()) {
            QString fileName = it.next();
            QFile::remove(fileName);
            filesDeleted++;

            qint8 newProgress = (filesDeleted*100)/filesTotal;

            if (newProgress != progressValue) {
              progressValue = newProgress;
              emit this->totalProgressChanged(state, progressValue);
            }
          }
          QDir dir(dirName);
          dir.removeRecursively();
        }

        this->removeEmptyFolders(service);

        emit statusMessageChanged(state, QObject::tr("STATUS_FINISHED").arg(service->displayName()));
        emit finished(state);
      }

      QStringList UninstallBehavior::getDirectoriesToRemove(const P1::Core::Service *service) 
      {
        //INFO Пытается удалить только то, что поставили мы сами.
        QStringList result;

        QString installPath = service->installPath();
        if (installPath.isEmpty()) {
          return result;
        }

        QString distribPath = service->downloadPath();
        if (!service->hashDownloadPath() || distribPath == installPath) {
          result << installPath + "/" + service->areaString();
          return result;
        }
        
        installPath += "/" + service->areaString();
        distribPath += "/" + service->areaString();

        result << installPath << distribPath;
      
        return result;
      }

      int UninstallBehavior::getFilesCount(const QString &directory)
      {
        int result = 0;
        QDirIterator it(directory, QDir::Files, QDirIterator::Subdirectories);
        while (it.hasNext()) {
          ++result;
          it.next();
        }

        return result;
      }

      void UninstallBehavior::removeEmptyFolders(const P1::Core::Service *service)
      {
        QDir installDir(service->installPath());
        if (installDir.count() == 0)
          installDir.removeRecursively();

        QDir downloadDir(service->downloadPath());
        if (downloadDir.count() == 0)
          downloadDir.removeRecursively();
      }
    }
  }
}