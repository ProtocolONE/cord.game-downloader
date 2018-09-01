#pragma once

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/StartType.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QByteArray>

namespace P1 {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    namespace Behavior {
      class BaseBehavior;
    }

    class DOWNLOADSERVICE_EXPORT ServiceState : public QObject
    {
      Q_OBJECT
      Q_ENUMS(P1::GameDownloader::ServiceState::State);
    public:
      enum State {
        Started = 1,
        Stopping = 2,
        Stopped = 3
      };

      explicit ServiceState(QObject *parent = 0);
      ~ServiceState();

      void setService(const P1::Core::Service *service);
      const P1::Core::Service* service() const;

      const QString& id() const;

      void setState(const State state);
      State state() const;

      void resetLastDateStateChanged();
      qint64 lastDateStateChanged() const;

      void setStartType(StartType type);
      StartType startType() const;

      bool isFoundNewUpdate() const;
      void setIsFoundNewUpdate(bool isFoundNewUpdate);

      bool isDirectoryChanged() const;
      void setIsDirectoryChanged(bool isDirectoryChanged);

      Behavior::BaseBehavior* currentBehavior() const;
      void setCurrentBehavior(Behavior::BaseBehavior* behavior);

      bool isGameClientComplete() const;
      void setIsGameClientComplete(bool value);

      bool isDownloadSuccess();
      void setDownloadSuccess(bool value);

      void setPackingFiles(const QStringList& files);
      QStringList packingFiles();

      void setPatchFiles(const QStringList& files);
      void setPatchVersion(const QString& version);

      QStringList patchFiles();
      QString patchVersion();

      void setTorrentLastModifedDate(const QString& date);
      QString torrentLastModifedDate() const;

      void setForceReaload(bool);
      bool isForceReload();


      /*!
        \fn bool ServiceState::isInstalled() const;
        \brief Query if this object is installed.
        \author Ilya.Tkachenko
        \date 13.09.2013
        \return true if installed, false if not.
      */
      bool isInstalled() const;

      /*!
        \fn void ServiceState::setIsInstalled(bool isInstalled);
        \brief Sets the is installed. Don't call it if you are not really sure that you have to.
        \author Ilya.Tkachenko
        \date 13.09.2013
        \param isInstalled true if is installed.
      */
      void setIsInstalled(bool isInstalled);


      /*!
        \fn static bool ServiceState::isInstalled(const QString& serviceId);
        \brief Узнает установлена ли игра.
        \author Ilya.Tkachenko
        \date 13.09.2013
        \param serviceId Identifier for the service.
        \return true if installed, false if not.
      */
      static bool isInstalled(const QString& serviceId);

      /*!
       \fn  bool ServiceState::shouldGenerateFastResume() const;
       \brief Queries if we should generate fast resume.
       \author  "Ilya Tkachenko"
       \date  10.11.2015
       \return  true if it succeeds, false if it fails.
       */
      bool shouldGenerateFastResume() const;

      /*!
       \fn  void ServiceState::setGenerateFastResume(bool value);
       \brief Sets a generate fast resume.
       \author  "Ilya Tkachenko"
       \date  10.11.2015
       \param value true if we want to generate fast resume.
       */
      void setGenerateFastResume(bool value);

    private:
      QStringList deserialize(QByteArray serialized);
      QByteArray serialize(QStringList stringList);

      const P1::Core::Service *_service;

      State _state;

      QString _lastTorrentModifedDate;
      qint64 _lastDateStateChanded;
      StartType _startType;

      bool _isFoundNewUpdate;
      bool _isDirectoryChanged;

      Behavior::BaseBehavior *_currentBehavior;
      bool _isGameClientComplete;
      bool _shouldReload;
    };

  }
}
