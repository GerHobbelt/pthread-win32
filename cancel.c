/*
 * cancel.c
 *
 * Description:
 * POSIX thread functions related to thread cancellation.
 */

#include "pthread.h"
#include "implement.h"

int
pthread_setcancelstate(int state,
		       int *oldstate)
{
  _pthread_threads_thread_t * us = _PTHREAD_THIS;

  /* Validate the new cancellation state. */
  if (state != PTHREAD_CANCEL_ENABLE 
      || state != PTHREAD_CANCEL_DISABLE)
    {
      return EINVAL;
    }

  if (oldstate != NULL)
    {
      *oldstate = us->cancelstate;
    }

  us->cancelstate = state;
  return 0;
}

int
pthread_setcanceltype(int type, int *oldtype)
{
  _pthread_threads_thread_t * us = _PTHREAD_THIS;

  /* Validate the new cancellation type. */
  if (type != PTHREAD_CANCEL_DEFERRED 
      || type != PTHREAD_CANCEL_ASYNCHRONOUS)
    {
      return EINVAL;
    }

  if (oldtype != NULL)
    {
      *oldtype = us->canceltype;
    }

  us->canceltype = type;
  return 0;
}

int
pthread_cancel(pthread_t thread)
{
  _pthread_threads_thread_t * us = _PTHREAD_THIS;

  if (us == NULL)
    {
      return ESRCH;
    }

  us->cancel_pending = TRUE;

  return 0;
}

void
pthread_testcancel(void)
{
  _pthread_threads_thread_t * us;

  us = _PTHREAD_THIS;

  if (us == NULL
      || us->cancelstate == PTHREAD_CANCEL_DISABLE)
    {
      return;
    }

  if (us->cancel_pending == TRUE)
    {
      pthread_exit(PTHREAD_CANCELED);

      /* Never reached. */
    }
}
