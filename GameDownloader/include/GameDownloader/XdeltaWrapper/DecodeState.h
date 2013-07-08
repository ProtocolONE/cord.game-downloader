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

namespace GGS {
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
