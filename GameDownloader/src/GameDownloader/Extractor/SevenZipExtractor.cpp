#include <GameDownloader/Extractor/SevenZipExtractor.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Extractor/UpdateInfoGetterResultEventLoopKiller.h>
#include <GameDownloader/Common/FileUtils.h>

#include <UpdateSystem/Extractor/SevenZipExtractor.h>
#include <UpdateSystem/Compressor/SevenZipCompressor.h>

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

      SevenZipExtractor::SevenZipExtractor(QObject *parent)
        : GameExtractor("D9E40EE5-806F-4B7D-8D5C-B6A4BF0110E9", ".7z", parent)
      {
      }

      void SevenZipExtractor::extract(P1::GameDownloader::ServiceState* state, StartType startType)
      {
        P1::Extractor::SevenZipExtractor extractor;
        GameExtractor::extract(&extractor, state, startType);
      }

      void SevenZipExtractor::compress(P1::GameDownloader::ServiceState* state)
      {
        P1::Compressor::SevenZipCompressor compressor;
        compressor.setCompressionLevel(P1::Compressor::SevenZipCompressor::Low);

        if (QThread::idealThreadCount() > 1)
          compressor.setNumThreads(2);

        GameExtractor::compress(&compressor, state);
      }

      void SevenZipExtractor::setAllUnpacked(ServiceState* state)
      {
        P1::Extractor::SevenZipExtractor extractor;
        GameExtractor::setAllUnpacked(&extractor, state);
      }
    }
  }
}