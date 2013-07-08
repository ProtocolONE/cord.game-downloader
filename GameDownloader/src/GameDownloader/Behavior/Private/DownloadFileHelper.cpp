/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/private/DownloadFileHelper.h>
#include <GameDownloader/ServiceState.h>

#include <UpdateSystem/Downloader/downloadmanager>
#include <UpdateSystem/Downloader/DynamicRetryTimeout>
#include <UpdateSystem/Downloader/RetryFileDownloader>
#include <UpdateSystem/Downloader/MultiFileDownloader>
#include <UpdateSystem/Downloader/MultiFileDownloaderWithExtracter>
#include <UpdateSystem/Extractor/SevenZipExtractor>

#include <Core/Service>

using namespace GGS::Downloader;
using namespace GGS::Extractor;
using namespace GGS::Core;

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      DownloadFileHelper::DownloadFileHelper(ServiceState *state, QObject *parent)
        : QObject(parent)
        , _state(state)
        , _manager(new QNetworkAccessManager(this))
        , _extractor(new SevenZipExtactor(this))
      {
      }

      DownloadFileHelper::~DownloadFileHelper()
      {
      }

      void DownloadFileHelper::download(const QString& url, const QString& path)
      {
        this->_url = url;

        DynamicRetryTimeout* dynamicRetryTimeout = new DynamicRetryTimeout(this);
        *dynamicRetryTimeout << 5000 << 10000 << 15000 << 30000;

        RetryFileDownloader* retryDownloader = new RetryFileDownloader(this);
        retryDownloader->setMaxRetry(5);
        retryDownloader->setTimeout(dynamicRetryTimeout);

        DownloadManager* downloader = new DownloadManager(this);     
        retryDownloader->setDownloader(downloader);

        MultiFileDownloader* multi = new MultiFileDownloader(this);
        multi->setDownloader(retryDownloader);

        MultiFileDownloaderWithExtracter* multiExtractor = new MultiFileDownloaderWithExtracter(this);
        multiExtractor->setExtractor(this->_extractor);
        multiExtractor->setMultiDownloader(multi);
        multiExtractor->setResultCallback(this);

        multiExtractor->addFile(url, path);
        multiExtractor->start();
      }

      void DownloadFileHelper::fileDownloaded(const QString& filePath)
      {
      }

      void DownloadFileHelper::downloadResult(bool isError, GGS::Downloader::DownloadResults error)
      {
        if (isError) {
          WARNING_LOG << "Download error " << this->_state->id()
            << "for url " << this->_url
            << "error code" << error;

          emit this->error(this->_state);
          return;
        } 

        emit this->progressChanged(this->_state->id(), 100);
        emit this->finished(this->_state);
      }

      void DownloadFileHelper::downloadProgress(quint64 downloadSize, quint64 currentFileDownloadSize, quint64 currestFileSize)
      {
        if (currestFileSize <= 0) 
          return;

        qreal progress = 100.0f * (static_cast<qreal>(currentFileDownloadSize) / currestFileSize);
        if (progress < 0)
          progress = 0;

        if (progress > 100)
          progress = 100;

        emit this->progressChanged(this->_state->id(), static_cast<quint8>(progress));
      }

      void DownloadFileHelper::downloadWarning(bool isError, GGS::Downloader::DownloadResults error)
      {
        WARNING_LOG << "Download warning " << this->_state->id()
          << "for url " << this->_url
          << "error code" << error;
      }

    }
  }
}