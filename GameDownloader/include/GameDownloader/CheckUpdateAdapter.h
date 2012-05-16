/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_CHECKUPDATEADAPTER_H_
#define _GGS_GAMEDOWNLOADER_CHECKUPDATEADAPTER_H_

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <QtCore/QObject>

namespace GGS {
  namespace Core {
    class Service;
  }
  
  namespace GameDownloader {
    class GameDownloadService;


    /*!
      \class CheckUpdateAdapter
      \brief Check update adapter. 
      \author Ilya.Tkachenko
      \date 25.05.2012
    */
    class DOWNLOADSERVICE_EXPORT CheckUpdateAdapter : public QObject
    {
      Q_OBJECT
    public:
      explicit CheckUpdateAdapter(QObject *parent = 0);
      ~CheckUpdateAdapter();

      void setGameDownloaderService(GameDownloadService *gameDownloader);

    public slots:
      void checkUpdateRequest(const GGS::Core::Service *service, GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType type);

    signals:
      void checkUpdateCompleted(const GGS::Core::Service *service, bool isUpdated);
      void checkUpdateFailed(const GGS::Core::Service *service);
      void checkUpdateProgressChanged(const QString& serviceId, quint8 progress);

    private slots:
      void checkUpdateResult(const GGS::Core::Service *service, bool isUpdated);
      void checkUpdateError(const GGS::Core::Service *service);

    private:
      GameDownloadService *_gameDownloaderService;
    };

  }
}
#endif // _GGS_GAMEDOWNLOADER_CHECKUPDATEADAPTER_H_