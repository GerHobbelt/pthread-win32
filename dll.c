/*
 * dll.c
 *
 * Description:
 * This translation unit implements DLL initialisation.
 */

#include <windows.h>
#include <malloc.h>
#include "pthread.h"
#include "implement.h"


/* Function pointer to TryEnterCriticalSection if it exists; otherwise NULL */
BOOL (WINAPI *_pthread_try_enter_critical_section)(LPCRITICAL_SECTION) = NULL;

/* Handle to kernel32.dll */
static HINSTANCE _pthread_h_kernel32;

#ifdef _WIN32
/* 
 * lpvReserved yields an unreferenced formal parameter;
 * ignore it
 */
#pragma warning( disable : 4100 )
#endif

BOOL WINAPI
DllMain (
	  HINSTANCE hinstDll,
	  DWORD fdwReason,
	  LPVOID lpvReserved
)
{
  BOOL result = TRUE;

  switch (fdwReason)
    {

    case DLL_PROCESS_ATTACH:
      /*
       * The DLL is being mapped into the process's address space
       */
      result = _pthread_processInitialize ();

      /* Load KERNEL32 and try to get address of TryEnterCriticalSection */
      _pthread_h_kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
      _pthread_try_enter_critical_section =
	(void *) GetProcAddress(_pthread_h_kernel32,
				"TryEnterCriticalSection");
      break;

    case DLL_THREAD_ATTACH:
      /*
       * A thread is being created
       */
      result = TRUE;
      break;

    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
      /*
       * A thread is exiting cleanly
       * NOTE: The "main" thread detaches using
       *               DLL_PROCESS_DETACH
       */
      {
	pthread_t self;

	if (_pthread_processInitialized)
	  {
#if defined( KLUDGE )
	    _pthread_cleanupStack ();
#endif /* KLUDGE */

	    self = (pthread_t) pthread_getspecific (_pthread_selfThreadKey);

	    /*
	     * Detached threads have their resources automatically
	     * cleaned up upon exit (others must be 'joined'
	     */
	    if (self != NULL &&
		self->detachState == PTHREAD_CREATE_DETACHED)
	      {

		pthread_setspecific (_pthread_selfThreadKey, NULL);

		_pthread_threadDestroy (self);
	      }

	    if (fdwReason == DLL_PROCESS_DETACH)
	      {
		/*
		 * The DLL is being unmapped into the process's address space
		 */
		_pthread_processTerminate ();
	      }
	  }

	(void) FreeLibrary(_pthread_h_kernel32);

	result = TRUE;
      }
      break;
    }
  return (result);

}				/* DllMain */



#if 0 /* Pre Bossom */

/* We use the DLL entry point function to set up per thread storage
   specifically to hold the threads own thread ID.

   The thread ID is stored by _pthread_start_call().

   The thread ID is retrieved by pthread_self().

 */

/* Global index for TLS data. */
DWORD _pthread_threadID_TlsIndex;

/* Global index for thread TSD key array. */
DWORD _pthread_TSD_keys_TlsIndex;


/* Function pointer to TryEnterCriticalSection if it exists; otherwise NULL */
BOOL (WINAPI *_pthread_try_enter_critical_section)(LPCRITICAL_SECTION) = NULL;

/* Handle to kernel32.dll */
static HINSTANCE _pthread_h_kernel32;

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
      /* Set up per thread thread ID storage. */
      _pthread_threadID_TlsIndex = TlsAlloc();

      if (_pthread_threadID_TlsIndex == 0xFFFFFFFF)
	{
	  return FALSE;
	}

      /* Set up per thread TSD key array pointer. */
      _pthread_TSD_keys_TlsIndex = TlsAlloc();

      if (_pthread_TSD_keys_TlsIndex == 0xFFFFFFFF)
	{
	  return FALSE;
	}

      /* Load KERNEL32 and try to get address of TryEnterCriticalSection */
      _pthread_h_kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
      _pthread_try_enter_critical_section = (void *) GetProcAddress(_pthread_h_kernel32, "TryEnterCriticalSection");
      break;

    case DLL_PROCESS_DETACH:
      (void) TlsFree(_pthread_TSD_keys_TlsIndex);
      (void) TlsFree(_pthread_threadID_TlsIndex);
      (void) FreeLibrary(_pthread_h_kernel32);
      break;

    default:
      return FALSE;
    }

  return TRUE;
}

#endif /* Pre Bossom */
