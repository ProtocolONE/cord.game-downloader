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

#include <QtCore/QObject>
#include <GameDownloader/ServiceState>
#include <QtNetwork/QNetworkReply>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    class CheckPatchExist : public QObject
    {
      Q_OBJECT

    public:
      CheckPatchExist(GGS::GameDownloader::ServiceState *state, QObject *parent = 0);
      ~CheckPatchExist();

    public slots:
      void startCheck(const QString& url);

    private slots:
      void error(QNetworkReply::NetworkError);
      void finished();

    signals:
      void found(GGS::GameDownloader::ServiceState *);
      void notFound(GGS::GameDownloader::ServiceState *);

    private:
      GGS::GameDownloader::ServiceState* _state;
      QNetworkAccessManager *_manager;
      QString _url;
    };
  }
}