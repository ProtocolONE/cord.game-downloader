/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_SERVICESTATE_H_
#define _GGS_GAMEDOWNLOADER_SERVICESTATE_H_

#include <GameDownloader/GameDownloader_global.h>
#include <GameDownloader/StartType.h>

#include <QtCore/QObject>
#include <QtCore/QString>

namespace GGS {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    
    class DOWNLOADSERVICE_EXPORT ServiceState : public QObject
    {
      Q_OBJECT
      Q_ENUMS(GGS::GameDownloader::ServiceState::State);
      Q_ENUMS(GGS::GameDownloader::ServiceState::Stage);
    public:

      enum Stage {
        Nowhere = 0,
        CheckUpdate,
        PreHook,
        Download,
        PostHook,
        Extraction,
        Finished
      };

      enum State {
        Unknown = 0,
        Started,
        Stopping,
        Stopped
      };

      explicit ServiceState(QObject *parent = 0);
      ~ServiceState();

      void setService(const GGS::Core::Service *service);
      const GGS::Core::Service* service() const;

      const QString& id() const;

      void setState(const State state);
      State state() const;

      void setStage(const Stage stage);
      Stage stage() const;

      qint64 lastDateStateChanged() const;

      void setStartType(StartType type);
      StartType startType() const;

      bool isFoundNewUpdate() const;
      void setIsFoundNewUpdate(bool isFoundNewUpdate);

      bool isDirectoryChanged() const;
      void setIsDirectoryChanged(bool isDirectoryChanged);

    private:
      const GGS::Core::Service *_service;

      QString _id;
      State _state;
      Stage _stage;

      qint64 _lastDateStateChanded;
      StartType _startType;

      bool _isFoundNewUpdate;
      bool _isDirectoryChanged;
    };

  }
}
#endif // _GGS_GAMEDOWNLOADER_SERVICESTATE_H_