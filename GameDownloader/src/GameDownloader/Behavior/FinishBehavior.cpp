/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/FinishBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <Core/Service>
#include <QtCore/QDir>
#include <Qtcore/QFile>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      FinishBehavior::FinishBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      FinishBehavior::~FinishBehavior()
      {
      }

      bool FinishBehavior::removeDir(const QString &dirName)
      {
        bool result = true;
        QDir dir(dirName);

        QDir::Filters filter = QDir::NoDotAndDotDot | QDir::System | QDir::Hidden | QDir::AllDirs | QDir::Files;
        if (dir.exists(dirName)) {
          Q_FOREACH(QFileInfo info, dir.entryInfoList(filter, QDir::DirsFirst)) {
            result = info.isDir() ? removeDir(info.absoluteFilePath()) : QFile::remove(info.absoluteFilePath());

            if (!result)
              return result;
          }
        }

        result = dir.rmdir(dirName);
        return result;
      }

      void FinishBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        QString patchDir = QString("%1/patch").arg(state->service()->downloadPath());
        this->removeDir(patchDir);

        emit this->finished(state);
      }

      void FinishBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
      }

    }
  }
}
