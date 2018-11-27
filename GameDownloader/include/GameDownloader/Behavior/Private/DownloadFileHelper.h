#pragma once

#include <UpdateSystem/Downloader/MultiDownloadResultInterface.h>

#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>

namespace P1 {
  namespace Extractor {
  class ExtractorInterface;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DownloadFileHelper : public QObject,
        public P1::Downloader::MultiFileDownloadResultInterface
      {
        Q_OBJECT
      public:
        explicit DownloadFileHelper(ServiceState *state, QObject *parent = 0);
        ~DownloadFileHelper();

        void download(const QString& url, const QString& path);

        virtual void fileDownloaded(const QString& filePath) override;
        virtual void downloadResult(bool isError, P1::Downloader::DownloadResults error) override;
        virtual void downloadProgress(quint64 downloadSize, quint64 currentFileDownloadSize, quint64 currestFileSize) override;
        virtual void downloadWarning(bool isError, P1::Downloader::DownloadResults error) override;

      signals:
        void finished(P1::GameDownloader::ServiceState *state);
        void error(P1::GameDownloader::ServiceState *state);
        void progressChanged(const QString& serviceId, quint8 progress);

      private:
        QNetworkAccessManager *_manager;
        P1::GameDownloader::ServiceState *_state;
        P1::Extractor::ExtractorInterface *_extractor;
        QString _url;

      };

    }
  }
}