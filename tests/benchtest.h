/****************************************************************************************/

#include "../config.h"

enum {
  OLD_WIN32CS,
  OLD_WIN32MUTEX
};

static int old_mutex_use = OLD_WIN32CS;

struct old_mutex_t_ {
  HANDLE mutex;
  CRITICAL_SECTION cs;
};

typedef struct old_mutex_t_ * old_mutex_t;

struct old_mutexattr_t_ {
  int pshared;
};

typedef struct old_mutexattr_t_ * old_mutexattr_t;

static BOOL (WINAPI *ptw32_try_enter_critical_section)(LPCRITICAL_SECTION) = NULL;
static HINSTANCE ptw32_h_kernel32;

#define PTW32_OBJECT_AUTO_INIT ((void *) -1)

static int
old_mutex_init(old_mutex_t *mutex, const old_mutexattr_t *attr)
{
  int result = 0;
  old_mutex_t mx;

  if (mutex == NULL)
    {
      return EINVAL;
    }

  mx = (old_mutex_t) calloc(1, sizeof(*mx));

  if (mx == NULL)
    {
      result = ENOMEM;
      goto FAIL0;
    }

  mx->mutex = 0;

  if (attr != NULL
      && *attr != NULL
      && (*attr)->pshared == PTHREAD_PROCESS_SHARED
      )
    {
      result = ENOSYS;
    }
  else
    {
        CRITICAL_SECTION cs;

        /*
         * Load KERNEL32 and try to get address of TryEnterCriticalSection
         */
        ptw32_h_kernel32 = LoadLibrary(TEXT("KERNEL32.DLL"));
        ptw32_try_enter_critical_section = (BOOL (WINAPI *)(LPCRITICAL_SECTION))

#if defined(NEED_UNICODE_CONSTS)
        GetProcAddress(ptw32_h_kernel32,
                       (const TCHAR *)TEXT("TryEnterCriticalSection"));
#else
        GetProcAddress(ptw32_h_kernel32,
                       (LPCSTR) "TryEnterCriticalSection");
#endif

        if (ptw32_try_enter_critical_section != NULL)
          {
            InitializeCriticalSection(&cs);
            if ((*ptw32_try_enter_critical_section)(&cs))
              {
                LeaveCriticalSection(&cs);
              }
            else
              {
                /*
                 * Not really supported (Win98?).
                 */
                ptw32_try_enter_critical_section = NULL;
              }
            DeleteCriticalSection(&cs);
          }

        if (ptw32_try_enter_critical_section == NULL)
          {
            (void) FreeLibrary(ptw32_h_kernel32);
            ptw32_h_kernel32 = 0;
          }

      if (old_mutex_use == OLD_WIN32CS)
	{
	  InitializeCriticalSection(&mx->cs);
	}
      else if (old_mutex_use == OLD_WIN32MUTEX)
      {
	  mx->mutex = CreateMutex (NULL,
				   FALSE,
				   NULL);

	  if (mx->mutex == 0)
	    {
	      result = EAGAIN;
	    }
	}
      else
	{
        result = EINVAL;
      }
    }

  if (result != 0 && mx != NULL)
    {
      free(mx);
      mx = NULL;
    }

FAIL0:
  *mutex = mx;

  return(result);
}


static int
old_mutex_lock(old_mutex_t *mutex)
{
  int result = 0;
  old_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  if (*mutex == (old_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      /*
       * Don't use initialisers when benchtesting.
       */
      result = EINVAL;
    }

  mx = *mutex;

  if (result == 0)
    {
      if (mx->mutex == 0)
	{
	  EnterCriticalSection(&mx->cs);
	}
      else
	{
	  result = (WaitForSingleObject(mx->mutex, INFINITE) 
		    == WAIT_OBJECT_0)
	    ? 0
	    : EINVAL;
	}
    }

  return(result);
}

static int
old_mutex_unlock(old_mutex_t *mutex)
{
  int result = 0;
  old_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  mx = *mutex;

  if (mx != (old_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      if (mx->mutex == 0)
	{
	  LeaveCriticalSection(&mx->cs);
	}
      else
	{
	  result = (ReleaseMutex (mx->mutex) ? 0 : EINVAL);
	}
    }
  else
    {
      result = EINVAL;
    }

  return(result);
}


static int
old_mutex_trylock(old_mutex_t *mutex)
{
  int result = 0;
  old_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  if (*mutex == (old_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      /*
       * Don't use initialisers when benchtesting.
       */
      result = EINVAL;
    }

  mx = *mutex;

  if (result == 0)
    {
      if (mx->mutex == 0)
	{
	  if (ptw32_try_enter_critical_section == NULL)
          {
            result = 0;
          }
        else if ((*ptw32_try_enter_critical_section)(&mx->cs) != TRUE)
	    {
	      result = EBUSY;
	    }
	}
      else
	{
	  DWORD status;

	  status = WaitForSingleObject (mx->mutex, 0);

	  if (status != WAIT_OBJECT_0)
	    {
	      result = ((status == WAIT_TIMEOUT)
			? EBUSY
			: EINVAL);
	    }
	}
    }

  return(result);
}


static int
old_mutex_destroy(old_mutex_t *mutex)
{
  int result = 0;
  old_mutex_t mx;

  if (mutex == NULL
      || *mutex == NULL)
    {
      return EINVAL;
    }

  if (*mutex != (old_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      mx = *mutex;

      if ((result = old_mutex_trylock(&mx)) == 0)
        {
          *mutex = NULL;

          (void) old_mutex_unlock(&mx);

          if (mx->mutex == 0)
            {
              DeleteCriticalSection(&mx->cs);
            }
          else
            {
              result = (CloseHandle (mx->mutex) ? 0 : EINVAL);
            }

          if (result == 0)
            {
              mx->mutex = 0;
              free(mx);
            }
          else
            {
              *mutex = mx;
            }
        }
    }
  else
    {
      result = EINVAL;
    }

  if (ptw32_try_enter_critical_section != NULL)
    {
      (void) FreeLibrary(ptw32_h_kernel32);
      ptw32_h_kernel32 = 0;
    }

  return(result);
}

/****************************************************************************************/
