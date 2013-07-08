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
#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

namespace GGS {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class CheckTorrentModify : public QObject
      {
        Q_OBJECT
      public:
        explicit CheckTorrentModify(QObject *parent = 0);
        ~CheckTorrentModify();

        void start(GGS::GameDownloader::ServiceState *state);
        QString lastModified() const;

      signals:
        void result(GGS::GameDownloader::ServiceState *state, bool isModified);
        void error(GGS::GameDownloader::ServiceState *state);

      private slots:
        void slotError(QNetworkReply::NetworkError error);
        void slotReplyDownloadFinished();

      private:
        QUrl getTorrentUrlWithArchiveExtension();

        void saveLastModifiedDate(const QString& date);
        QString loadLastModifiedDate() const;

        void saveTorrenthash(const QString& date);
        QString torrentHash() const;

        GGS::GameDownloader::ServiceState *_state;
        QString _lastModified;
        QNetworkAccessManager *_manager;
        int _headRequestRetryCount;
        int _maxHeadRequestRetryCount;

      };

    }
  }
}