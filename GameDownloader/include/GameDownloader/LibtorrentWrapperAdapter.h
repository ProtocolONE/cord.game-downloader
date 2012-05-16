/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_LIBTORRENTWRAPPERADAPTER_H_
#define _GGS_GAMEDOWNLOADER_LIBTORRENTWRAPPERADAPTER_H_

#include <GameDownloader/StartType.h>

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <QtCore/QMutex>

namespace GGS {
  namespace Core {
    class Service;
  }

  namespace Libtorrent {
    class Wrapper;
  }

  namespace GameDownloader {

    class LibtorrentWrapperAdapter : public QObject
    {
      Q_OBJECT
    public:
      explicit LibtorrentWrapperAdapter(QObject *parent = 0);
      ~LibtorrentWrapperAdapter();

      void setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper);

    public slots:
      void downloadRequest(const GGS::Core::Service *service, GGS::GameDownloader::StartType startType, bool isReloadRequired);
      void pauseRequest(const GGS::Core::Service *service);
      
    signals:
      void torrentPaused(const GGS::Core::Service *service);
      void torrentDownloadFinished(const GGS::Core::Service *service);
      void torrentDownloadFailed(const GGS::Core::Service *service);

    private slots:
      void torrentPausedSlot(QString id);
      void torrentDownloadFinishedSlot(QString id);
      void torrentDownloadFailedSlot(QString id);

    private:
      void saveService(const GGS::Core::Service *service);
      const GGS::Core::Service* getService(const QString& id);

      GGS::Libtorrent::Wrapper *_wrapper;
      QMutex _mutex;
      QHash<QString, const GGS::Core::Service *> _serviceHash;
        
    };

  }
}

#endif // _GGS_GAMEDOWNLOADER_LIBTORRENTWRAPPERADAPTER_H_