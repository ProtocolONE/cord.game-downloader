#include <GameDownloader/XdeltaWrapper/DecodeState.h>
#include <GameDownloader/XdeltaWrapper/DecodeStatePrivate.h>

namespace P1 {
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