/*
 * sync.c
 *
 * Description:
 * This translation unit implements functions related to thread
 * synchronisation.
 */

#include "pthread.h"

int
pthread_join(pthread_t thread, void ** valueptr)
{
  LPDWORD exitcode;
  _pthread_threads_thread_t * target;

  pthread_t us = pthread_self();

  /* First check if we are trying to join to ourselves. */
  if (pthread_equal(thread, us) == 0)
    {
      return EDEADLK;
    }

  /* If the thread is detached, then join will return immediately. */

  target = _pthread_find_thread_entry(thread);
  if (target < 0)
    {
      return EINVAL;
    }
  else if (target->detached == PTHREAD_CREATE_DETACHED)
    {
      return ESRCH;
    }

  /* Wait on the kernel thread object. */
  switch (WaitForSingleObject(thread, INFINITE))
    {
    case WAIT_FAILED:
      /* The thread does not exist. */
      return ESRCH;
    case WAIT_OBJECT_0:
      /* The thread has finished. */
      break;
    default:
      /* This should never happen. */
      break;
    }
 
  /* We don't get the exit code as a result of the last operation,
     so we do it now. */

  if (GetExitCodeThread(thread, exitcode) != TRUE)
    {
      return ESRCH;
    }

  /* FIXME: this is wrong. */
  return &exitcode;
}

