/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates. 
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#include <Settings/Settings.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <Core/Service>

#include <QtCore/QDateTime>
#include <QtCore/QDataStream>

namespace GGS {
  namespace GameDownloader {

    ServiceState::ServiceState(QObject *parent) 
      : QObject(parent)
      , _service(0)
      , _state(Stopped)
      , _startType(Normal)
      , _lastDateStateChanded(-1)
      , _isFoundNewUpdate(false)
      , _isDirectoryChanged(false)
      , _currentBehavior(NULL)
      , _isGameClientComplete(false)
      , _shouldReload(false)
    {
    }

    ServiceState::~ServiceState()
    {
    }

    const QString& ServiceState::id() const
    {
      Q_ASSERT(this->_service);
      return this->_service->id();
    }

    void ServiceState::setTorrentLastModifedDate(const QString& date)
    {
      this->_lastTorrentModifedDate = date;
    }

    QString ServiceState::torrentLastModifedDate() const
    {
      return this->_lastTorrentModifedDate;
    }

    void ServiceState::setForceReaload(bool value)
    {
      this->_shouldReload = value;
    }

    bool ServiceState::isForceReload()
    {
      return this->_shouldReload;
    }

    void ServiceState::setState(const State state)
    {
      this->_state = state;
      this->_lastDateStateChanded = QDateTime::currentMSecsSinceEpoch();
    }

    GGS::GameDownloader::ServiceState::State ServiceState::state() const
    {
      return this->_state;
    }

    void ServiceState::resetLastDateStateChanged()
    {
      this->_lastDateStateChanded = 0;
    }

    qint64 ServiceState::lastDateStateChanged() const
    {
      return this->_lastDateStateChanded;
    }

    void ServiceState::setStartType(StartType type)
    {
      this->_startType = type;
    }

    StartType ServiceState::startType() const
    {
      return this->_startType;
    }

    bool ServiceState::isFoundNewUpdate() const
    {
      return this->_isFoundNewUpdate;
    }

    void ServiceState::setIsFoundNewUpdate(bool isFoundNewUpdate)
    {
      this->_isFoundNewUpdate = isFoundNewUpdate;
    }

    void ServiceState::setService(const GGS::Core::Service *service)
    {
      Q_ASSERT(service);
      this->_service = service;
    }

    const GGS::Core::Service* ServiceState::service() const
    {
      return this->_service;
    }

    bool ServiceState::isDirectoryChanged() const
    {
      return this->_isDirectoryChanged;
    }

    void ServiceState::setIsDirectoryChanged(bool isDirectoryChanged)
    {
      this->_isDirectoryChanged = isDirectoryChanged;
    }

    Behavior::BaseBehavior* ServiceState::currentBehavior() const
    {
      return  this->_currentBehavior;
    }

    void ServiceState::setCurrentBehavior(Behavior::BaseBehavior* behavior)
    {
      this->_currentBehavior = behavior;
    }

    bool ServiceState::isGameClientComplete() const
    {
      return this->_isGameClientComplete;
    }

    void ServiceState::setIsGameClientComplete(bool value)
    {
      this->_isGameClientComplete = value;
    }

    bool ServiceState::isDownloadSuccess()
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup(this->id());
      bool ok;
      int result = settings.value("DistrProcessed", 0).toInt(&ok);
      return ok && result == 1;
    }

    void ServiceState::setDownloadSuccess(bool value)
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup(this->id());
      settings.setValue("DistrProcessed", value ? 1 : 0);
    }

    void ServiceState::setPatchFiles(const QStringList& files)
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("Bindiff");
      settings.beginGroup(this->id());
        
      settings.setValue("patchFiles", serialize(files));
    }

    void ServiceState::setPatchVersion(const QString& version)
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("Bindiff");
      settings.beginGroup(this->id());

      settings.setValue("patchVersion", version);
    }

    QStringList ServiceState::patchFiles() 
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("Bindiff");
      settings.beginGroup(this->id());

      return this->deserialize(settings.value("patchFiles", QByteArray()).toByteArray());
    }

    QString ServiceState::patchVersion()
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("Bindiff");
      settings.beginGroup(this->id());

      return settings.value("patchVersion", QString()).toString();
    }

    void ServiceState::setPackingFiles(const QStringList& files)
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("7zCompressor");
      settings.beginGroup(this->id());
      settings.setValue("Files", serialize(files));
    }

    QStringList ServiceState::packingFiles()
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("7zCompressor");
      settings.beginGroup(this->id());

     return this->deserialize(settings.value("Files", QByteArray()).toByteArray());
    }

    QStringList ServiceState::deserialize(QByteArray serialized)
    {
      QStringList result;
      QDataStream in(&serialized, QIODevice::ReadOnly);
      in >> result;
      return result;
    }

    QByteArray ServiceState::serialize(QStringList stringList)
    {
      QByteArray byteArray;
      QDataStream out(&byteArray, QIODevice::WriteOnly);
      out << stringList;
      return byteArray;
    }

    void ServiceState::setIsInstalled(bool isInstalled)
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup(this->id());
      settings.setValue("isInstalled", isInstalled ? 1 : 0);

      if (isInstalled && !settings.contains("installDate"))
        settings.setValue("installDate", QDateTime::currentDateTime());
    }

    bool ServiceState::isInstalled() const
    {
      return ServiceState::isInstalled(this->id());
    }

    bool ServiceState::isInstalled(const QString& serviceId)
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup(serviceId);
      bool ok;
      int result = settings.value("isInstalled", 0).toInt(&ok);
      return ok && result == 1;
    }

    bool ServiceState::shouldGenerateFastResume() const
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("FastResume");
      settings.beginGroup(this->id());
      bool ok;
      int result = settings.value("generate", 0).toInt(&ok);
      return ok && result == 1;
    }

    void ServiceState::setGenerateFastResume(bool value)
    {
      GGS::Settings::Settings settings;
      settings.beginGroup("GameDownloader");
      settings.beginGroup("FastResume");
      settings.beginGroup(this->id());
      settings.setValue("generate", value ? 1 : 0);
    }

  }
}