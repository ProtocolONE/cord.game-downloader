#pragma once

#include <UpdateSystem/UpdateInfoGetterResultInterface.h>
#include <QtCore/QObject>
#include <QtCore/QEventLoop>
#include <QtCore/QPointer>

namespace P1 {
  namespace GameDownloader {
    namespace Extractor {

      class UpdateInfoGetterResultEventLoopKiller : public QObject,
        public P1::UpdateSystem::UpdateInfoGetterResultInterface
      {
        Q_OBJECT
      public:
        explicit UpdateInfoGetterResultEventLoopKiller(QObject *parent = 0);
        ~UpdateInfoGetterResultEventLoopKiller();

        void updateInfoCallback(P1::UpdateSystem::UpdateInfoGetterResults error);
        void infoGetterUpdateProggress(quint64 current, quint64 total);
        
        void setEventLoop(QEventLoop *loop);
        P1::UpdateSystem::UpdateInfoGetterResults result() const;

      private:
        QPointer<QEventLoop> _loop;
        P1::UpdateSystem::UpdateInfoGetterResults _result;
      };

    }
  }
}
