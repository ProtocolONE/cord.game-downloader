/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_BUILDER_H_
#define _GGS_GAMEDOWNLOADER_BUILDER_H_

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/LibtorrentWrapperAdapter.h>
#include <GameDownloader/GameDownloadService.h>

#include <LibtorrentWrapper/Wrapper>

#include <QtCore/QObject>

namespace GGS {
  namespace GameDownloader {

    class DOWNLOADSERVICE_EXPORT Builder : public QObject
    {
      Q_OBJECT
    public:
      Builder(QObject *parent = 0);
      ~Builder();

      void build();

      GGS::Libtorrent::Wrapper& torrentWrapper();
      GameDownloadService& gameDownloader();

    private:
      GameDownloadService _gameDownloader;
      LibtorrentWrapperAdapter _libtorrentAdapter;
      GGS::Libtorrent::Wrapper _wrapper;
    };

  }
}
#endif // _GGS_GAMEDOWNLOADER_BUILDER_H_