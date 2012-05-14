/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_EXTRACTOR_SEVENZIPGAMEEXTRACTOR_H_
#define _GGS_GAMEDOWNLOADER_EXTRACTOR_SEVENZIPGAMEEXTRACTOR_H_

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/ExtractorBase.h>
#include <Core/Service>
#include <UpdateSystem/UpdateFileInfo>

namespace GGS {
  namespace GameDownloader {
    class PauseRequestWatcher;
    namespace Extractor {
      class DOWNLOADSERVICE_EXPORT SevenZipGameExtractor : public GGS::GameDownloader::ExtractorBase
      {
        Q_OBJECT
      public:
        explicit SevenZipGameExtractor(QObject *parent = 0);
        ~SevenZipGameExtractor();

        virtual void extract(const GGS::Core::Service *service, StartType startType);

      private:
        bool getUpdateInfo(const GGS::Core::Service *service, QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash);
        void loadUpdateInfo(const GGS::Core::Service *service, QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash);
        void saveUpdateInfo(const GGS::Core::Service *service, const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash);
        void getFilesInDirectory(const QString &dirictory, QHash<QString, QString> &result);
        QString getServiceAreaString(const GGS::Core::Service *service);
        QString createDirectoryIfNotExist(const QString &targetFilePath);
        void extractFiles(
          const GGS::Core::Service *service, 
          const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &filesToExtraction, 
          const QString& extractionDirectory,
          const PauseRequestWatcher *watcher,
          QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &savedInfo);
      };

    }
  }
}

#endif // _GGS_GAMEDOWNLOADER_EXTRACTOR_SEVENZIPGAMEEXTRACTOR_H_