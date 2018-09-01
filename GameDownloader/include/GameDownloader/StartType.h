#pragma once

#include <QtCore/QMetaType>

namespace P1 {
  namespace GameDownloader {

    enum StartType {
      Unknown = -1,
      Normal = 0,
      Force,
      Recheck, // Заставили перепроверить клиент из настроек
      Shadow,
      Uninstall
    };

    Q_ENUMS(P1::GameDownloader::StartType);
    Q_DECLARE_METATYPE(P1::GameDownloader::StartType);
  }
}
