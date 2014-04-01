#pragma once
#include <GameDownloader/GameDownloader_global>
#include <GameDownloader/Behavior/BaseBehavior.h>

#include <QtCore/QObject>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {

      class DOWNLOADSERVICE_EXPORT Bindiff : public BaseBehavior
      {
        Q_OBJECT
      public:
        explicit Bindiff(QObject *parent = 0);
        ~Bindiff();

      signals:
        void gg();

      };


    }
  }
}
