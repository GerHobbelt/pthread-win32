/*
 * sync.c
 *
 * Description:
 * This translation unit implements functions related to thread
 * synchronisation.
 */

#include "pthread.h"
#include "implement.h"

/*
 * Code contributed by John E. Bossom <JEB>.
 */

int
pthread_detach (pthread_t tid)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function detaches the given thread.
      *
      * PARAMETERS
      *      thread
      *              an instance of a pthread_t
      *
      *
      * DESCRIPTION
      *      This function detaches the given thread. You may
      *      detach the main thread or to detach a joinable thread
      *      (You should have used pthread_attr_t to create the
      *      thread as detached!)
      *      NOTE:   detached threads cannot be joined nor canceled;
      *                      storage is freed immediately on termination.
      *
      * RESULTS
      *              0               successfully detached the thread,
      *              EINVAL          thread is not a joinable thread,
      *              ENOSPC          a required resource has been exhausted,
      *              ESRCH           no thread could be found for 'thread',
      *
      * ------------------------------------------------------
      */
{
  int result;

  if (tid == NULL ||
      tid->detachState == PTHREAD_CREATE_DETACHED)
    {

      result = EINVAL;

    }
  else
    {
      result = 0;
      tid->detachState = PTHREAD_CREATE_DETACHED;
    }

  return (result);

}				/* pthread_detach */

int
pthread_join (pthread_t thread, void **value_ptr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function waits for 'thread' to terminate and
      *      returns the thread's exit value if 'value_ptr' is not
      *      NULL. This also detaches the thread on successful
      *      completion.
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      *
      * DESCRIPTION
      *      This function waits for 'thread' to terminate and
      *      returns the thread's exit value if 'value_ptr' is not
      *      NULL. This also detaches the thread on successful
      *      completion.
      *      NOTE:   detached threads cannot be joined or canceled
      *
      * RESULTS
      *              0               'thread' has completed
      *              EINVAL          thread is not a joinable thread,
      *              ESRCH           no thread could be found with ID 'thread',
      *              EDEADLK         attempt to join thread with self
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_t self;

  self = pthread_self ();

  if (pthread_equal (self, thread) == 0)
    {
      result = EDEADLK;

    }
  else
    {
      DWORD stat;

      stat = WaitForSingleObject (thread->threadH, INFINITE);

      if (stat != WAIT_OBJECT_0 &&
	  !GetExitCodeThread (thread->threadH, (LPDWORD) value_ptr))
	{
	  result = ESRCH;
	}
      else
	{
	  _pthread_threadDestroy (thread);
	}
    }

  return (result);

}				/* pthread_join */

/* </JEB> */

