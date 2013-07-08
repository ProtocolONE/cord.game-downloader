/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/ExtractorBase.h>
#include <UpdateSystem/UpdateFileInfo>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    namespace Extractor {
      class DOWNLOADSERVICE_EXPORT SevenZipGameExtractor : public GGS::GameDownloader::ExtractorBase
      {
        Q_OBJECT
      public:
        explicit SevenZipGameExtractor(QObject *parent = 0);
        ~SevenZipGameExtractor();

        virtual void extract(GGS::GameDownloader::ServiceState* state, StartType startType) override;
        virtual void compress(GGS::GameDownloader::ServiceState* state) override;
        virtual void setAllUnpacked(GGS::GameDownloader::ServiceState* state) override;

      private:
        bool getUpdateInfo(GGS::GameDownloader::ServiceState* state, QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash);
        void loadUpdateInfo(GGS::GameDownloader::ServiceState* state, QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash);
        void saveUpdateInfo(GGS::GameDownloader::ServiceState* state, const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash);
        void getFilesInDirectory(const QString &directory, QHash<QString, QString> &result);

        void extractFiles(
          GGS::GameDownloader::ServiceState* state, 
          const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &filesToExtraction, 
          const QString& extractionDirectory,
          QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &savedInfo);

        QStringList deserialize(QByteArray serialized);
        QByteArray serialize(QStringList stringList);
      };

    }
  }
}
