#include <GameDownloader/HookBase>
#include <GameDownloader/StartType.h>
#include <GameDownloader/CheckUpdateHelper>

#include <LibtorrentWrapper/EventArgs/ProgressEventArgs>

#include <Core/Service>

#include <Windows.h>
#include <QtCore/QMetaType>

void registerTypes()
{
  qRegisterMetaType<GGS::GameDownloader::HookBase::HookResult>("GGS::GameDownloader::HookBase::HookResult");
  qRegisterMetaType<GGS::GameDownloader::StartType>("GGS::GameDownloader::StartType");
  qRegisterMetaType<GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType>("GGS::GameDownloader::CheckUpdateHelper::CheckUpdateType");
  qRegisterMetaType<GGS::Libtorrent::EventArgs::ProgressEventArgs>("GGS::Libtorrent::EventArgs::ProgressEventArgs");
  qRegisterMetaType<const GGS::Core::Service *>("const GGS::Core::Service *");
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpReserved )  // reserved
{
  // Perform actions based on the reason for calling.
  switch( fdwReason ) 
  { 
  case DLL_PROCESS_ATTACH:
    // Initialize once for each new process.
    // Return FALSE to fail DLL load.
    registerTypes();
    break;

  case DLL_THREAD_ATTACH:
    // Do thread-specific initialization.
    break;

  case DLL_THREAD_DETACH:
    // Do thread-specific cleanup.
    break;

  case DLL_PROCESS_DETACH:
    // Perform any necessary cleanup.
    break;
  }
  return TRUE;  // Successful DLL_PROCESS_ATTACH.
}