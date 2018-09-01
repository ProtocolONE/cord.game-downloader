#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/StartType.h>

#include <QtCore/QObject>

namespace P1 {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    class ServiceState;
    class GameDownloadService;
    class DOWNLOADSERVICE_EXPORT ExtractorBase : public QObject
    {
      Q_OBJECT
    public:
      ExtractorBase(const QString &extractorId, QObject *parent = 0);
      virtual ~ExtractorBase();

      virtual void extract(ServiceState* state, StartType startType) = 0;
      virtual void compress(ServiceState* state) = 0;
      virtual void setAllUnpacked(ServiceState* state) = 0;

      const QString &extractorId();

    public slots:
      void pauseRequestSlot(P1::GameDownloader::ServiceState* state);

    signals:
      void pauseRequest(P1::GameDownloader::ServiceState* state);
      void extractFinished(P1::GameDownloader::ServiceState* state);
      void extractPaused(P1::GameDownloader::ServiceState* state);
      void extractFailed(P1::GameDownloader::ServiceState* state);

      void extractionProgressChanged(P1::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total);

      void compressFinished(P1::GameDownloader::ServiceState* state);
      void compressPaused(P1::GameDownloader::ServiceState* state);
      void compressFailed(P1::GameDownloader::ServiceState* state);

      void compressProgressChanged(P1::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total);

      void unpackStateSaved(P1::GameDownloader::ServiceState* state);
      void unpackStateSaveFailed(P1::GameDownloader::ServiceState* state);

    private:
      QString _extractorId;

    };
  }
}
