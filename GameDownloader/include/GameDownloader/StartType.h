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

#include <QtCore/QMetaType>

namespace GGS {
  namespace GameDownloader {

    enum StartType {
      Unknown = -1,
      Normal = 0,
      Force,
      Recheck, // Заставили перепроверить клиент из настроек
      Shadow
    };

    Q_ENUMS(GGS::GameDownloader::StartType);
    Q_DECLARE_METATYPE(GGS::GameDownloader::StartType);
  }
}
