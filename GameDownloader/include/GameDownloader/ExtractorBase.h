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
#include <GameDownloader/StartType.h>

#include <QtCore/QObject>

namespace GGS {
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
      void pauseRequestSlot(GGS::GameDownloader::ServiceState* state);

    signals:
      void pauseRequest(GGS::GameDownloader::ServiceState* state);
      void extractFinished(GGS::GameDownloader::ServiceState* state);
      void extractPaused(GGS::GameDownloader::ServiceState* state);
      void extractFailed(GGS::GameDownloader::ServiceState* state);

      void extractionProgressChanged(GGS::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total);

      void compressFinished(GGS::GameDownloader::ServiceState* state);
      void compressPaused(GGS::GameDownloader::ServiceState* state);
      void compressFailed(GGS::GameDownloader::ServiceState* state);

      void compressProgressChanged(GGS::GameDownloader::ServiceState* state, qint8 progress, qint64 current, qint64 total);

      void unpackStateSaved(GGS::GameDownloader::ServiceState* state);
      void unpackStateSaveFailed(GGS::GameDownloader::ServiceState* state);

    private:
      QString _extractorId;

    };
  }
}
