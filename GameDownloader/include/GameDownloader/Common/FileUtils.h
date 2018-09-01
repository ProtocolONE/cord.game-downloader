#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <QtCore/QString>

namespace P1 {
  namespace GameDownloader {
    namespace Common {

      class FileUtils
      {
      public:
        static QString createDirectoryIfNotExist(const QString &targetFilePath);

      private:
        FileUtils();
        ~FileUtils();
      };

    }
  }
}
