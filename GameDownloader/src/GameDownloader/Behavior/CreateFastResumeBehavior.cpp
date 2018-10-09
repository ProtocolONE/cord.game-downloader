/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (c) 2011 - 2015, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/Behavior/CreateFastResumeBehavior.h>
#include <GameDownloader/ServiceState.h>
#include <GameDownloader/CheckUpdateHelper.h>

#include <Core/Service>

#include <LibtorrentWrapper/TorrentConfig>
#include <LibtorrentWrapper/Wrapper>

using GGS::Libtorrent::TorrentConfig;

namespace GGS {
  namespace GameDownloader {
    namespace Behavior {
      CreateFastResumeBehavior::CreateFastResumeBehavior(QObject *parent /*= 0*/)
        : BaseBehavior(parent)
      {
      }

      CreateFastResumeBehavior::~CreateFastResumeBehavior()
      {
      }

      void CreateFastResumeBehavior::start(GGS::GameDownloader::ServiceState *state)
      {
        Q_CHECK_PTR(state);
        Q_CHECK_PTR(this->_wrapper);

        TorrentConfig config;
        config.setPathToTorrentFile(CheckUpdateHelper::getTorrentPath(state));
        config.setDownloadPath(state->service()->downloadPath());
        config.setIsForceRehash(false);
        config.setIsReloadRequired(false);

        this->_wrapper->createFastResume(state->id(), config);

        emit this->next(CreateFastResumeBehavior::Created, state);
      }

      void CreateFastResumeBehavior::stop(GGS::GameDownloader::ServiceState *state)
      {
      }

      void CreateFastResumeBehavior::setTorrentWrapper(GGS::Libtorrent::Wrapper *wrapper)
      {
        Q_CHECK_PTR(wrapper);
        this->_wrapper = wrapper;
      }
    }
  }
}