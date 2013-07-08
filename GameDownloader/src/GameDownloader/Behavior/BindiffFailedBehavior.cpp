/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (�) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/BindiffFailedBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <Settings/Settings>
#include <Core/Service.h>
#include <QtCore/QStringList>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      BindiffFailedBehavior::BindiffFailedBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      BindiffFailedBehavior::~BindiffFailedBehavior()
      {
      }

      void BindiffFailedBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);

        state->setPatchVersion("");
        state->setPatchFiles(QStringList());
        state->setPackingFiles(QStringList());

        GGS::Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup("SevenZipGameExtractor");
<<<<<<< HEAD
<<<<<<< HEAD
        if (settings.value(state->id(), QByteArray()) != QByteArray()) {
          settings.setValue(state->id(), QByteArray());
        }
=======
        settings.setValue(state->id(), QByteArray());
>>>>>>> 4ffdcdc... QGNA-295 правки по ревью
=======
        if (settings.value(state->id(), QByteArray()) != QByteArray())
          settings.setValue(state->id(), QByteArray());
>>>>>>> 87d1f88... QGNA-295 Правки по ревью

        emit this->next(Finished, state);
      }

      void BindiffFailedBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
      }

    }
  }
}
