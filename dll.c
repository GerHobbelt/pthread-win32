/*
 * dll.c
 *
 * Description:
 * This translation unit implements DLL initialisation.
 */

#include <malloc.h>
#include "pthread.h"
#include "implement.h"


/* 
 * Function pointer to TryEnterCriticalSection if it exists; otherwise NULL 
 */
BOOL (WINAPI *_pthread_try_enter_critical_section)(LPCRITICAL_SECTION) = NULL;

/*
 * Handle to kernel32.dll 
 */
static HINSTANCE _pthread_h_kernel32;


#ifdef _MSC_VER
/* 
 * lpvReserved yields an unreferenced formal parameter;
 * ignore it
 */
#pragma warning( disable : 4100 )
#endif

#ifdef __cplusplus
/*
 * Dear c++: Please don't mangle this name. -thanks
 */
extern "C"
{
#endif /* __cplusplus */

  BOOL WINAPI DllMain( HINSTANCE, DWORD, LPVOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

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
	(BOOL (PT_STDCALL *)(LPCRITICAL_SECTION))
	GetProcAddress(_pthread_h_kernel32,
		       (LPCSTR) "TryEnterCriticalSection");
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

		(void) FreeLibrary(_pthread_h_kernel32);
	      }
	  }

	result = TRUE;
      }
      break;
    }
  return (result);

}				/* DllMain */
