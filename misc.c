/*
 * misc.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 */

#include "pthread.h"

int
pthread_once(pthread_once_t *once_control,
	     void (*init_routine)(void))
{
  /* A flag, allocated per invocation, that indicates if the amotic
     test-and-set occured. */
  unsigned short flag = 0;

  if (once_control == NULL || init_routine == NULL)
    {
      return EINVAL;
    }

  /* FIXME: we are assuming that the `cs' object is initialised at DLL
     load time.  Likewise, the object should be destroyed when (if)
     the DLL is unloaded. */

  /* An atomic test-and-set of the "once" flag. */
  EnterCriticalSection(&_pthread_once_lock);
  if (_pthread_once_flag == 0)
    {
      flag = _pthread_once_flag = 1;
    }
  LeaveCriticalSection(&_pthread_once_lock);

  if (flag)
    {
      /* Run the init routine. */
      init_routine();
    }
  
  return 0;
}

pthread_t
pthread_self(void)
{
  /* It looks like a pthread_t needs to be a HANDLE, but Win32 also has
     another way of identifying threads: their thread id.  We hope
     that all of the Win32 functions we are going to use only need
     HANDLEs.  The morons. */

  return GetCurrentThread();
}

int
pthread_equal(pthread_t t1, pthread_t t2)
{
  /* For the time being, assume that HANDLEs can be directly compared.
     If not, then use the appropriate Win32 function for
     comparison. */

  return (t1 != t2);
}
