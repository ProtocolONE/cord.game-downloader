/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#pragma once
#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class BindiffBehaviorPrivate : public QObject 
      {
        Q_OBJECT
      public:
        explicit BindiffBehaviorPrivate(QObject *parent = 0);
        ~BindiffBehaviorPrivate();

        GGS::GameDownloader::XdeltaWrapper::XdeltaDecoder _decoder;
        QStringList _patchFiles;
        QString _workingFile;
        QString _rootDir;
        QString _patchDir;
        QString _patchVersion;
        GGS::GameDownloader::ServiceState* _state;

        qint64 _totalFileCount;
        qint64 _filesCount;
        bool _skipMode;
        int _progressMod;
      };
    };
  };
};