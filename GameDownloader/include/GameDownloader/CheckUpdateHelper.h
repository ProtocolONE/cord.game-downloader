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

#include <UpdateSystem/Downloader/MultiDownloadResultInterface>

#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>

namespace GGS {
  namespace Extractor {
    class SevenZipExtactor;
  }

  namespace GameDownloader {
    class ServiceState;

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
      virtual void fileDownloaded(const QString& filePath) override;
      virtual void downloadResult(bool isError, GGS::Downloader::DownloadResults error) override;
      virtual void downloadProgress(quint64 downloadSize, quint64 currentFileDownloadSize, quint64 currestFileSize) override;
      virtual void downloadWarning(bool isError, GGS::Downloader::DownloadResults error) override;

      void setMaxHeadRequestRetry(int count);

      static QString getTorrentPath(GGS::GameDownloader::ServiceState *state);

    public slots:
      void startCheck(GGS::GameDownloader::ServiceState *state, CheckUpdateType checkUpdateType);
      static void saveLastModifiedDate(const QString& date, GGS::GameDownloader::ServiceState *state);
      static void saveTorrenthash(const QString& date, GGS::GameDownloader::ServiceState *state);

    signals:
      void result(GGS::GameDownloader::ServiceState *state, bool isUpdate);
      void error(GGS::GameDownloader::ServiceState *state);
      void checkUpdateProgressChanged(GGS::GameDownloader::ServiceState *state, quint8 progress);

    private slots:
      void slotError(QNetworkReply::NetworkError error);
      void slotReplyDownloadFinished();

    private:
      QString getTorrentUrlWithoutArchiveExtension();
      QUrl getTorrentUrlWithArchiveExtension();
      void startDownloadTorrent();

      QString loadLastModifiedDate() const;
      QString torrentHash() const;

      QString _lastModified;
      QNetworkAccessManager *_manager;
      GGS::GameDownloader::ServiceState *_state;
      GGS::Extractor::SevenZipExtactor *_extractor;
      int _headRequestRetryCount;
      int _maxHeadRequestRetryCount;
      CheckUpdateType _checkUpdateType;
    };
  }
}
