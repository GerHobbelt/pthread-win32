/*
 * misc.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 */

#include <errno.h>

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
  EnterCriticalSection(&once_control->lock);
  if (once_control->flag == 0)
    {
      flag = once_control->flag = 1;
    }
  LeaveCriticalSection(&once_control->lock);

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
  pthread_t ret;
  /* This TLS index is allocated on DLL load by dll.c */
  extern DWORD _pthread_threadID_TlsIndex;
 
  ret = (pthread_t) TlsGetValue(_pthread_threadID_TlsIndex);

  if (ret == 0)
    {
      /* FIXME: Oh no! This can't happen. */
    }

  return ret;
}

int
pthread_equal(pthread_t t1, pthread_t t2)
{
  return (t1 != t2);
}
