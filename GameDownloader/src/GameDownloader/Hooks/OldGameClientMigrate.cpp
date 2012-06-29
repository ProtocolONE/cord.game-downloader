/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/Hooks/OldGameClientMigrate.h>
#include <GameDownloader/GameDownloadService>
#include <GameDownloader/PauseRequestWatcher>

#include <Core/UI/Message>
#include <Settings/Settings>

#include <QtCore/QDirIterator>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <Windows.h>

using GGS::Core::UI::Message;

namespace GGS {
  namespace GameDownloader {
    namespace Hooks {

      OldGameClientMigrate::OldGameClientMigrate() 
        : HookBase("59D1D11A-9D75-4049-99E1-2BFD0C8C7FED")
      {
        this->_beforeProgressWeight = 5;
      }

      OldGameClientMigrate::~OldGameClientMigrate()
      {
      }

      GGS::GameDownloader::HookBase::HookResult OldGameClientMigrate::beforeDownload(
        GameDownloadService *gameDownloader, const GGS::Core::Service *service)
      {
        Q_CHECK_PTR(gameDownloader);
        Q_CHECK_PTR(service);

        if (!service->isDownloadable())
          return HookBase::Continue;

        if (gameDownloader->isInstalled(service))
          return HookBase::Continue;
        
        if (!this->isFirstRun(service))
          return HookBase::Continue;
        
        QString message = QString::fromUtf8("Приложение обнаружило, что у вас уже есть игра %1. Приложение " \
                                            "может скопировать игру, чтобы вам не пришлось загружать ее заново.")
                                            .arg(service->name());
        
        Message::StandardButtons result = Message::information(
          QString::fromUtf8("Внимание"),
          message, 
          static_cast<Message::StandardButton>(Message::Yes | Message::No));
        
        if (result != Message::Yes) {
          this->saveHookFinished(service);
          return HookBase::Continue;
        }

        if (!this->migrate(gameDownloader, service))
          return HookBase::Abort;

        this->saveHookFinished(service);
        return HookBase::Continue;
      }

      GGS::GameDownloader::HookBase::HookResult OldGameClientMigrate::afterDownload(
        GameDownloadService *gameDownloader, const GGS::Core::Service *service)
      {
        return HookBase::Continue;
      }

      bool OldGameClientMigrate::isFirstRun(const GGS::Core::Service *service)
      {
        GGS::Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup(service->id());
        settings.beginGroup("OldGameClientMigrate");
        return settings.value("migrateFinished", 0).toInt() != 1;
      }

      void OldGameClientMigrate::saveHookFinished(const GGS::Core::Service *service)
      {
        GGS::Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup(service->id());
        settings.beginGroup("OldGameClientMigrate");
        settings.setValue("migrateFinished", 1);
      }

      bool OldGameClientMigrate::migrate(GameDownloadService *gameDownloader, const GGS::Core::Service *service)
      {
        PauseRequestWatcher watcher(service);
        connect(this, SIGNAL(pauseRequest(const GGS::Core::Service*)), 
          &watcher, SLOT(pauseRequestSlot(const GGS::Core::Service*)), Qt::DirectConnection);

        if (gameDownloader->isStoppedOrStopping(service))
          return false;

        QString key = QString("SOFTWARE\\GGS\\GNA\\%1\\").arg(service->id());
        QString paramName = service->hashDownloadPath() ? "PathToInstaller" : "INSTALL_DIRECTORY";
        QString sourceDirectory;
        if (!this->readString(key, paramName, sourceDirectory)) 
          return true;

        QString targetDir = service->downloadPath();
        QString targetDrive = this->getDriveName(targetDir);
        quint64 totalsize = this->getDirectorySize(sourceDirectory);
        if (totalsize > this->getDriveFreeSpace(targetDrive)) {
          QString message = QString::fromUtf8("Недостаточно места на диске %1").arg(targetDrive);
          GGS::Core::UI::Message::warning(QString::fromUtf8("Внимание"), message, GGS::Core::UI::Message::Ok);
          return false;
        }

        QDir dir(targetDir);
        QDirIterator dirIterator(sourceDirectory, QDirIterator::Subdirectories);
        qreal currentSize = 0;
        while (dirIterator.hasNext()) {
          if (watcher.isPaused())
            return false;

          QString file = dirIterator.next();
          if (dirIterator.fileInfo().isDir())
            continue;

          QString relativePath = file.right(file.length() - sourceDirectory.length());
          QString targetPath = QString("%1/%2/%3").arg(targetDir, service->areaString(), relativePath);
          targetPath = QDir::cleanPath(targetPath);
          if (QFile::exists(targetPath))
            QFile::remove(targetPath);

          QFileInfo info(targetPath);
          dir.mkpath(info.absolutePath());

          if (totalsize > 0) {
            currentSize += dirIterator.fileInfo().size();
            qint8 progress = static_cast<qint8>(100 * (currentSize / totalsize));
            emit this->beforeProgressChanged(service->id(), this->_hookId, progress);
          }
          
          if (!QFile::copy(file, targetPath)) {
            CRITICAL_LOG << "Can't migrate file from old GNA. Source: " << file << "Destination" << targetPath;
            continue;
          }

          
        }

        return true;
      }

      bool OldGameClientMigrate::readString(const QString& key, const QString& paramName, QString& result)
      {
        wchar_t keyArray[MAX_PATH] = {0};
        key.toWCharArray(keyArray);
        HKEY hkey;
        DWORD res = RegOpenKeyExW(HKEY_LOCAL_MACHINE, keyArray, 0, KEY_WOW64_64KEY | KEY_READ, &hkey);
        if (res != ERROR_SUCCESS) {
          return false;
        }
        
        wchar_t paramNameArray[MAX_PATH] = {0};
        paramName.toWCharArray(paramNameArray);

        wchar_t resultArray[MAX_PATH + 1] = {0};
        DWORD size = MAX_PATH;
        DWORD type = REG_SZ;
        res = RegQueryValueExW(hkey, paramNameArray, 0, &type, reinterpret_cast<LPBYTE>(resultArray), &size);
        if (size > 32000) {
          DEBUG_LOG << "Path is too long: " << size;
          RegCloseKey(hkey);
          return false;
        }

        if (res == ERROR_MORE_DATA) {
          // Можно обработать, но QDirIterator не работает с такими длинными путями тоже.
          CRITICAL_LOG << "Source path is too long" << size;
          return false;
        }
        
        RegCloseKey(hkey);

        if (res == ERROR_SUCCESS) {
          result = QString::fromWCharArray(resultArray);
          return true;
        }
        
        return false;
      }

      quint64 OldGameClientMigrate::getDirectorySize(const QString& directory)
      {
        quint64 result = 0;
        QDirIterator dirIterator(directory, QDirIterator::Subdirectories);
        while (dirIterator.hasNext()) {
          QString file = dirIterator.next();
          if (dirIterator.fileInfo().isDir())
            continue;

          result += dirIterator.fileInfo().size();
        }

        return result;
      }

      quint64 OldGameClientMigrate::getDriveFreeSpace(const QString& drive)
      {
        Q_ASSERT(drive.size() < MAX_PATH);
        wchar_t tmp[MAX_PATH] = {0};
        drive.toWCharArray(tmp);

        ULARGE_INTEGER freeBytesToCaller;
        freeBytesToCaller.QuadPart = 0L;

        if (!GetDiskFreeSpaceEx(tmp, &freeBytesToCaller, NULL, NULL)) {
          DEBUG_LOG << "ERROR: Call to GetDiskFreeSpaceEx() failed.";
          return 0;
        }
        
        return freeBytesToCaller.QuadPart;
      }

      QString OldGameClientMigrate::getDriveName(const QString& path)
      {
        int index = path.indexOf(":");
        if (index == -1)
          return QString();

        return path.left(index + 1);
      }

    }
  }
}