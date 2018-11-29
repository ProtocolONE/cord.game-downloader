#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/ExtractorBase.h>

#include <UpdateSystem/UpdateFileInfo.h>
#include <UpdateSystem/Extractor/ExtractorInterface.h>
#include <UpdateSystem/Compressor/CompressorInferface.h>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    namespace Extractor {
      class DOWNLOADSERVICE_EXPORT GameExtractor : public P1::GameDownloader::ExtractorBase
      {
        Q_OBJECT
      public:
        explicit GameExtractor(const QString &extractorId, const QString &fileExtention, QObject *parent = 0);
        ~GameExtractor();

      protected:
        void extract(P1::Extractor::ExtractorInterface* extractor, P1::GameDownloader::ServiceState* state, StartType startType);
        void compress(P1::Compressor::CompressorInterface* compressor, P1::GameDownloader::ServiceState* state);
        void setAllUnpacked(P1::Extractor::ExtractorInterface* extractor, P1::GameDownloader::ServiceState* state);

      private:
        bool getUpdateInfo(P1::Extractor::ExtractorInterface* extractor, P1::GameDownloader::ServiceState* state, QHash<QString, P1::UpdateSystem::UpdateFileInfo> &resultHash);
        void loadUpdateInfo(P1::GameDownloader::ServiceState* state, QHash<QString, P1::UpdateSystem::UpdateFileInfo> &resultHash);
        void saveUpdateInfo(P1::GameDownloader::ServiceState* state, const QHash<QString, P1::UpdateSystem::UpdateFileInfo> &resultHash);
        void getFilesInDirectory(const QString &directory, QHash<QString, QString> &result);

        void extractFiles(
          P1::Extractor::ExtractorInterface* extractor, 
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
