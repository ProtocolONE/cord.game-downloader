/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_HOOKS_CHECKUPDATEHOOK_H_
#define _GGS_GAMEDOWNLOADER_HOOKS_CHECKUPDATEHOOK_H_

#include <Core/Service.h>

#include <GameDownloader/GameDownloader_global.h>

#include <UpdateSystem/Downloader/MultiDownloadResultInterface>
#include <UpdateSystem/Extractor/SevenZipExtractor.h>

#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>

namespace GGS {
  namespace GameDownloader {
    class DOWNLOADSERVICE_EXPORT CheckUpdateHelper : public QObject,
      public GGS::Downloader::MultiFileDownloadResultInterface
    {
      Q_OBJECT
      Q_ENUMS(GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType)

    public:
      enum CheckUpdateType {
        Normal = 0,
        ForceDownloadTorrent = 1
      };

      explicit CheckUpdateHelper(QObject *parent = 0);
      ~CheckUpdateHelper();

      //MultiFileDownloadResultInterface
      virtual void fileDownloaded(const QString& filePath);
      virtual void downloadResult(bool isError, GGS::Downloader::DownloadResults error);
      virtual void downloadProgress(quint64 downloadSize, quint64 currentFileDownloadSize, quint64 currestFileSize);
      virtual void downloadWarning(bool isError, GGS::Downloader::DownloadResults error);

      void setMaxHeadRequestRetry(int count);

    public slots:
      void startCheck(const GGS::Core::Service *service, CheckUpdateType checkUpdateType);

    signals:
      void result(const GGS::Core::Service *service, bool isUpdate);
      void error(const GGS::Core::Service *service);
      void checkUpdateProgressChanged(const QString& serviceId, quint8 progress);

    private slots:
      void slotError(QNetworkReply::NetworkError error);
      void slotReplyDownloadFinished();

    private:
      QString getTorrentUrlWithoutArchiveExtension();
      QString getTorrentUrlWithArchiveExtension();
      QString getTorrentPath();
      QString getServiceAreaString();
      void startDownloadTorrent();

      void saveLastModifiedDate(const QString& date);
      QString loadLastModifiedDate() const;

      QString _lastModified;
      QNetworkAccessManager * _manager;
      const GGS::Core::Service *_service;
      GGS::Extractor::SevenZipExtactor *_extractor;
      int _headRequestRetryCount;
      int _maxHeadRequestRetryCount;
      CheckUpdateType _checkUpdateType;
    };


  }
}

#endif // _GGS_GAMEDOWNLOADER_HOOKS_CHECKUPDATEHOOK_H_