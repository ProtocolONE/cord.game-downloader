#include <GameDownloader/Behavior/BindiffFailedBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <Settings/Settings.h>
#include <Core/Service.h>
#include <QtCore/QStringList>

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {

      BindiffFailedBehavior::BindiffFailedBehavior(QObject *parent)
        : BaseBehavior(parent)
      {
      }

      BindiffFailedBehavior::~BindiffFailedBehavior()
      {
      }

      void BindiffFailedBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);

        state->setPatchVersion("");
        state->setPatchFiles(QStringList());
        state->setPackingFiles(QStringList());

        P1::Settings::Settings settings;
        settings.beginGroup("GameDownloader");
        settings.beginGroup("SevenZipGameExtractor");
        if (settings.value(state->id(), QByteArray()) != QByteArray())
          settings.setValue(state->id(), QByteArray());

        emit this->next(Finished, state);
      }

      void BindiffFailedBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
      }

    }
  }
}
