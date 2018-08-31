#include <GameDownloader/HookBase.h>
#include <GameDownloader/StartType.h>
#include <GameDownloader/CheckUpdateHelper.h>
#include <GameDownloader/ServiceState.h>

#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs.h>

#include <Core/Service.h>

#include <Windows.h>
#include <QtCore/QMetaType>

namespace P1 {
  namespace GameDownloader {

    void registerTypes()
    {
      qRegisterMetaType<P1::GameDownloader::HookBase::HookResult>("P1::GameDownloader::HookBase::HookResult");
      qRegisterMetaType<P1::GameDownloader::StartType>("P1::GameDownloader::StartType");
      qRegisterMetaType<P1::GameDownloader::CheckUpdateHelper::CheckUpdateType>("P1::GameDownloader::CheckUpdateHelper::CheckUpdateType");
      qRegisterMetaType<P1::Libtorrent::EventArgs::ProgressEventArgs>("P1::Libtorrent::EventArgs::ProgressEventArgs");
      qRegisterMetaType<const P1::Core::Service*>("const P1::Core::Service*");
      qRegisterMetaType<P1::GameDownloader::ServiceState *>("P1::GameDownloader::ServiceState *");
    }
  }
}

