/*
 * tryentercs.c
 *
 * See if we have the TryEnterCriticalSection function.
 * Does not use any part of pthreads.
 */

#include <windows.h>
#include <process.h>
#include <stdio.h>

/*
 * Function pointer to TryEnterCriticalSection if it exists
 * - otherwise NULL
 */
BOOL (WINAPI *_try_enter_critical_section)(LPCRITICAL_SECTION) =
NULL;

/*
 * Handle to kernel32.dll
 */
static HINSTANCE _h_kernel32;


int
main()
{
  CRITICAL_SECTION cs;

  SetLastError(0);

  printf("Last Error [main enter] %ld\n", (long) GetLastError());

  /*
   * Load KERNEL32 and try to get address of TryEnterCriticalSection
   */
  _h_kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
  _try_enter_critical_section =
        (BOOL (PT_STDCALL *)(LPCRITICAL_SECTION))
        GetProcAddress(_h_kernel32,
                         (LPCSTR) "TryEnterCriticalSection");

  if (_try_enter_critical_section != NULL)
    {
      InitializeCriticalSection(&cs);

      SetLastError(0);

      if ((*_try_enter_critical_section)(&cs) != 0)
        {
          LeaveCriticalSection(&cs);
        }
	else
	  {
	    printf("Last Error [try enter] %ld\n", (long) GetLastError());

	    _try_enter_critical_section = NULL;
	  }
      DeleteCriticalSection(&cs);
    }

  (void) FreeLibrary(_h_kernel32);

  printf("This system %s TryEnterCriticalSection.\n",
         (_try_enter_critical_section == NULL) ? "DOES NOT SUPPORT" : "SUPPORTS");
  printf("POSIX Mutexes will be based on Win32 %s.\n",
         (_try_enter_critical_section == NULL) ? "Mutexes" : "Critical Sections");

  return(0);
}

