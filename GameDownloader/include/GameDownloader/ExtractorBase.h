/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_EXTRACTORBASE_H_
#define _GGS_GAMEDOWNLOADER_EXTRACTORBASE_H_

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/StartType.h>

#include <QtCore/QObject>

namespace GGS {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    class GameDownloadService;
    class DOWNLOADSERVICE_EXPORT ExtractorBase : public QObject
    {
      Q_OBJECT
    public:
      ExtractorBase(const QString &extractorId, QObject *parent = 0);
      virtual ~ExtractorBase();

      void setGameDownloadService(GameDownloadService *gameDownloadService);
      virtual void extract(const GGS::Core::Service *service, StartType startType) = 0;
      const QString &extractorId();

    public slots:
      void pauseRequestSlot(const GGS::Core::Service *service);

    signals:
      void pauseRequest(const GGS::Core::Service *service);
      void extractFinished(const GGS::Core::Service *service);
      void extractPaused(const GGS::Core::Service *service);
      void extractFailed(const GGS::Core::Service *service);
      void extractionProgressChanged(const QString& serviceId, qint8 progress, qint64 current, qint64 total);

    protected:
      GameDownloadService *_gameDownloadService;

    private:
      QString _extractorId;

    };
  }
}

#endif // _GGS_GAMEDOWNLOADER_EXTRACTORBASE_H_