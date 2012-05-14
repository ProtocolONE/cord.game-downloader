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
#include <LibtorrentWrapper/TorrentConfig>
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
      this->saveService(service);
      TorrentConfig config;
      config.setPathToTorrentFile(this->getTorrentPath(service));
      config.setDownloadPath(service->downloadPath());
      config.setIsForceRehash(startType == Recheck);
      config.setIsReloadRequired(startType == Recheck || isReloadRequired);
      this->_wrapper->start(service->id(), config);
    }

    void LibtorrentWrapperAdapter::pauseRequest(const Service *service)
    {
      this->saveService(service);
      this->_wrapper->stop(service->id());
    }

    QString LibtorrentWrapperAdapter::getTorrentPath(const Service *service)
    {
      QString dirName = service->torrentFilePath();

      if (!(dirName.endsWith('/') || dirName.endsWith('\\')))
        dirName.append('/');
      return QString("%1%2/%3.torrent")
        .arg(dirName)
        .arg(this->getServiceAreaString(service))
        .arg(service->id());
    }

    QString LibtorrentWrapperAdapter::getServiceAreaString(const Service *service)
    {
      switch(service->area()) {
      case Service::Live:;
        return QString("live");
      case Service::Pts:
        return QString("pts");
      case Service::Tst:
        return QString("tst");
      default:
        return QString("");
      }
    }

    void LibtorrentWrapperAdapter::saveService(const GGS::Core::Service *service)
    {
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
        qCritical() << "Unknown torrent event for id " << id;
        return;
      }

      emit this->torrentPaused(service);
    }

    void LibtorrentWrapperAdapter::torrentDownloadFinishedSlot(QString id)
    {
      const GGS::Core::Service *service = this->getService(id);
      if (!service) {
        qCritical() << "Unknown torrent event for id " << id;
        return;
      }

      emit this->torrentDownloadFinished(service);
    }

    void LibtorrentWrapperAdapter::torrentDownloadFailedSlot(QString id)
    {
      const GGS::Core::Service *service = this->getService(id);
      if (!service) {
        qCritical() << "Unknown torrent event for id " << id;
        return;
      }

      emit this->torrentDownloadFailed(service);
    }

  }
}