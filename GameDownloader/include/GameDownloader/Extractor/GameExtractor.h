#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/ExtractorBase.h>
#include <UpdateSystem/UpdateFileInfo.h>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    namespace Extractor {
      class DOWNLOADSERVICE_EXPORT GameExtractor : public P1::GameDownloader::ExtractorBase
      {
        Q_OBJECT
      public:
        explicit GameExtractor(QObject *parent = 0);
        ~GameExtractor();

        virtual void extract(P1::GameDownloader::ServiceState* state, StartType startType) override;
        virtual void compress(P1::GameDownloader::ServiceState* state) override;
        virtual void setAllUnpacked(P1::GameDownloader::ServiceState* state) override;

      private:
        bool getUpdateInfo(P1::GameDownloader::ServiceState* state, QHash<QString, P1::UpdateSystem::UpdateFileInfo> &resultHash);
        void loadUpdateInfo(P1::GameDownloader::ServiceState* state, QHash<QString, P1::UpdateSystem::UpdateFileInfo> &resultHash);
        void saveUpdateInfo(P1::GameDownloader::ServiceState* state, const QHash<QString, P1::UpdateSystem::UpdateFileInfo> &resultHash);
        void getFilesInDirectory(const QString &directory, QHash<QString, QString> &result);

        void extractFiles(
          P1::GameDownloader::ServiceState* state, 
          const QHash<QString, P1::UpdateSystem::UpdateFileInfo> &filesToExtraction, 
          const QString& extractionDirectory,
          QHash<QString, P1::UpdateSystem::UpdateFileInfo> &savedInfo);

        QStringList deserialize(QByteArray serialized);
        QByteArray serialize(QStringList stringList);
      };

    }
  }
}
