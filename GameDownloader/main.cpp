#ifndef DOWNLOADSERVICE_STATIC_LIB

#include <Windows.h>
#include <QtCore/QMetaType>

#include <GameDownloader/RegisterTypes.h>

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
    P1::GameDownloader::registerTypes();
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
#endif