/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/LibtorrentWrapperAdapter.h>
#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/CheckUpdateHelper>

#include <LibtorrentWrapper/TorrentConfig>
#include <LibtorrentWrapper/Wrapper>

#include <Core/Service>

#include <QtCore/QMutexLocker>
#include <QtCore/QDebug>

using namespace GGS::Libtorrent;
using namespace GGS::Core;
namespace GGS {
  namespace GameDownloader {

    LibtorrentWrapperAdapter::LibtorrentWrapperAdapter(QObject *parent)
      : QObject(parent)
    {
    }

    LibtorrentWrapperAdapter::~LibtorrentWrapperAdapter()
    {
    }

    void LibtorrentWrapperAdapter::downloadRequest(const Service *service, StartType startType, bool isReloadRequired)
    {
      Q_ASSERT(service);
      this->saveService(service);
      TorrentConfig config;
      config.setPathToTorrentFile(CheckUpdateHelper::getTorrentPath(service));
      config.setDownloadPath(service->downloadPath());
      config.setIsForceRehash(startType == Recheck);
      config.setIsReloadRequired(startType == Recheck || isReloadRequired);
      this->_wrapper->start(service->id(), config);
    }

    void LibtorrentWrapperAdapter::pauseRequest(const Service *service)
    {
      Q_ASSERT(service);
      this->saveService(service);
      this->_wrapper->stop(service->id());
    }

    void LibtorrentWrapperAdapter::saveService(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      QMutexLocker lock(&this->_mutex);
      this->_serviceHash[service->id()] = service;
    }

    const GGS::Core::Service* LibtorrentWrapperAdapter::getService(const QString& id)
    {
      QMutexLocker lock(&this->_mutex);
      if (!this->_serviceHash.contains(id))
        return 0;

      return this->_serviceHash[id];
    }

    void LibtorrentWrapperAdapter::setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper)
    {
      this->_wrapper = wrapper;
      QObject::connect(this->_wrapper, SIGNAL(torrentPaused(QString)), this, SLOT(torrentPausedSlot(QString)), Qt::QueuedConnection);
      QObject::connect(this->_wrapper, SIGNAL(torrentDownloadFinished(QString)), this, SLOT(torrentDownloadFinishedSlot(QString)), Qt::QueuedConnection);

      // Просто реемитим ошибку безликую, при желании можно что-нить поконкретнее пробросить
      QObject::connect(this->_wrapper, SIGNAL(torrentError(QString)), 
        this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection);
      QObject::connect(this->_wrapper, SIGNAL(fileError(QString, QString, int)), 
        this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection);
      QObject::connect(this->_wrapper, SIGNAL(startTorrentFailed(QString, int)), 
        this, SLOT(torrentDownloadFailedSlot(QString)), Qt::QueuedConnection);
    }

    void LibtorrentWrapperAdapter::torrentPausedSlot(QString id)
    {
      const GGS::Core::Service *service = this->getService(id);
      if (!service) {
        CRITICAL_LOG << "Unknown torrent event for id " << id;
        return;
      }

      emit this->torrentPaused(service);
    }

    void LibtorrentWrapperAdapter::torrentDownloadFinishedSlot(QString id)
    {
      const GGS::Core::Service *service = this->getService(id);
      if (!service) {
        CRITICAL_LOG << "Unknown torrent event for id " << id;
        return;
      }

      emit this->torrentDownloadFinished(service);
    }

    void LibtorrentWrapperAdapter::torrentDownloadFailedSlot(QString id)
    {
      const GGS::Core::Service *service = this->getService(id);
      if (!service) {
        CRITICAL_LOG << "Unknown torrent event for id " << id;
        return;
      }

      emit this->torrentDownloadFailed(service);
    }

  }
}