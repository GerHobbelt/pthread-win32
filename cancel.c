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
      *      type,
      *      oldtype
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
      *      'oldstate'
      *
      *      NOTES:
      *      1)      Use to disable cancellation around 'atomic' code that
      *              includes cancellation points
      *
      * RESULTS
      *              0               successfully set cancelability type,
      *              EINVAL          'state' is invalid
      *
      * ------------------------------------------------------
      */
{
  pthread_t self;
  int result;

  if (((self = pthread_self ()) != NULL) &&
      (state == PTHREAD_CANCEL_ENABLE ||
       state == PTHREAD_CANCEL_DISABLE))
    {

      *oldstate = self->cancelState;
      self->cancelState = state;
      result = 0;

    }
  else
    {
      result = EINVAL;
    }

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
      * RESULTS
      *              0               successfully set cancelability type,
      *              EINVAL          'type' is invalid
      *
      * ------------------------------------------------------
      */
{
  pthread_t self;
  int result;

  if (((self = pthread_self ()) != NULL) &&
      (type == PTHREAD_CANCEL_DEFERRED ||
       type == PTHREAD_CANCEL_ASYNCHRONOUS))
    {

      *oldtype = self->cancelType;
      self->cancelType = type;
      result = 0;

    }
  else
    {
      result = EINVAL;
    }

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
  pthread_t self;

  if ((self = (pthread_t) pthread_getspecific (_pthread_selfThreadKey)) 
      != NULL)
    {

      if (self->cancelState == PTHREAD_CANCEL_ENABLE)
	{

	  if (WaitForSingleObject (self->cancelEvent, 0) ==
	      WAIT_OBJECT_0)
	    {
	      /*
	       * Canceling!
	       */

#ifdef _MSC_VER

	      DWORD exceptionInformation[3];

	      exceptionInformation[0] = (DWORD) (0);
	      exceptionInformation[1] = (DWORD) (0);
	      exceptionInformation[2] = (DWORD) (0);

	      RaiseException (
			       EXCEPTION_PTHREAD_SERVICES,
			       0,
			       3,
			       exceptionInformation);

#else /* _MSC_VER */

#ifdef __cplusplus

	      throw Pthread_exception();

#endif /* __cplusplus */

#endif /* _MSC_VER */

	    }
	}
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
      *
      * ------------------------------------------------------
      */
{
  int result;

  if (thread != NULL)
    {
      /*
       * Set for deferred cancellation.
       */
      if (!SetEvent (thread->cancelEvent))
	{
	  result = ESRCH;
	}
      else
	{
	  result = 0;
	}

    }
  else
    {
      result = ESRCH;
    }

  return (result);
}

