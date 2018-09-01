#pragma once

#include <GameDownloader/ServiceState.h>

#include <QtCore/QObject>
#include <QtNetwork/QNetworkReply>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    class CheckPatchExist : public QObject
    {
      Q_OBJECT

    public:
      CheckPatchExist(P1::GameDownloader::ServiceState *state, QObject *parent = 0);
      ~CheckPatchExist();

    public slots:
      void startCheck(const QString& url);

    private slots:
      void error(QNetworkReply::NetworkError);
      void finished();

    signals:
      void found(P1::GameDownloader::ServiceState *);
      void notFound(P1::GameDownloader::ServiceState *);

    private:
      P1::GameDownloader::ServiceState* _state;
      QNetworkAccessManager *_manager;
      QString _url;
    };
  }
}