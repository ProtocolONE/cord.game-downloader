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
#include <GameDownloader/ServiceState.h>

#include <QtCore/QObject>

namespace GGS {
  namespace GameDownloader {

    class GameDownloadService;
    class DOWNLOADSERVICE_EXPORT HookBase : public QObject
    {
      Q_OBJECT
      Q_ENUMS(GGS::GameDownloader::HookBase::HookResult);
    public:
      
      // UNDONE Пересмотреть и вохможно уменьшить число результатов
      enum HookResult {
        Continue = 0,
        Abort = 1,
        CheckUpdate,
        PreHookStartPoint,
        PreHookEndPoint,
        PostHookStartPoint,
        PostHookEndPoint,
        DownloadStartPoint,
        DownloadEndPoint,
        ExtractionStartPoint,
        ExtractionEndPoint,
        Finish,
      };

      HookBase(const QString& hookId, QObject *parent = 0);
      virtual ~HookBase();

      virtual HookResult beforeDownload(GameDownloadService *gameDownloader, ServiceState *state) = 0;
      virtual HookResult afterDownload(GameDownloadService *gameDownloader, ServiceState *state) = 0;

      const QString& hookId();

      quint8 beforeProgressWeight() const;
      void setBeforeProgressWeight(quint8 weight);
      
      quint8 afterProgressWeight() const;
      void setAfterProgressWeight(quint8 weight);

    public slots:
      void pauseRequestSlot(GGS::GameDownloader::ServiceState *state);

    signals:
      void beforeProgressChanged(const QString& serviceId, const QString& hookId, quint8 progress);
      void afterProgressChanged(const QString& serviceId, const QString& hookId, quint8 progress);
      void statusMessageChanged(GGS::GameDownloader::ServiceState *state, const QString& message);

      void pauseRequest(GGS::GameDownloader::ServiceState *state);

    protected:
      quint8 _beforeProgressWeight;
      quint8 _afterProgressWeight;
      QString _hookId;

    };
  }
}
