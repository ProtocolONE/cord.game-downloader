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
#include <GameDownloader/PauseRequestWatcher.h>
#include <GameDownloader/GameDownloadService.h>
#include <GameDownloader/Extractor/UpdateInfoGetterResultEventLoopKiller.h>

#include <UpdateSystem/UpdateInfoGetter>
#include <UpdateSystem/UpdateInfoContainer>
#include <UpdateSystem/UpdateFileInfo>

#include <UpdateSystem/Downloader/downloadmanager>
#include <UpdateSystem/Downloader/DynamicRetryTimeout>
#include <UpdateSystem/Downloader/RetryFileDownloader>
#include <UpdateSystem/Hasher/Md5FileHasher>

#include <Settings/Settings>

#include <QtCore/QScopedPointer>
#include <QtCore/QEventLoop>
#include <QtCore/QTimer>
#include <QtCore/QDir>
#include <QtCore/QDirIterator>

#define CRITICAL_LOG qCritical() << __FILE__ << __LINE__ << __FUNCTION__

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

      void SevenZipGameExtractor::extract(const GGS::Core::Service *service, StartType startType)
      {
        PauseRequestWatcher watcher(service);
        connect(this, SIGNAL(pauseRequest(const GGS::Core::Service*)), &watcher, SLOT(pauseRequestSlot(const GGS::Core::Service*)), Qt::ConnectionType::QueuedConnection);

        if (this->_gameDownloadService->isStoppedOrStopping(service)) {
          emit this->extractPaused(service);
          return;
        }

        emit this->extractionProgressChanged(service->id(), 0, 0, 0);

        using namespace GGS::UpdateSystem;
        QHash<QString, UpdateFileInfo> onlineInfo;
        if (!this->getUpdateInfo(service, onlineInfo)) {
          emit this->extractFailed(service);
          return;
        }

        if (watcher.isPaused()) {
          emit this->extractPaused(service);
          return;
        }

        QString extractionDirectory = QString("%1/%2/").arg(service->extractionPath(), this->getServiceAreaString(service));
        QHash<QString, QString> existingFilesHash;
        this->getFilesInDirectory(extractionDirectory, existingFilesHash);

        QHash<QString, UpdateFileInfo> savedInfo;
        this->loadUpdateInfo(service, savedInfo);

        QScopedPointer<GGS::Hasher::Md5FileHasher> hasher(new GGS::Hasher::Md5FileHasher());
        
        qreal totalFilesCount = onlineInfo.count();
        qreal checkedFilesCount = 0;

        QHash<QString, UpdateFileInfo> filesToExtraction;
        Q_FOREACH(QString relativePath, onlineInfo.keys()) {
          if (watcher.isPaused()) {
            emit this->extractPaused(service);
            return;
          }

          checkedFilesCount++;
          emit this->extractionProgressChanged(service->id(), static_cast<qint8>(40.0f * checkedFilesCount / totalFilesCount), 0, 0);

          if (!existingFilesHash.contains(relativePath)) {
            filesToExtraction[relativePath] = onlineInfo[relativePath];
            continue;
          }

          if (!savedInfo.contains(relativePath)) {
            filesToExtraction[relativePath] = onlineInfo[relativePath];
            continue;
          }

          if (onlineInfo[relativePath].forceCheck() || startType == StartType::Recheck) {
            QString filePath = QString("%1%2").arg(extractionDirectory, relativePath);
            if (hasher->getFileHash(filePath) != onlineInfo[relativePath].hash()) {
              filesToExtraction[relativePath] = onlineInfo[relativePath];
              savedInfo.remove(relativePath);
            }
          } else if (savedInfo[relativePath].hash() != onlineInfo[relativePath].hash()) {
            filesToExtraction[relativePath] = onlineInfo[relativePath];
            savedInfo.remove(relativePath);
          }

        }

        if (filesToExtraction.isEmpty()) {
          emit this->extractFinished(service);
          return;
        }
          
        this->extractFiles(
          service, 
          filesToExtraction,
          extractionDirectory, 
          &watcher,
          savedInfo);
      }

      bool SevenZipGameExtractor::getUpdateInfo(const GGS::Core::Service * service, QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash)
      {
        using namespace GGS::UpdateSystem;
        using namespace GGS::Extractor;
        using namespace GGS::Downloader;

        QScopedPointer<UpdateInfoGetter> infoGetter(new UpdateInfoGetter());
        QScopedPointer<SevenZipExtactor> extractor(new SevenZipExtactor());
        infoGetter->setExtractor(extractor.data());

        QScopedPointer<DynamicRetryTimeout> dynamicRetryTimeout(new DynamicRetryTimeout());
        dynamicRetryTimeout->addTimeout(5000);
        dynamicRetryTimeout->addTimeout(10000);
        dynamicRetryTimeout->addTimeout(15000);
        dynamicRetryTimeout->addTimeout(30000);

        QScopedPointer<RetryFileDownloader> retryDownloader(new RetryFileDownloader());
        retryDownloader->setMaxRetry(DynamicRetryTimeout::InfinityRetryCount);
        retryDownloader->setTimeout(dynamicRetryTimeout.data());

        QScopedPointer<DownloadManager> downloader(new DownloadManager());
        retryDownloader->setDownloader(downloader.data());
        infoGetter->setDownloader(retryDownloader.data());

        QString downloadUpdateCrcDirectory = QString("%1/%2/").arg(service->downloadPath(), this->getServiceAreaString(service));
        downloadUpdateCrcDirectory = QDir::cleanPath(downloadUpdateCrcDirectory);
        infoGetter->setCurrentDir(downloadUpdateCrcDirectory);
        QUrl updateUrl = service->torrentUrlWithArea().resolved(QUrl("./update.crc.7z"));
        infoGetter->setUpdateFileName("update.crc");
        infoGetter->setUpdateCrcUrl(updateUrl.toString());

        QScopedPointer<UpdateInfoGetterResultEventLoopKiller> loopKiller(new UpdateInfoGetterResultEventLoopKiller());
        infoGetter->setResultCallback(loopKiller.data());

        QEventLoop loop;
        loopKiller->setEventLoop(&loop);

        QTimer::singleShot(0, infoGetter.data(), SLOT(start()));
        loop.exec();

        UpdateInfoGetterResults result = loopKiller->result();
        if (result != UpdateInfoGetterResults::NoError) {
          CRITICAL_LOG << "UpdateInfoGetter error: " << result;
          return false;
        }

        UpdateInfoContainer *container = infoGetter->updateInfo();
        QList<UpdateFileInfo*>* files = container->getFiles();
        QList<UpdateFileInfo*>::iterator it = files->begin();
        QList<UpdateFileInfo*>::iterator end = files->end();

        for (; it != end; ++it) {
          resultHash[QDir::cleanPath((*it)->relativePath())] = **it;
        }

        return true;
      }

      void SevenZipGameExtractor::loadUpdateInfo(const GGS::Core::Service * service, QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash)
      {
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
        const GGS::Core::Service *service, 
        const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &resultHash)
      {
        QByteArray byteArray;
        QDataStream stream(&byteArray, QIODevice::WriteOnly);
        stream << resultHash;

        QByteArray compressed = qCompress(byteArray, 9);

        GGS::Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup("SevenZipGameExtractor");
        settings.setValue(service->id(), compressed);
      }

      void SevenZipGameExtractor::getFilesInDirectory(const QString &dirictory, QHash<QString, QString> &result)
      {
        QString tmp = QString("%1/").arg(dirictory);
        tmp = QDir::cleanPath(tmp);
        int len = tmp.length() + 1;
        QDir dir(tmp);

        QDirIterator it(dir, QDirIterator::Subdirectories);
        while (it.hasNext()) {
          QString file = it.next();
          if (!it.fileInfo().isDir()) {
            result[file.right(file.length() - len)] = file;
          }
        }
      }

      QString SevenZipGameExtractor::getServiceAreaString(const GGS::Core::Service *service)
      {
        switch(service->area()) {
        case GGS::Core::Service::Live:
          return QString("live");
        case GGS::Core::Service::Pts:
          return QString("pts");
        case GGS::Core::Service::Tst:
          return QString("tst");
        default:
          return QString("");
        }
      }

      QString SevenZipGameExtractor::createDirectoryIfNotExist(const QString &targetFilePath)
      {
        QString cleanTargetFilePath = QDir::cleanPath(targetFilePath);
        int lastIndex = cleanTargetFilePath.lastIndexOf('/');
        if(lastIndex == -1) 
          return cleanTargetFilePath;

        cleanTargetFilePath = cleanTargetFilePath.left(lastIndex + 1);
        QDir targetPath(cleanTargetFilePath);
        if(!targetPath.exists(cleanTargetFilePath))
          targetPath.mkpath(cleanTargetFilePath);

        return cleanTargetFilePath;
      }

      void SevenZipGameExtractor::extractFiles(
        const GGS::Core::Service *service, 
        const QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &filesToExtraction, 
        const QString& extractionDirectory,
        const PauseRequestWatcher *watcher,
        QHash<QString, GGS::UpdateSystem::UpdateFileInfo> &savedInfo)
      {
        qint64 totalFilesCount = filesToExtraction.count();
        QString id = service->id();
        emit this->extractionProgressChanged(id, 40, 0, totalFilesCount);

        using namespace GGS::Extractor;
        QScopedPointer<SevenZipExtactor> extractor(new SevenZipExtactor());

        qint64 timeOfLastSaveUpdateInfo = QDateTime::currentMSecsSinceEpoch();
        qint64 extractedFilesCount = 0;
        QString archiveDirectory = QString("%1/%2/").arg(service->downloadPath(), this->getServiceAreaString(service));
        Q_FOREACH(QString relativePath, filesToExtraction.keys()) {
          if (watcher->isPaused()) {
            emit this->extractPaused(service);
            return;
          }

          QString archivePath = QString("%1%2.7z").arg(archiveDirectory, relativePath);
          QString targetFilePath = QString("%1%2").arg(extractionDirectory, relativePath);
          QString targetDirectory = this->createDirectoryIfNotExist(targetFilePath);

          GGS::Extractor::ExtractionResult result = extractor->extract(archivePath, targetDirectory);
          if (result != ExtractionResult::NoError) {
            CRITICAL_LOG << "Extraction error: " << result;
            emit this->extractFailed(service);
            return;
          }
        
          extractedFilesCount++;
          qint8 progress = 40 + static_cast<qint8>(60 * (static_cast<qreal>(extractedFilesCount) / static_cast<qreal>(totalFilesCount)));
          emit this->extractionProgressChanged(id, progress, extractedFilesCount, totalFilesCount);
          savedInfo[relativePath] = filesToExtraction[relativePath];
          
          qint64 diff = QDateTime::currentMSecsSinceEpoch() - timeOfLastSaveUpdateInfo;
          if (diff < 0 || diff > 10000) {
            this->saveUpdateInfo(service, savedInfo);
            timeOfLastSaveUpdateInfo = QDateTime::currentMSecsSinceEpoch();
          }
        }

        this->saveUpdateInfo(service, savedInfo);
        emit this->extractFinished(service);
      }

    }
  }
}