#include <GameDownloader/Behavior/CreateFastResumeBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <Core/Service.h>

#include <LibtorrentWrapper/TorrentConfig.h>
#include <LibtorrentWrapper/Wrapper.h>

using P1::Libtorrent::TorrentConfig;

namespace P1 {
  namespace GameDownloader {
    namespace Behavior {
      CreateFastResumeBehavior::CreateFastResumeBehavior(QObject *parent /*= 0*/)
        : BaseBehavior(parent)
      {
      }

      CreateFastResumeBehavior::~CreateFastResumeBehavior()
      {
      }

      void CreateFastResumeBehavior::start(P1::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(this->_wrapper);

        if (state->shouldGenerateFastResume()) {
          state->setGenerateFastResume(false);

          TorrentConfig config;
          config.setPathToTorrentFile(CheckUpdateHelper::getTorrentPath(state));
          config.setDownloadPath(state->service()->downloadPath());
          config.setIsForceRehash(false);
          config.setIsReloadRequired(false);

          this->_wrapper->createFastResume(state->id(), config);
        }

        emit this->next(CreateFastResumeBehavior::Created, state);
      }

      void CreateFastResumeBehavior::stop(P1::GameDownloader::ServiceState *state)
      {
      }

      void CreateFastResumeBehavior::setTorrentWrapper(P1::Libtorrent::Wrapper *wrapper)
      {
        Q_CHECK_PTR(wrapper);
        this->_wrapper = wrapper;
      }
    }
  }
}