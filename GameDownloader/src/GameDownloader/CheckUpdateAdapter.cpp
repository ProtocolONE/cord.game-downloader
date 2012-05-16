/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <GameDownloader/CheckUpdateAdapter.h>
#include <GameDownloader/GameDownloadService.h>
#include <Core/Service>

namespace GGS {
  namespace GameDownloader {

    CheckUpdateAdapter::CheckUpdateAdapter(QObject *parent)
      : QObject(parent)
    {
    }

    CheckUpdateAdapter::~CheckUpdateAdapter()
    {
    }

    void CheckUpdateAdapter::checkUpdateRequest(const GGS::Core::Service *service, CheckUpdateHelper::CheckUpdateType type)
    {
      Q_ASSERT(service);
      GGS::GameDownloader::CheckUpdateHelper *helper = new GGS::GameDownloader::CheckUpdateHelper(this);
      QObject::connect(helper, SIGNAL(result(const GGS::Core::Service *, bool)),
        this, SLOT(checkUpdateResult(const GGS::Core::Service *, bool)),
        Qt::QueuedConnection);

      QObject::connect(helper, SIGNAL(error(const GGS::Core::Service *)),
        this, SLOT(checkUpdateError(const GGS::Core::Service *)),
        Qt::QueuedConnection);
      
      QObject::connect(helper, SIGNAL(checkUpdateProgressChanged(const QString&, quint8)),
        this, SIGNAL(checkUpdateProgressChanged(const QString&, quint8)));

      helper->startCheck(service, type);
    }

    void CheckUpdateAdapter::checkUpdateResult(const GGS::Core::Service *service, bool isUpdated)
    {
      Q_ASSERT(service);
      CheckUpdateHelper *helper = qobject_cast<CheckUpdateHelper *>(QObject::sender());
      if (helper)
        helper->deleteLater();

      emit this->checkUpdateCompleted(service, isUpdated);
    }

    void CheckUpdateAdapter::checkUpdateError(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      CheckUpdateHelper *helper = qobject_cast<CheckUpdateHelper *>(QObject::sender());
      if (helper)
        helper->deleteLater();

      emit this->checkUpdateFailed(service);
    }

    void CheckUpdateAdapter::setGameDownloaderService(GameDownloadService *gameDownloader)
    {
      this->_gameDownloaderService = gameDownloader;
    }

  }
}