#pragma once

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkAccessManager>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    namespace Behavior {

      class CheckTorrentModify : public QObject
      {
        Q_OBJECT
      public:
        explicit CheckTorrentModify(QObject *parent = 0);
        ~CheckTorrentModify();

        void start(P1::GameDownloader::ServiceState *state);
        QString lastModified() const;

      signals:
        void result(P1::GameDownloader::ServiceState *state, bool isModified);
        void error(P1::GameDownloader::ServiceState *state);

      private slots:
        void slotError(QNetworkReply::NetworkError error);
        void slotReplyDownloadFinished();

      private:
        QUrl getTorrentUrlWithArchiveExtension();

        void saveLastModifiedDate(const QString& date);
        QString loadLastModifiedDate() const;

        void saveTorrenthash(const QString& date);
        QString torrentHash() const;

        P1::GameDownloader::ServiceState *_state;
        QString _lastModified;
        QNetworkAccessManager *_manager;
        int _headRequestRetryCount;
        int _maxHeadRequestRetryCount;

      };

    }
  }
}