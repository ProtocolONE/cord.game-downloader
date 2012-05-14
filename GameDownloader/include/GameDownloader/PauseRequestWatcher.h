/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/

#ifndef _GGS_GAMEDOWNLOADER_PAUSEREQUESTWACHER_H_
#define _GGS_GAMEDOWNLOADER_PAUSEREQUESTWACHER_H_

#include <Core/Service.h>

#include <QtCore/QObject>
#include <QtCore/QString>

namespace GGS {
  namespace GameDownloader {

    class PauseRequestWatcher : public QObject
    {
      Q_OBJECT
    public:
      PauseRequestWatcher(const GGS::Core::Service *service);
      ~PauseRequestWatcher();

      bool isPaused() const;

    public slots:
      void pauseRequestSlot(const GGS::Core::Service *service);

    private:
      bool _isPaused;
      QString _id;
    };

  }
}
#endif // _GGS_GAMEDOWNLOADER_PAUSEREQUESTWACHER_H_