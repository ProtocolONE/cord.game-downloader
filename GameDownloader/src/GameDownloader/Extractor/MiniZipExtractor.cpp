#include <GameDownloader/Extractor/MiniZipExtractor.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Extractor/UpdateInfoGetterResultEventLoopKiller.h>
#include <GameDownloader/Common/FileUtils.h>

#include <UpdateSystem/Extractor/MiniZipExtractor.h>
#include <UpdateSystem/Compressor/MiniZipCompressor.h>

#include <UpdateSystem/UpdateInfoGetter.h>
#include <UpdateSystem/UpdateInfoContainer.h>
#include <UpdateSystem/UpdateFileInfo.h>

#include <UpdateSystem/Downloader/downloadmanager.h>
#include <UpdateSystem/Downloader/DynamicRetryTimeout.h>
#include <UpdateSystem/Downloader/RetryFileDownloader.h>
#include <UpdateSystem/Hasher/Md5FileHasher.h>

#include <Settings/Settings.h>
#include <Core/Service.h>

#include <QtCore/QDebug>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>
#include <QtCore/QDataStream>

namespace P1 {
  namespace GameDownloader {
    namespace Extractor {

      MiniZipExtractor::MiniZipExtractor(QObject *parent)
        : GameExtractor("72DC8A5A-2A53-436C-88CC-4C553226290D", ".zip", parent)
      {
      }

      void MiniZipExtractor::extract(P1::GameDownloader::ServiceState* state, StartType startType)
      {
        P1::Extractor::MiniZipExtractor extractor;
        GameExtractor::extract(&extractor, state, startType);
      }

      void MiniZipExtractor::compress(P1::GameDownloader::ServiceState* state)
      {
        P1::Compressor::MiniZipCompressor compressor;
        compressor.setCompressionLevel(P1::Compressor::MiniZipCompressor::Normal);

        if (QThread::idealThreadCount() > 1)
          compressor.setNumThreads(2);

        GameExtractor::compress(&compressor, state);
      }

      void MiniZipExtractor::setAllUnpacked(ServiceState* state)
      {
        P1::Extractor::MiniZipExtractor extractor;
        GameExtractor::setAllUnpacked(&extractor, state);
      }
    }
  }
}