/*
 * dll.c
 *
 * Description:
 * This translation unit implements DLL initialisation.
 */

/* We use the DLL entry point function to set up per thread storage
   specifically to hold the threads own thread ID.

   The thread ID is stored by _pthread_start_call().

   The thread ID is retrieved by pthread_self().

 */

#include <windows.h>

/* Global index for TLS data. */
DWORD _pthread_threadID_TlsIndex;

BOOL WINAPI PthreadsEntryPoint(HINSTANCE dllHandle,
			  DWORD reason,
			  LPVOID situation)
{
  

  switch (reason)
    {
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;

    case DLL_PROCESS_ATTACH:
      _pthread_threadID_TlsIndex = TlsAlloc();

      if (_pthread_threadID_TlsIndex == 0xFFFFFFFF)
	{
	  return FALSE;
	}
      break;

    case DLL_PROCESS_DETACH:
      (void) TlsFree(_pthread_threadID_TlsIndex);
      break;

    default:
      return FALSE;
    }

  return TRUE;
}
