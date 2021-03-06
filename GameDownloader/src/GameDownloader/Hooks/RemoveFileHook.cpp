
#include <GameDownloader/Hooks/RemoveFileHook.h>
#include <GameDownloader/GameDownloadService.h>

#include <Core/Service.h>
#include <Settings/Settings.h>

#include <UpdateSystem/Hasher/Md5FileHasher.h>

#include <QtCore/QFileInfo>
#include <QtXml/QDomDocument>

namespace P1 {
  namespace GameDownloader {
    namespace Hooks {

      RemoveFileHook::RemoveFileHook(QObject *parent /*= 0*/)
        : HookBase("B963B92F-17D5-4DA3-A5C0-942776CE680A", parent)
      {
      }

      RemoveFileHook::~RemoveFileHook()
      {
      }

      P1::GameDownloader::HookBase::HookResult RemoveFileHook::beforeDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        // nothing to do, really
        return P1::GameDownloader::HookBase::Continue;
      }

      P1::GameDownloader::HookBase::HookResult RemoveFileHook::afterDownload(GameDownloadService *gameDownloader, ServiceState *state)
      {
        bool canSaveInfo = true;

        const P1::Core::Service *service = state->service();

        QString gameRoot = QString("%1/%2").arg(service->installPath(), service->areaString());
        QString removeFilePath = QString("%1/%2/Dependency/%3").arg(service->installPath(), service->areaString(), "removeFile.xml");

        QString downloadRoot;
        
        // INFO Это проверка на 7-zip распаковщик.
        bool hasArchive = (state->service()->extractorType() == "D9E40EE5-806F-4B7D-8D5C-B6A4BF0110E9") || 
          (state->service()->extractorType() == "72DC8A5A-2A53-436C-88CC-4C553226290D");
        if (hasArchive)
          downloadRoot = QString("%1/%2").arg(service->downloadPath(), service->areaString());

        StartType startType = state->startType();
        bool forceRecheck = startType == Force || startType == Recheck;

        if (!forceRecheck && !this->shouldDelete(state->id(), removeFilePath))
          return P1::GameDownloader::HookBase::Continue;

        QDomDocument doc;
        QFile file(removeFilePath);
        if (!file.open(QIODevice::ReadOnly))
          return P1::GameDownloader::HookBase::Continue;

        if (!doc.setContent(&file)) {
          file.close();
          return P1::GameDownloader::HookBase::Continue;
        }

        file.close();

        QDomElement fileInfo = doc.firstChildElement("RemoveFileList")
          .firstChildElement("files")
          .firstChildElement("file");

        if (fileInfo.isNull())
          return P1::GameDownloader::HookBase::Continue;

        for (; !fileInfo.isNull(); fileInfo = fileInfo.nextSiblingElement("file")) {
          if (!fileInfo.hasAttribute("path"))
            continue;

          QString relativePath = fileInfo.attribute("path");
          if (relativePath.isEmpty())
            continue;

          QString filePath = QString("%1/%2").arg(gameRoot, relativePath);

          QFileInfo checkFileInfo(filePath);
          if (!checkFileInfo.exists() || !checkFileInfo.isFile())
            continue;

          canSaveInfo &= QFile::remove(filePath);
          
          if (hasArchive) {
            // INFO Удалять желательно но не критично
            QString archiveFilePath;
            if ((state->service()->extractorType() == "D9E40EE5-806F-4B7D-8D5C-B6A4BF0110E9"))
              archiveFilePath = QString("%1/%2.7z").arg(downloadRoot, relativePath);
            else
              archiveFilePath = QString("%1/%2.zip").arg(downloadRoot, relativePath);

            QFile::remove(archiveFilePath);
          }
        }

        if (canSaveInfo)
          this->saveDataInfo(state->id(), removeFilePath);

        return P1::GameDownloader::HookBase::Continue;
      }

      bool RemoveFileHook::shouldDelete(const QString& id, const QString& filePath)
      {
        Hasher::Md5FileHasher hasher;
        QString hash = hasher.getFileHash(filePath);

        Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup("Hooks");
        settings.beginGroup("RemoveFileHook");
        settings.beginGroup(id);
        
        return settings.value("hash", QString()).toString() != hash;
      }

      void RemoveFileHook::saveDataInfo(const QString& id, const QString& filePath)
      {
        Hasher::Md5FileHasher hasher;
        QString hash = hasher.getFileHash(filePath);

        Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup("Hooks");
        settings.beginGroup("RemoveFileHook");
        settings.beginGroup(id);
        settings.setValue("hash", hash, true);
      }

    }

  }
}