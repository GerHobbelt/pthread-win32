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
#include <malloc.h>
#include "pthread.h"
#include "implement.h"

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
      /* Allocate storage for thread admin arrays. */
      _pthread_virgins = 
	(_pthread_t *) malloc(sizeof(_pthread_t) * PTHREAD_THREADS_MAX);

      _pthread_reuse =
	(pthread_t *) malloc(sizeof(pthread_t) * PTHREAD_THREADS_MAX);

      _pthread_win32handle_map =
	(pthread_t *) malloc(sizeof(pthread_t) * PTHREAD_THREADS_MAX);

      _pthread_threads_mutex_table =
	(pthread_mutex_t *) malloc(sizeof(pthread_mutex_t) * PTHREAD_THREADS_MAX);

      /* Per thread thread ID storage. */
      _pthread_threadID_TlsIndex = TlsAlloc();

      if (_pthread_threadID_TlsIndex == 0xFFFFFFFF)
	{
	  return FALSE;
	}
      break;

    case DLL_PROCESS_DETACH:
      free(_pthread_threads_mutex_table);
      free(_pthread_win32handle_map);
      free(_pthread_reuse);
      free(_pthread_virgins);
      (void) TlsFree(_pthread_threadID_TlsIndex);
      break;

    default:
      return FALSE;
    }

  return TRUE;
}
