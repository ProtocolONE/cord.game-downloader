#include <GameDownloader/Behavior/Private/CheckPatchExist.h>

namespace P1 {
  namespace GameDownloader {
    class ServiceState;

    CheckPatchExist::CheckPatchExist(P1::GameDownloader::ServiceState* state, QObject *parent)
      : QObject(parent)
      , _state(state)
      , _manager(new QNetworkAccessManager(this))
    {
    }

    CheckPatchExist::~CheckPatchExist()
    {
    }

    void CheckPatchExist::startCheck(const QString& url)
    {
      this->_url = url;

      QNetworkRequest request(url);

      QNetworkReply *reply = this->_manager->head(request);
      connect(reply, SIGNAL(finished()), this, SLOT(finished()));
      connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(error(QNetworkReply::NetworkError))); 
    }

    void CheckPatchExist::error(QNetworkReply::NetworkError error)
    {
      DEBUG_LOG << "Head error" << this->_url << "NetworkError" << error;
      emit this->notFound(this->_state);
    }

    void CheckPatchExist::finished()
    {
      QNetworkReply *reply = qobject_cast<QNetworkReply*>(QObject::sender());
      if (!reply)
        return;

      reply->deleteLater();

      int httpCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
      // Http codes defined by rfc: http://www.w3.org/Protocols/rfc2616/rfc2616-sec10.html
      // 200 OK
      // 304 Not Modified
      if (httpCode != 200) {
        emit this->notFound(this->_state);
        return;
      }

      emit this->found(this->_state);
    }

  }
}