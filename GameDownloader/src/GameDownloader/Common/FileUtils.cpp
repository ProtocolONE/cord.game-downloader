/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Common/FileUtils.h>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

namespace GGS {
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