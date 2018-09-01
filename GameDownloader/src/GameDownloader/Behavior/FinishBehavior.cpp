#include <GameDownloader/Behavior/FinishBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <Core/Service.h>

#include <QtCore/QDir>
#include <Qtcore/QFile>

namespace P1 {
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

      void FinishBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        state->setDownloadSuccess(true);
        QString patchDir = QString("%1/patch").arg(state->service()->downloadPath());
        this->removeDir(patchDir);

        emit this->finished(state);
      }

      void FinishBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
      }

    }
  }
}
