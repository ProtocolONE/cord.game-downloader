#pragma once

#include <GameDownloader/GameDownloader_global.h>

#include <UpdateSystem/Downloader/MultiDownloadResultInterface.h>

#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>

namespace P1 {
  namespace Extractor {
    class SevenZipExtactor;
  }

  namespace GameDownloader {
    class ServiceState;

    class DOWNLOADSERVICE_EXPORT CheckUpdateHelper : public QObject,
      public P1::Downloader::MultiFileDownloadResultInterface
    {
      Q_OBJECT
      Q_ENUMS(P1::GameDownloader::CheckUpdateHelper::CheckUpdateType)

    public:
      enum CheckUpdateType {
        Normal = 0,
        ForceDownloadTorrent = 1
      };

      explicit CheckUpdateHelper(QObject *parent = 0);
      ~CheckUpdateHelper();

      //MultiFileDownloadResultInterface
      virtual void fileDownloaded(const QString& filePath) override;
      virtual void downloadResult(bool isError, P1::Downloader::DownloadResults error) override;
      virtual void downloadProgress(quint64 downloadSize, quint64 currentFileDownloadSize, quint64 currestFileSize) override;
      virtual void downloadWarning(bool isError, P1::Downloader::DownloadResults error) override;

      void setMaxHeadRequestRetry(int count);

      static QString getTorrentPath(P1::GameDownloader::ServiceState *state);

    public slots:
      void startCheck(P1::GameDownloader::ServiceState *state, CheckUpdateType checkUpdateType);
      static void saveLastModifiedDate(const QString& date, P1::GameDownloader::ServiceState *state);
      static void saveTorrenthash(const QString& date, P1::GameDownloader::ServiceState *state);

    signals:
      void result(P1::GameDownloader::ServiceState *state, bool isUpdate);
      void error(P1::GameDownloader::ServiceState *state);
      void checkUpdateProgressChanged(P1::GameDownloader::ServiceState *state, quint8 progress);

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
      P1::GameDownloader::ServiceState *_state;
      P1::Extractor::SevenZipExtactor *_extractor;
      int _headRequestRetryCount;
      int _maxHeadRequestRetryCount;
      CheckUpdateType _checkUpdateType;
    };
  }
}
