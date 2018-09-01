#include <GameDownloader/Common/FileUtils.h>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

namespace P1 {
  namespace GameDownloader {
    namespace Common {

      FileUtils::FileUtils()
      {
      }

      FileUtils::~FileUtils()
      {
      }

      QString FileUtils::createDirectoryIfNotExist(const QString &targetFilePath)
      {
        QFileInfo info(targetFilePath);
        QDir targetPath = info.dir();
        QString path = info.absolutePath();
        if (!targetPath.exists())
          targetPath.mkpath(path);

        return path;
      }

    }
  }
}