/*
 * cancel.c
 *
 * Description:
 * POSIX thread functions related to thread cancellation.
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


static void 
ptw32_cancel_self(void)
{
  ptw32_throw(PTW32_EPS_CANCEL);

  /* Never reached */
}


/*
 * ptw32_cancel_thread implements asynchronous cancellation.
 */
static void 
ptw32_cancel_thread(pthread_t thread)
{
  HANDLE threadH = thread->threadH;

  (void) pthread_mutex_lock(&thread->cancelLock);

  SuspendThread(threadH);

  if (WaitForSingleObject(threadH, 0) == WAIT_TIMEOUT)
    {
#if defined(_M_IX86) || defined(_X86_)
      CONTEXT context;
      context.ContextFlags = CONTEXT_CONTROL;
      GetThreadContext(threadH, &context);
      context.Eip = (DWORD) ptw32_cancel_self;
      SetThreadContext(threadH, &context);
#endif
      ResumeThread(threadH);
    }

  (void) pthread_mutex_unlock(&thread->cancelLock);
}


int
pthread_setcancelstate (int state, int *oldstate)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function atomically sets the calling thread's
      *      cancelability state to 'state' and returns the previous
      *      cancelability state at the location referenced by
      *      'oldstate'
      *
      * PARAMETERS
      *      state,
      *      oldstate
      *              PTHREAD_CANCEL_ENABLE
      *                      cancellation is enabled,
      *
      *              PTHREAD_CANCEL_DISABLE
      *                      cancellation is disabled
      *
      *
      * DESCRIPTION
      *      This function atomically sets the calling thread's
      *      cancelability state to 'state' and returns the previous
      *      cancelability state at the location referenced by
      *      'oldstate'.
      *
      *      NOTES:
      *      1)      Use to disable cancellation around 'atomic' code that
      *              includes cancellation points
      *
      * COMPATIBILITY ADDITIONS
      *      If 'oldstate' is NULL then the previous state is not returned
      *      but the function still succeeds. (Solaris)
      *
      * RESULTS
      *              0               successfully set cancelability type,
      *              EINVAL          'state' is invalid
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_t self = pthread_self();

  if (self == NULL
      || (state != PTHREAD_CANCEL_ENABLE
	  && state != PTHREAD_CANCEL_DISABLE))
    {
      return EINVAL;
    }

  /*
   * Lock for async-cancel safety.
   */
  (void) pthread_mutex_lock(&self->cancelLock);

  if (oldstate != NULL)
    {
      *oldstate = self->cancelState;
    }

  self->cancelState = state;

  /*
   * Check if there is a pending asynchronous cancel
   */
  if (self->cancelState == PTHREAD_CANCEL_ENABLE
      && self->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS
      && WaitForSingleObject(self->cancelEvent, 0) == WAIT_OBJECT_0)
    {
      ResetEvent(self->cancelEvent);
      (void) pthread_mutex_unlock(&self->cancelLock);
      ptw32_throw(PTW32_EPS_CANCEL);

      /* Never reached */
    }

  (void) pthread_mutex_unlock(&self->cancelLock);

  return (result);

}				/* pthread_setcancelstate */


int
pthread_setcanceltype (int type, int *oldtype)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function atomically sets the calling thread's
      *      cancelability type to 'type' and returns the previous
      *      cancelability type at the location referenced by
      *      'oldtype'
      *
      * PARAMETERS
      *      type,
      *      oldtype
      *              PTHREAD_CANCEL_DEFERRED
      *                      only deferred cancelation is allowed,
      *
      *              PTHREAD_CANCEL_ASYNCHRONOUS
      *                      Asynchronous cancellation is allowed
      *
      *
      * DESCRIPTION
      *      This function atomically sets the calling thread's
      *      cancelability type to 'type' and returns the previous
      *      cancelability type at the location referenced by
      *      'oldtype'
      *
      *      NOTES:
      *      1)      Use with caution; most code is not safe for use
      *              with asynchronous cancelability.
      *
      * COMPATIBILITY ADDITIONS
      *      If 'oldtype' is NULL then the previous type is not returned
      *      but the function still succeeds. (Solaris)
      *
      * RESULTS
      *              0               successfully set cancelability type,
      *              EINVAL          'type' is invalid
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_t self = pthread_self();

  if (self == NULL
      || (type != PTHREAD_CANCEL_DEFERRED
	  && type != PTHREAD_CANCEL_ASYNCHRONOUS))
    {
      return EINVAL;
    }

  /*
   * Lock for async-cancel safety.
   */
  (void) pthread_mutex_lock(&self->cancelLock);

  if (oldtype != NULL)
    {
      *oldtype = self->cancelType;
    }

  self->cancelType = type;

  /*
   * Check if there is a pending asynchronous cancel
   */
  if (self->cancelState == PTHREAD_CANCEL_ENABLE
      && self->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS
      && WaitForSingleObject(self->cancelEvent, 0) == WAIT_OBJECT_0)
    {
      ResetEvent(self->cancelEvent);
      (void) pthread_mutex_unlock(&self->cancelLock);
      ptw32_throw(PTW32_EPS_CANCEL);

      /* Never reached */
    }

  (void) pthread_mutex_unlock(&self->cancelLock);

  return (result);

}				/* pthread_setcanceltype */

void
pthread_testcancel (void)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function creates a deferred cancellation point
      *      in the calling thread. The call has no effect if the
      *      current cancelability state is
      *              PTHREAD_CANCEL_DISABLE
      *
      * PARAMETERS
      *      N/A
      *
      *
      * DESCRIPTION
      *      This function creates a deferred cancellation point
      *      in the calling thread. The call has no effect if the
      *      current cancelability state is
      *              PTHREAD_CANCEL_DISABLE
      *
      *      NOTES:
      *      1)      Cancellation is asynchronous. Use pthread_join
      *              to wait for termination of thread if necessary
      *
      * RESULTS
      *              N/A
      *
      * ------------------------------------------------------
      */
{
  pthread_t self = pthread_self();

  if (self != NULL
      && self->cancelState == PTHREAD_CANCEL_ENABLE
      && WaitForSingleObject (self->cancelEvent, 0) == WAIT_OBJECT_0
      )
    {
      /*
       * Canceling!
       */
      ptw32_throw(PTW32_EPS_CANCEL);
    }
}				/* pthread_testcancel */

int
pthread_cancel (pthread_t thread)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function requests cancellation of 'thread'.
      *
      * PARAMETERS
      *      thread
      *              reference to an instance of pthread_t
      *
      *
      * DESCRIPTION
      *      This function requests cancellation of 'thread'.
      *      NOTE: cancellation is asynchronous; use pthread_join to
      *                wait for termination of 'thread' if necessary.
      *
      * RESULTS
      *              0               successfully requested cancellation,
      *              ESRCH           no thread found corresponding to 'thread',
      *              ENOMEM          implicit self thread create failed.
      * ------------------------------------------------------
      */
{
  int result;
  int cancel_self;
  pthread_t self;

  if (thread == NULL )
    {
      return ESRCH;
    }

  result = 0;
  if ((self = pthread_self()) == NULL)
    {
      return ENOMEM;
    };

  /*
   * FIXME!!
   *
   * Can a thread cancel itself?
   *
   * The standard doesn't
   * specify an error to be returned if the target
   * thread is itself.
   *
   * If it may, then we need to ensure that a thread can't
   * deadlock itself trying to cancel itself asyncronously
   * (pthread_cancel is required to be an async-cancel
   * safe function).
   */
  cancel_self = pthread_equal(thread, self);

  /*
   * Lock for async-cancel safety.
   */
  (void) pthread_mutex_lock(&self->cancelLock);

  if (thread->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS
      && thread->cancelState == PTHREAD_CANCEL_ENABLE )
    {
      if (cancel_self)
	{
	  (void) pthread_mutex_unlock(&self->cancelLock);
	  ptw32_throw(PTW32_EPS_CANCEL);

	  /* Never reached */
	}

      ptw32_cancel_thread(thread);
    }
  else
    {
      /*
       * Set for deferred cancellation.
       */
      if (!SetEvent (thread->cancelEvent))
	{
	  result = ESRCH;
	}
    }

  (void) pthread_mutex_unlock(&self->cancelLock);

  return (result);

}



