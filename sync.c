/*
 * sync.c
 *
 * Description:
 * This translation unit implements functions related to thread
 * synchronisation.
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA
 */

#include "pthread.h"
#include "implement.h"

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
      *      thread
      *              an instance of pthread_t
      *
      *      value_ptr
      *              pointer to an instance of pointer to void
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
      *              ENOENT          thread couldn't find it's own valid handle,
      *              EDEADLK         attempt to join thread with self
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_t self;

  self = pthread_self ();
  if (self == NULL)
    {
       return ENOENT;
    }

  if (pthread_equal (self, thread) != 0)
    {
      result = EDEADLK;
    }
  else if (thread->detachState == PTHREAD_CREATE_DETACHED)
    {
      result = EINVAL;
    }
  else
    {
      /*
       * Pthread_join is a cancelation point.
       * If we are cancelled then our target thread must not be
       * detached (destroyed). This is guarranteed because
       * pthreadCancelableWait will not return if we
       * are cancelled.
       */
      result = pthreadCancelableWait(thread->threadH);

      if (result == 0)
	{

#if ! defined (__MINGW32__) || defined (__MSVCRT__)

	  if (value_ptr != NULL
	      && !GetExitCodeThread (thread->threadH, (LPDWORD) value_ptr))
	    {
	      result = ESRCH;
	    }
	  else
	    {
	      /*
	       * The result of making multiple simultaneous calls to
	       * pthread_join() specifying the same target is undefined.
	       */
	      ptw32_threadDestroy (thread);
	    }

#else /* __MINGW32__ && ! __MSVCRT__ */

	  /*
	   * If using CRTDLL, the thread may have exited, and endthread
	   * will have closed the handle.
	   */
	  if (value_ptr != NULL)
	    {
	      *value_ptr = self->exitStatus;
	    }
      
	  /*
	   * The result of making multiple simultaneous calls to
	   * pthread_join() specifying the same target is undefined.
	   */
	  ptw32_threadDestroy (thread);

#endif /* __MINGW32__ && ! __MSVCRT__ */

	}
      else
	{
	  result = ESRCH;
	}
    }

  return (result);

}				/* pthread_join */
