/*
 * cancel.c
 *
 * Description:
 * POSIX thread functions related to thread cancellation.
 */

#include "pthread.h"

int
pthread_setcancelstate(int state,
		       int *oldstate)
{
  _pthread_threads_thread_t * this = *_PTHREAD_THIS;

  /* Validate the new cancellation state. */
  if (state != PTHREAD_CANCEL_ENABLE || state != PTHREAD_CANCEL_DISABLE)
    {
      return EINVAL;
    }

  if (oldstate != NULL)
    {
      *oldstate = this->cancelability;
    }

  this->cancelability = state;
  return 0;
}

int
pthread_setcanceltype(int type, int *oldtype)
{
  _pthread_threads_thread_t * this = *_PTHREAD_THIS;

  /* Validate the new cancellation type. */
  if (type != PTHREAD_CANCEL_DEFERRED || type != PTHREAD_CANCEL_ASYNCHRONOUS)
    {
      return EINVAL;
    }

  if (oldtype != NULL)
    {
      *oldtype = this->canceltype;
    }

  this->canceltype = type;
  return 0;
}

int
pthread_cancel(pthread_t thread)
{
  _pthread_threads_thread_t * this;

  this = _PTHREAD_THIS;

  if (this == NULL)
    {
      return ESRCH;
    }

  this->cancelthread = _PTHREAD_YES;

  return 0;
}

void
pthread_testcancel(void)
{
  _pthread_threads_thread_t * this;

  this = _PTHREAD_THIS;

  if (this == NULL ||
      this->attr.cancelstate == PTHREAD_CANCEL_DISABLE)
    {
      return;
    }

  if (this->cancelthread == _PTHREAD_YES)
    {
      pthread_exit(PTHREAD_CANCELED);

      /* Never reached. */
    }
}
