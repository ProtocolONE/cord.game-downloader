#include <GameDownloader/Behavior/CheckUpdateBehavior.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/ServiceState.h>

#include <Core/Service>

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {
      class PriveateCheckUpdateHelper : public CheckUpdateHelper
      {
        Q_OBJECT
      public:
        explicit PriveateCheckUpdateHelper(GGS::GameDownloader::ServiceState *state, QObject *parent)
          : CheckUpdateHelper(parent) 
          , _state(state)
        {
        }

        ~PriveateCheckUpdateHelper(){};

        GGS::GameDownloader::ServiceState* state() 
        {
          return this->_state;
        }

      private:
        GGS::GameDownloader::ServiceState *_state;
      };
    }
  }
}