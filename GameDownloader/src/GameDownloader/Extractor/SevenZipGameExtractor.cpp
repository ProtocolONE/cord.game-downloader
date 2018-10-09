/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Extractor/SevenZipGameExtractor.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Extractor/UpdateInfoGetterResultEventLoopKiller.h>
#include <GameDownloader/Common/FileUtils.h>

#include <UpdateSystem/Extractor/SevenZipExtractor.h>
#include <UpdateSystem/Compressor/ZevenZipCompressor.h>
#include <UpdateSystem/UpdateInfoGetter>
#include <UpdateSystem/UpdateInfoContainer>
#include <UpdateSystem/UpdateFileInfo>

#include <UpdateSystem/Downloader/downloadmanager>
#include <UpdateSystem/Downloader/DynamicRetryTimeout>
#include <UpdateSystem/Downloader/RetryFileDownloader>
#include <UpdateSystem/Hasher/Md5FileHasher>

#include <Settings/Settings>
#include <Core/Service>

#include <QtCore/QDebug>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>

namespace GGS {
  namespace GameDownloader {
    namespace Extractor {

      SevenZipGameExtractor::SevenZipGameExtractor(QObject *parent)
        : GGS::GameDownloader::ExtractorBase("D9E40EE5-806F-4B7D-8D5C-B6A4BF0110E9", parent)
      {
      }

      SevenZipGameExtractor::~SevenZipGameExtractor()
      {
      }
      
      QStringList SevenZipGameExtractor::deserialize(QByteArray serialized)
      {
        QStringList result;
        QDataStream in(&serialized, QIODevice::ReadOnly);
        in >> result;
        return result;
      }

      QByteArray SevenZipGameExtractor::serialize(QStringList stringList)
      {
        QByteArray byteArray;
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        out << stringList;
        return byteArray;
      }

      void SevenZipGameExtractor::compress(GGS::GameDownloader::ServiceState* state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->compressPaused(state);
          return;
        }
        
        const GGS::Core::Service *service = state->service();
        emit this->compressProgressChanged(state, 0, 0, 0);

        using namespace GGS::Compressor;
        SevenZipCompressor compressor;
        compressor.setCompressionLevel(GGS::Compressor::SevenZipCompressor::Low);

        if (QThread::idealThreadCount() > 1)
          compressor.setNumThreads(2);

        QStringList files = state->packingFiles();

        QString sourceDirectory = QString("%1/%2").arg(service->installPath(), service->areaString());
        QString distrDirectory = QString("%1/%2").arg(service->downloadPath(), service->areaString());

        qint64 filesCount = 0;
        qint64 totalFilesCount = files.length();

        bool skipedProgres = false;
        int progressMod = 0;
        if (totalFilesCount > 200) {
          skipedProgres = true;
          progressMod = static_cast<int>(totalFilesCount / 100);
        }

        Q_FOREACH(QString fileName, files) {
          if (state->state() != ServiceState::Started) {
            emit this->compressPaused(state);
            return;
          }

          filesCount++;
          if (!skipedProgres || ((filesCount % progressMod) == 0 || filesCount == totalFilesCount)) {
            emit this->compressProgressChanged(
              state, 
              static_cast<qint8>(100.0f * static_cast<qreal>(filesCount) / static_cast<qreal>(totalFilesCount)), 
              filesCount, 
              totalFilesCount);
          } 

          QString archiveFile = QString("%1/%2.7z").arg(distrDirectory, fileName);
          QString sourceFile = QString("%1/%2").arg(sourceDirectory, fileName);
          QFile::remove(archiveFile); // TODO обсудить

          Common::FileUtils::createDirectoryIfNotExist(archiveFile);

          DEBUG_LOG << "compress " << sourceFile;
          CompressorInterface::CompressionResult result = compressor.compressFile(sourceFile, archiveFile);

          if (result != CompressorInterface::NoError) {
            emit this->compressFailed(state);
            return;
          }

          files.removeOne(fileName);
          state->setPackingFiles(files);
        }

        emit this->compressFinished(state);
      }

      void SevenZipGameExtractor::extract(GGS::GameDownloader::ServiceState* state, StartType startType)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(state->service());

        if (state->state() != ServiceState::Started) {
          emit this->extractPaused(state);
          return;
        }

        const GGS::Core::Service *service = state->service();
        emit this->extractionProgressChanged(state, 0, 0, 0);

        using namespace GGS::UpdateSystem;
        QHash<QString, UpdateFileInfo> onlineInfo;
        if (!this->getUpdateInfo(state, onlineInfo)) {
          emit this->extractFailed(state);
          return;
        }

        if (state->state() != ServiceState::Started) {
          emit this->extractPaused(state);
          return;
        }

        QString extractionDirectory = QString("%1/%2/").arg(service->installPath(), service->areaString());
        QHash<QString, QString> existingFilesHash;
        this->getFilesInDirectory(extractionDirectory, existingFilesHash);

        QHash<QString, UpdateFileInfo> savedInfo;
        this->loadUpdateInfo(state, savedInfo);
        
        GGS::Hasher::Md5FileHasher hasher;
        qint64 totalFilesCount = onlineInfo.count();
        bool skipedProgres = false;
        int progressMod = 0;
        if (totalFilesCount > 200) {
          skipedProgres = true;
          progressMod = static_cast<int>(totalFilesCount / 100);
        }

        qint64 checkedFilesCount = 0;

        QHash<QString, UpdateFileInfo> filesToExtraction;
        Q_FOREACH(QString relativePath, onlineInfo.keys()) {
          if (state->state() != ServiceState::Started) {
            emit this->extractPaused(state);
            return;
          }

          checkedFilesCount++;
          if (!skipedProgres || ((checkedFilesCount % progressMod) == 0 || checkedFilesCount == totalFilesCount)) {
            emit this->extractionProgressChanged(state, 
              static_cast<qint8>(15.0f * static_cast<qreal>(checkedFilesCount) / static_cast<qreal>(totalFilesCount)), 0, 0);
          } 

          if (!existingFilesHash.contains(relativePath.toLower())) {
            filesToExtraction[relativePath] = onlineInfo[relativePath];
            continue;
          }

          if (!savedInfo.contains(relativePath)) {
            filesToExtraction[relativePath] = onlineInfo[relativePath];
            continue;
          }

          if (onlineInfo[relativePath].forceCheck() || startType == GGS::GameDownloader::Recheck) {
            QString filePath = QString("%1%2").arg(extractionDirectory, relativePath);
            if (hasher.getFileHash(filePath) != onlineInfo[relativePath].hash()) {
              filesToExtraction[relativePath] = onlineInfo[relativePath];
              savedInfo.remove(relativePath);
            }
          } else if (savedInfo[relativePath].hash() != onlineInfo[relativePath].hash()) {
            filesToExtraction[relativePath] = onlineInfo[relativePath];
            savedInfo.remove(relativePath);
          }
        }

        if (filesToExtraction.isEmpty()) {
          emit this->extractFinished(state);
          return;
        }
          
        this->extractFiles(state, filesToExtraction, extractionDirectory, savedInfo);
      }

      bool SevenZipGameExtractor::getUpdateInfo(GGS::GameDownloader::ServiceState* state, 
        QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash)
      {
        using namespace GGS::UpdateSystem;
        using namespace GGS::Extractor;
        using namespace GGS::Downloader;

        const GGS::Core::Service *service = state->service();

        DynamicRetryTimeout dynamicRetryTimeout;
        dynamicRetryTimeout << 5000 << 10000 << 15000 << 30000;

        RetryFileDownloader retryDownloader;
        retryDownloader.setMaxRetry(DynamicRetryTimeout::InfinityRetryCount);
        retryDownloader.setTimeout(&dynamicRetryTimeout);

        DownloadManager downloader;
        retryDownloader.setDownloader(&downloader);

        QString downloadUpdateCrcDirectory = QString("%1/%2/").arg(service->downloadPath(), service->areaString());
        downloadUpdateCrcDirectory = QDir::cleanPath(downloadUpdateCrcDirectory);
        QUrl updateUrl = service->torrentUrlWithArea().resolved(QUrl("./update.crc.7z"));

        UpdateInfoGetter infoGetter;
        SevenZipExtactor extractor;
        infoGetter.setExtractor(&extractor);
        infoGetter.setDownloader(&retryDownloader);
        infoGetter.setCurrentDir(downloadUpdateCrcDirectory);
        infoGetter.setUpdateFileName("update.crc");
        infoGetter.setUpdateCrcUrl(updateUrl.toString());

        UpdateInfoGetterResultEventLoopKiller loopKiller;
        infoGetter.setResultCallback(&loopKiller);

        QEventLoop loop;
        loopKiller.setEventLoop(&loop);

        QTimer::singleShot(0, &infoGetter, SLOT(start()));
        loop.exec();

        UpdateInfoGetterResults result = loopKiller.result();
        if (result != UpdateInfoGetterResults::NoError) {
          CRITICAL_LOG << "UpdateInfoGetter error: " << result;
          return false;
        }

        UpdateInfoContainer *container = infoGetter.updateInfo();
        QList<UpdateFileInfo*>* files = container->getFiles();
        QList<UpdateFileInfo*>::const_iterator it = files->begin();
        QList<UpdateFileInfo*>::const_iterator end = files->end();

        for (; it != end; ++it) {
          resultHash[QDir::cleanPath((*it)->relativePath())] = **it;
        }

        return true;
      }

      void SevenZipGameExtractor::loadUpdateInfo(GGS::GameDownloader::ServiceState* state,
        QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash)
      {
        const GGS::Core::Service *service = state->service();

        GGS::Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup("SevenZipGameExtractor");
        
        QByteArray byteArray = settings.value(service->id(), QByteArray()).toByteArray();
        if (byteArray.isEmpty())
          return;

        QByteArray uncompressed = qUncompress(byteArray);
        QDataStream stream(&uncompressed, QIODevice::ReadOnly);
        stream >> resultHash;
      }

      void SevenZipGameExtractor::saveUpdateInfo(
        GGS::GameDownloader::ServiceState* state, 
        const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash)
      {
        const GGS::Core::Service *service = state->service();

        QByteArray byteArray;
        QDataStream stream(&byteArray, QIODevice::WriteOnly);
        stream << resultHash;

        QByteArray compressed = qCompress(byteArray, 9);

        GGS::Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup("SevenZipGameExtractor");
        settings.setValue(service->id(), compressed);
      }

      void SevenZipGameExtractor::getFilesInDirectory(const QString &directory, QHash<QString, QString> &result)
      {
        QString tmp = QString("%1/").arg(directory);
        tmp = QDir::cleanPath(tmp);
        int len = tmp.length() + 1;
        QDir dir(tmp);

        QDirIterator it(dir, QDirIterator::Subdirectories);
        while (it.hasNext()) {
          QString file = it.next();
          if (!it.fileInfo().isDir()) {
            result[file.right(file.length() - len).toLower()] = file;
          }
        }
      }

      void SevenZipGameExtractor::extractFiles(
        GGS::GameDownloader::ServiceState* state, 
        const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &filesToExtraction, 
        const QString& extractionDirectory,
        QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &savedInfo)
      {
        const GGS::Core::Service *service = state->service();

        qint64 totalFilesCount = filesToExtraction.count();
        emit this->extractionProgressChanged(state, 15, 0, totalFilesCount);

        using namespace GGS::Extractor;
        SevenZipExtactor extractor;

        qint64 timeOfLastSaveUpdateInfo = QDateTime::currentMSecsSinceEpoch();
        qint64 extractedFilesCount = 0;
        QString archiveDirectory = QString("%1/%2/").arg(service->downloadPath(), service->areaString());

        bool skipedProgres = false;
        int progressMod = 0;
        if (totalFilesCount > 200) {
          skipedProgres = true;
          progressMod = static_cast<int>(totalFilesCount / 100);
        }

        Q_FOREACH(QString relativePath, filesToExtraction.keys()) {
          if (state->state() != ServiceState::Started) {
            emit this->extractPaused(state);
            return;
          }

          QString archivePath = QString("%1%2.7z").arg(archiveDirectory, relativePath);
          QString targetFilePath = QString("%1%2").arg(extractionDirectory, relativePath);
          QString targetDirectory = Common::FileUtils::createDirectoryIfNotExist(targetFilePath);
          DEBUG_LOG << "extracting " << relativePath;
          GGS::Extractor::ExtractionResult result = extractor.extract(archivePath, targetDirectory);
          if (result != ExtractionResult::NoError) {
            CRITICAL_LOG << "Extraction error: " << result;
            emit this->extractFailed(state);
            return;
          }
        
          extractedFilesCount++;

          if (!skipedProgres || ((extractedFilesCount % progressMod) == 0 || extractedFilesCount == totalFilesCount)) {
            qint8 progress = 15 + static_cast<qint8>(85 * (static_cast<qreal>(extractedFilesCount) / static_cast<qreal>(totalFilesCount)));
            emit this->extractionProgressChanged(state, progress, extractedFilesCount, totalFilesCount);
          } 

          savedInfo[relativePath] = filesToExtraction[relativePath];
          
          qint64 diff = QDateTime::currentMSecsSinceEpoch() - timeOfLastSaveUpdateInfo;
          if (diff < 0 || diff > 10000) {
            this->saveUpdateInfo(state, savedInfo);
            timeOfLastSaveUpdateInfo = QDateTime::currentMSecsSinceEpoch();
          }
        }

        this->saveUpdateInfo(state, savedInfo);
        emit this->extractFinished(state);
      }

      void SevenZipGameExtractor::setAllUnpacked(ServiceState* state)
      {
        Q_CHECK_PTR(state);

        using namespace GGS::UpdateSystem;
        QHash<QString, UpdateFileInfo> onlineInfo;
        if (!this->getUpdateInfo(state, onlineInfo)) {
          emit this->unpackStateSaveFailed(state);
          return;
        }

        this->saveUpdateInfo(state, onlineInfo);
        emit this->unpackStateSaved(state);
      }

    }
  }
}