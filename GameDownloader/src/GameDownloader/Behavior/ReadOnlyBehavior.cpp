/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (ñ) 2011 - 2017, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/ReadOnlyBehavior.h>
#include <GameDownloader/Behavior/Private/DirProcessor.h>
#include <GameDownloader/ServiceState.h>
#include <Core/Service.h>

#include <QtCore/QDirIterator>
#include <QtCore/QStringList>
#include <QtConcurrent/QtConcurrentRun>

#include <Windows.h>

#include <vector>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      ReadOnlyBehavior::ReadOnlyBehavior(QObject *parent /*= 0*/) : BaseBehavior(parent)
      {
        
      }

      ReadOnlyBehavior::~ReadOnlyBehavior()
      {

      }

      void ReadOnlyBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        if (!state->isDownloadSuccess() || state->startType() == Recheck) {
          const GGS::Core::Service *service = state->service();
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

      void ReadOnlyBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {

      }

      void ReadOnlyBehavior::keepCalmMessage(GGS::GameDownloader::ServiceState *state)
      {
        emit this->statusMessageChanged(state, QObject::tr("FOLDER_PROCESSING_MESSAGE"));
      }

      void ReadOnlyBehavior::processFinished(int result, GGS::GameDownloader::ServiceState *state)
      {
        DirProcessor *worker = qobject_cast<DirProcessor *>(QObject::sender());
        worker->deleteLater();

        emit this->next(result, state);
      }

    }
  }
}