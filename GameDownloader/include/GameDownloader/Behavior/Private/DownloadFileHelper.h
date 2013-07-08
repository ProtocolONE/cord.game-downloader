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

#include <UpdateSystem/Downloader/MultiDownloadResultInterface>

#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>

namespace GGS {
  namespace Extractor {
    class SevenZipExtactor;
  }

  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class DownloadFileHelper : public QObject,
        public GGS::Downloader::MultiFileDownloadResultInterface
      {
        Q_OBJECT
      public:
        explicit DownloadFileHelper(ServiceState *state, QObject *parent = 0);
        ~DownloadFileHelper();

        void download(const QString& url, const QString& path);

        virtual void fileDownloaded(const QString& filePath) override;
        virtual void downloadResult(bool isError, GGS::Downloader::DownloadResults error) override;
        virtual void downloadProgress(quint64 downloadSize, quint64 currentFileDownloadSize, quint64 currestFileSize) override;
        virtual void downloadWarning(bool isError, GGS::Downloader::DownloadResults error) override;

      signals:
        void finished(GGS::GameDownloader::ServiceState *state);
        void error(GGS::GameDownloader::ServiceState *state);
        void progressChanged(const QString& serviceId, quint8 progress);

      private:
        QNetworkAccessManager *_manager;
        GGS::GameDownloader::ServiceState *_state;
        GGS::Extractor::SevenZipExtactor *_extractor;
        QString _url;

      };

    }
  }
}