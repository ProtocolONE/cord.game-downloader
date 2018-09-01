#pragma once
#include <QtCore/QObject>
#include <QtCore/QString>

#include <GameDownloader/Behavior/ReadOnlyBehavior.h>

namespace P1 {
  namespace Core {
    class Service;
  }

  namespace GameDownloader {
    namespace Behavior {

      class DirProcessor : public QObject
      {
        Q_OBJECT
      public:
        explicit DirProcessor(QObject * parent = 0);
        virtual ~DirProcessor();

        void processFolders(const QStringList & folders, P1::GameDownloader::ServiceState *state);

      signals:
        void showMessage(P1::GameDownloader::ServiceState *state);
        void resultReady(int result, P1::GameDownloader::ServiceState *state);

      private:
        bool dropFileFlags(const QString & dirStr, P1::GameDownloader::ServiceState *state, bool & flag);

      };
    }
  }
}