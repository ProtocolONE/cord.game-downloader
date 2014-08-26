/****************************************************************************
** This file is a part of Syncopate Limited GameNet Application or it parts.
**
** Copyright (©) 2011 - 2012, Syncopate Limited and/or affiliates.
** All rights reserved.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
****************************************************************************/
#include <GameDownloader/HookBase>
#include <GameDownloader/StartType.h>
#include <GameDownloader/CheckUpdateHelper>
#include <GameDownloader/ServiceState.h>

#include <GameDownloader/XdeltaWrapper/XdeltaDecoder.h>

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
  qRegisterMetaType<const GGS::Core::Service*>("const GGS::Core::Service*");
  qRegisterMetaType<GGS::GameDownloader::ServiceState *>("GGS::GameDownloader::ServiceState *");
}

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,  // handle to DLL module
  DWORD fdwReason,     // reason for calling function
  LPVOID lpReserved )  // reserved
{
  UNREFERENCED_PARAMETER(hinstDLL);
  UNREFERENCED_PARAMETER(lpReserved);

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