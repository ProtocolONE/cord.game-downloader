#include <GameDownloader/Behavior/ReadOnlyBehavior.h>
#include <GameDownloader/Behavior/Private/DirProcessor.h>
#include <GameDownloader/ServiceState.h>
#include <Core/Service.h>

#include <QtCore/QDirIterator>
#include <QtCore/QStringList>
#include <QtConcurrent/QtConcurrentRun>

#include <Windows.h>

#include <vector>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      ReadOnlyBehavior::ReadOnlyBehavior(QObject *parent /*= 0*/) : BaseBehavior(parent)
      {
        
      }

      ReadOnlyBehavior::~ReadOnlyBehavior()
      {

      }

      void ReadOnlyBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        if (!state->isDownloadSuccess() || state->startType() == Recheck) {
          const P1::Core::Service *service = state->service();
          QString installPath = service->installPath();
          QString downloadPath = service->downloadPath();

          QStringList folders;
          folders << installPath;

          bool isSignleFolder = installPath == downloadPath;

          if (!isSignleFolder) {
            folders << downloadPath;
          }

          DirProcessor *worker = new DirProcessor;

          QObject::connect(worker, &DirProcessor::showMessage, this, &ReadOnlyBehavior::keepCalmMessage);
          QObject::connect(worker, &DirProcessor::resultReady, this, &ReadOnlyBehavior::processFinished);

          QtConcurrent::run(worker, &DirProcessor::processFolders, folders, state);
        } else {
          emit this->next(Finished, state);
        }
      }

      void ReadOnlyBehavior::stop(P1::GameDownloader::ServiceState *state)
      {

      }

      void ReadOnlyBehavior::keepCalmMessage(P1::GameDownloader::ServiceState *state)
      {
        emit this->statusMessageChanged(state, QObject::tr("FOLDER_PROCESSING_MESSAGE"));
      }

      void ReadOnlyBehavior::processFinished(int result, P1::GameDownloader::ServiceState *state)
      {
        DirProcessor *worker = qobject_cast<DirProcessor *>(QObject::sender());
        worker->deleteLater();

        emit this->next(result, state);
      }

    }
  }
}