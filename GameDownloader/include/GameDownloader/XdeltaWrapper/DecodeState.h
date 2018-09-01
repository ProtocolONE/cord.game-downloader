#pragma once
#include <QtCore/QObject>

namespace P1 {
  namespace GameDownloader {
    namespace XdeltaWrapper {
      class DecodeStatePrivate;
      class DecodeState : public QObject
      {
        Q_OBJECT
      public:
        friend class XdeltaDecoder;

        explicit DecodeState(QObject *parent = 0);
        ~DecodeState();

      private:
        DecodeStatePrivate *_d;
      };
    }
  }
}
