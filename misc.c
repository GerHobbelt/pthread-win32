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
  EnterCriticalSection(once_control.lock);
  if (_pthread_once_flag == 0)
    {
      flag = once_control->flag = 1;
    }
  LeaveCriticalSection(once_control.lock);

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

  /* FIXME: Need a new lookup method with the new thread allocation
     scheme.

     We can use the Win32 handle though as a basis (perhaps
     to look up a table) because pthread_self() will never be called
     after the Win32 thread has terminated (unless we can raise
     ourselves from the dead!), and therefore the Win32 handle cannot
     have been reused yet. */

#if 0
  return GetCurrentThread();
#endif
}

int
pthread_equal(pthread_t t1, pthread_t t2)
{
  return (t1 != t2);
}
