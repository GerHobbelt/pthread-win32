/*
 * create.c
 *
 * Description:
 * This translation unit implements routines associated with spawning a new
 * thread.
 */

#include "pthread.h"
#include "implement.h"

/* FIXME: There must be a Win32 routine to get this value. */
DWORD pthread_threads_count = 0;

int
pthread_create(pthread_t *thread, const pthread_attr_t *attr,
	       void * (*start_routine) (void *), void * arg)
{
  /* Call Win32 CreateThread.
     Map attributes as correctly as possible.
  */
  HANDLE handle;
  DWORD  flags = 0; /* Always 0 for POSIX threads */
  DWORD  stack = PTHREAD_STACK_MIN;
  LPSECURITY_ATTRIBUTES  security = NULL;

  /* FIXME: This needs to be moved into process space. 
     Perhaps into a structure that contains all
     per thread info that is Win32 thread specific but
     not visible from the pthreads API, and
     accessible through HANDLE (or pthread_t).
   */
  SECURITY_ATTRIBUTES security_attr;
  DWORD  threadID;

  if (pthread_threads_count >= PTHREAD_THREADS_MAX)
    return EAGAIN;

  switch (attr)
    {
    case NULL:
      /* Use POSIX default attributes */
      break;
    default:
      /* Map attributes */
      if (attr->stacksize != NULL)
	stack = (DWORD) attr->stacksize;
      break;
    }

  /* FIXME: I don't have error return values to work with so
     I'm assuming this always succeeds (obviously not).
   */
  handle = CreateThread(security,
			stack,
			start_routine,
			arg,
			flags,
			&threadID);

  *thread = (pthread_t) handle;
  pthread_threads_count++;
  return 0;
}

