/*
 * misc.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
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
pthread_once (
               pthread_once_t * once_control,
               void (*init_routine) (void)
)
        /*
         * ------------------------------------------------------
         * DOCPUBLIC
         *      If any thread in a process  with  a  once_control  parameter
         *      makes  a  call to pthread_once(), the first call will summon
         *      the init_routine(), but  subsequent  calls  will  not. The
         *      once_control  parameter  determines  whether  the associated
         *      initialization routine has been called.  The  init_routine()
         *      is complete upon return of pthread_once().
         *      This function guarantees that one and only one thread
         *      executes the initialization routine, init_routine when
         *      access is controlled by the pthread_once_t control
         *      key.
         *
         * PARAMETERS
         *      once_control
         *              pointer to an instance of pthread_once_t
         *
         *      init_routine
         *              pointer to an initialization routine
         *
         *
         * DESCRIPTION
         *      See above.
         *
         * RESULTS
         *              0               success,
         *              EINVAL          once_control or init_routine is NULL
         *
         * ------------------------------------------------------
         */
{
  int result;

  if (once_control == NULL || init_routine == NULL)
    {

      result = EINVAL;
      goto FAIL0;

    }
  else
    {
      result = 0;
    }

  if (!once_control->done)
    {
      if (InterlockedIncrement (&(once_control->started)) == 0)
        {
          /*
           * First thread to increment the started variable
           */
          (*init_routine) ();
          once_control->done = TRUE;

        }
      else
        {
          /*
           * Block until other thread finishes executing the onceRoutine
           */
          while (!(once_control->done))
            {
              /*
               * The following gives up CPU cycles without pausing
               * unnecessarily
               */
              Sleep (0);
            }
        }
    }

  /*
   * Fall through Intentionally
   */

  /*
   * ------------
   * Failure Code
   * ------------
   */
FAIL0:
  return (result);

}                               /* pthread_once */


pthread_t
pthread_self (void)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function returns a reference to the current running
      *      thread.
      *
      * PARAMETERS
      *      N/A
      *
      *
      * DESCRIPTION
      *      This function returns a reference to the current running
      *      thread.
      *
      * RESULTS
      *              pthread_t       reference to the current thread
      *
      * ------------------------------------------------------
      */
{
  pthread_t self = NULL;
  /*
   * need to ensure there always is a self
   */

  if ((self = (pthread_t) pthread_getspecific (ptw32_selfThreadKey)) 
      == NULL)
    {
      /*
       * Need to create an implicit 'self' for the currently
       * executing thread.
       */
      self = ptw32_new();

      if (self != NULL)
	{
	  /*
	   * This is a non-POSIX thread which has chosen to call
	   * a POSIX threads function for some reason. We assume that
	   * it isn't joinable, but we do assume that it's
	   * (deferred) cancelable.
	   */
	  self->implicit = 1;
	  self->detachState = PTHREAD_CREATE_DETACHED;
	  self->thread = GetCurrentThreadId ();

#ifdef NEED_DUPLICATEHANDLE
	  /* 
	   * DuplicateHandle does not exist on WinCE.
	   *
	   * NOTE:
	   * GetCurrentThread only returns a pseudo-handle
	   * which is only valid in the current thread context.
	   * Therefore, you should not pass the handle to
	   * other threads for whatever purpose.
	   */
	  self->threadH = GetCurrentThread();
#else
	  if( !DuplicateHandle(
			       GetCurrentProcess(),
			       GetCurrentThread(),
			       GetCurrentProcess(),
			       &self->threadH,
			       0,
			       FALSE,
			       DUPLICATE_SAME_ACCESS ) )
	    {
	      free( self );
	      return (NULL);
	    }
#endif
	}

      pthread_setspecific (ptw32_selfThreadKey, self);
    }

  return (self);

}				/* pthread_self */

int
pthread_equal (pthread_t t1, pthread_t t2)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function returns zero if t1 and t2 are equal, else
      *      returns nonzero
      *
      * PARAMETERS
      *      t1,
      *      t2
      *              references to an instances of thread_t
      *
      *
      * DESCRIPTION
      *      This function returns nonzero if t1 and t2 are equal, else
      *      returns zero.
      *
      * RESULTS
      *              non-zero        if t1 and t2 refer to the same thread,
      *              0               t1 and t2 do not refer to the same thread
      *
      * ------------------------------------------------------
      */
{
  int result;

  /*
   * We also accept NULL == NULL - treating NULL as a thread
   * for this special case, because there is no error that we can return.
   */
  result = ( ( t1 == t2 ) && ( t1 == NULL || ( t1->thread == t2->thread ) ) );

  return (result);

}				/* pthread_equal */


static int
ptw32_cancelable_wait (HANDLE waitHandle, DWORD timeout)
     /*
      * -------------------------------------------------------------------
      * This provides an extra hook into the pthread_cancel
      * mechanism that will allow you to wait on a Windows handle and make it a
      * cancellation point. This function blocks until the given WIN32 handle is
      * signaled or pthread_cancel has been called. It is implemented using
      * WaitForMultipleObjects on 'waitHandle' and a manually reset WIN32
      * event used to implement pthread_cancel.
      * 
      * Given this hook it would be possible to implement more of the cancellation
      * points.
      * -------------------------------------------------------------------
      */
{
  int result;
  pthread_t self;
  HANDLE handles[2];
  DWORD nHandles = 1;
  DWORD status;

  handles[0] = waitHandle;

  if ((self = pthread_self()) != NULL)
    {
      /*
       * Get cancelEvent handle
       */
      if (self->cancelState == PTHREAD_CANCEL_ENABLE)
        {

          if ((handles[1] = self->cancelEvent) != NULL)
            {
              nHandles++;
            }
        }
    }
  else
    {
      handles[1] = NULL;
    }

  status = WaitForMultipleObjects (
                                    nHandles,
                                    handles,
                                    FALSE,
                                    timeout);


  if (status == WAIT_FAILED)
    {
      result = EINVAL;
    }
  else if (status == WAIT_TIMEOUT)
    {
      result = ETIMEDOUT;
    }
  else if (status == WAIT_ABANDONED_0)
    {
      result = EINVAL;
    }
  else
    {
      /*
       * Either got the mutex or the cancel event
       * was signaled
       */
      switch (status - WAIT_OBJECT_0)
        {

        case 0:
          /*
           * Got the mutex
           */
          result = 0;
          break;

        case 1:
          /*
           * Got cancel request
           */
          ResetEvent (handles[1]);

          if (self != NULL && !self->implicit)
            {
              /*
               * Thread started with pthread_create
               */
	      ptw32_throw(PTW32_EPS_CANCEL);
            }

          /* Should never get to here. */
          result = EINVAL;
          break;

        default:
          result = EINVAL;
          break;
        }
    }

  return (result);

}                               /* CancelableWait */

int
pthreadCancelableWait (HANDLE waitHandle)
{
  return (ptw32_cancelable_wait(waitHandle, INFINITE));
}

int
pthreadCancelableTimedWait (HANDLE waitHandle, DWORD timeout)
{
  return (ptw32_cancelable_wait(waitHandle, timeout));
}


pthread_t
ptw32_new (void)
{
  pthread_t t;

  t = (pthread_t) calloc (1, sizeof (*t));

  if (t != NULL)
    {
      t->detachState = PTHREAD_CREATE_JOINABLE;
      t->cancelState = PTHREAD_CANCEL_ENABLE;
      t->cancelType  = PTHREAD_CANCEL_DEFERRED;
      t->cancelLock  = PTHREAD_MUTEX_INITIALIZER;
    }

  return t;

}


#ifdef NEED_CALLOC
void *
ptw32_calloc(size_t n, size_t s) {
	unsigned int m = n*s;
	void *p;
	
	p = malloc(m);
	if (p == NULL) return NULL;
	
	memset(p, 0, m);

	return p;
}
#endif
