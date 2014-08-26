/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/BindiffBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>

#include <QtConcurrent/QtConcurrentRun>
#include <Settings/Settings.h>
#include <Core/Service>

#include <QtCore/QByteArray>
#include <QtCore/QDir>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      BindiffBehavior::BindiffBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      BindiffBehavior::~BindiffBehavior()
      {
      }

      void BindiffBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        
        emit this->totalProgressChanged(state, 0);

        QString id = state->id();
        const GGS::Core::Service* service = state->service();

        BindiffBehaviorPrivate* data = new BindiffBehaviorPrivate();
        data->_state = state;

        data->_rootDir = QString("%1/%2").arg(service->installPath(), service->areaString());
        data->_patchDir = QString("%1/patch/%2/%3").arg(service->downloadPath(), state->patchVersion(), service->areaString());
        data->_patchFiles = state->patchFiles();
        data->_patchVersion = state->patchVersion();

        data->_filesCount = 0;
        data->_totalFileCount = data->_patchFiles.length();

        data->_skipMode = false;
        data->_progressMod = 0;
        if (data->_totalFileCount > 200) {
          data->_skipMode = true;
          data->_progressMod = static_cast<int>(data->_totalFileCount / 100);
        }

        QtConcurrent::run(this, &GGS::GameDownloader::Behavior::BindiffBehavior::run, data);
      }

      void BindiffBehavior::run(BindiffBehaviorPrivate* data) 
      {
        while(!data->_patchFiles.isEmpty()) {
          QString workingFile = data->_patchFiles.takeFirst();

          if (data->_state->state() != GGS::GameDownloader::ServiceState::Started) {
            data->deleteLater();
            emit this->next(Paused, data->_state);
            return;
          }

          data->_workingFile = workingFile;

          data->_filesCount++;
          if (!data->_skipMode 
            || (data->_filesCount % data->_progressMod) == 0 
            || data->_filesCount == data->_totalFileCount) {
              qint8 progres = static_cast<qint8>(100.0f * static_cast<qreal>(data->_filesCount) / static_cast<qreal>(data->_totalFileCount));
              emit this->totalProgressChanged(data->_state, progres);

              if (data->_totalFileCount > 0) {
                QString msg = QObject::tr("PATCH_APPLY_BINDIFF_STATUS_MESSAGE")
                  .arg(QString::number(data->_filesCount))
                  .arg(QString::number(data->_totalFileCount));
                this->statusMessageChanged(data->_state, msg);
              }
          }

          if (this->apply(workingFile, data)) {
            data->deleteLater();
            return;
          }

          data->_state->setPatchFiles(data->_patchFiles);
        }

        emit this->next(Finished, data->_state);
        data->deleteLater();
      }

      void BindiffBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
      }

      void BindiffBehavior::xdeltaFinished(BindiffBehaviorPrivate* data)
      {
        DEBUG_LOG << data->_workingFile << "finished";

        QString actualFile = data->_workingFile.left(data->_workingFile.size() - QString(".diff").size());
        QString source = QString("%1/%2").arg(data->_rootDir, actualFile);
        QString actualTarget = source + ".new";
        // UNDONE Подумать надо ли тут переименовывать а .old.123.old где 123 рандом
        QString oldSource = source + ".old";
        
        QFile::remove(oldSource);
        QFile::rename(source, oldSource);
        QFile::rename(actualTarget, source);
        QFile::remove(oldSource);
      }


      bool BindiffBehavior::apply(const QString& file, BindiffBehaviorPrivate* data)
      {
        QString actualFile = file.left(file.size() - QString(".diff").size());
        
        QString source = QString("%1/%3").arg(data->_rootDir, actualFile);
        QString patch = QString("%1/%3").arg(data->_patchDir, file);
        QString actualTarget = source + ".new";

        if (!QFile::exists(patch)) {
          DEBUG_LOG << "file not found" << patch;
          emit this->next(CRCFailed, data->_state);
          return true;
        }

        using namespace GGS::GameDownloader::XdeltaWrapper;
        XdeltaDecoder::Result result = data->_decoder.applySync(source, patch, actualTarget);     

        switch(result) {
          case XdeltaDecoder::CrcFailed: {
            WARNING_LOG << "Xdelta source crc failed" << source;
            emit this->next(CRCFailed, data->_state); 
            return true;
          }
          case XdeltaDecoder::Success: this->xdeltaFinished(data); break;
          case XdeltaDecoder::Failed:
          default: {
            WARNING_LOG << "Xdelta internal failed" << source;
            emit this->failed(data->_state); return true;
          }
        }

        return false;
      }

    }
  }
}