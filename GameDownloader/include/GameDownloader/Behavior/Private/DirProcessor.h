/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (ñ) 2011 - 2017, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>

#include <GameDownloader/Behavior/ReadOnlyBehavior.h>

namespace GGS {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    namespace Behavior {

      class DirProcessor : public QObject
      {
        Q_OBJECT
      public:
        explicit DirProcessor(QObject * parent = 0);
        virtual ~DirProcessor();

        void processFolders(const QStringList & folders, GGS::GameDownloader::ServiceState *state);

      signals:
        void showMessage(GGS::GameDownloader::ServiceState *state);
        void resultReady(int result, GGS::GameDownloader::ServiceState *state);

      private:
        bool dropFileFlags(const QString & dirStr, GGS::GameDownloader::ServiceState *state);

      };
    }
  }
}