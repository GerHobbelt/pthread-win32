/*
 * cancel.c
 *
 * Description:
 * POSIX thread functions related to thread cancellation.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

int
pthread_setcancelstate(int state,
		       int *oldstate)
{
  pthread_t us = pthread_self();

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
  pthread_t us = pthread_self();

  /* Validate the new cancellation type. */
  if (type == PTHREAD_CANCEL_ASYNCHRONOUS ||
      type != PTHREAD_CANCEL_DEFERRED)
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
  if (_PTHREAD_VALID(thread)
      && thread->ptstatus != _PTHREAD_REUSE)
    {
      thread->cancel_pending = TRUE;
      return 0;
    }

  return ESRCH;
}

void
pthread_testcancel(void)
{
  pthread_t thread = pthread_self();

  if (thread->cancelstate == PTHREAD_CANCEL_DISABLE)
    {
      return;
    }

  if (thread->cancel_pending == TRUE)
    {
      pthread_exit(PTHREAD_CANCELED);
    }
  /* Never reached. */
}
