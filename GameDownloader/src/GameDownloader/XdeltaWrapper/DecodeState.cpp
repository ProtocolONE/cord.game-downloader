/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/XdeltaWrapper/DecodeState.h>
#include <GameDownloader/XdeltaWrapper/DecodeStatePrivate.h>

namespace GGS {
  namespace GameDownloader {
    namespace XdeltaWrapper {

      DecodeState::DecodeState(QObject *parent) 
        : QObject(parent)
        , _d(new DecodeStatePrivate(this))
      {
      }

      DecodeState::~DecodeState()
      {
      }

    }
  }
}