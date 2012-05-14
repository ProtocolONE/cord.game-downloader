/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (�) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_HOOKINTERFACE_H_
#define _GGS_GAMEDOWNLOADER_HOOKINTERFACE_H_

#include <Core/Service>
#include <GameDownloader/GameDownloader_global.h>
#include <QtCore/QObject>

namespace GGS {
  namespace GameDownloader {
    class DOWNLOADSERVICE_EXPORT HookBase : public QObject
    {
      Q_OBJECT
      Q_ENUMS(GGS::GameDownloader::HookBase::HookResult);
    public:

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

      virtual HookResult beforeDownload(const GGS::Core::Service *service) = 0;
      virtual HookResult afterDownload(const GGS::Core::Service *service) = 0;

      const QString& hookId();

      quint8 beforeProgressWeight() const;
      void setBeforeProgressWeight(quint8 weight);
      
      quint8 afterProgressWeight() const;
      void setAfterProgressWeight(quint8 weight);

    signals:
      void beforeProgressChanged(const QString& serviceId, const QString& hookId, quint8 progress);
      void afterProgressChanged(const QString& serviceId, const QString& hookId, quint8 progress);

    private:
      QString _hookId;

      quint8 _beforeProgressWeight;
      quint8 _afterProgressWeight;

    };
  }
}

#endif // _GGS_GAMEDOWNLOADER_HOOKINTERFACE_H_