/*
 * cancel.c
 *
 * Description:
 * POSIX thread functions related to thread cancellation.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

/*
 * Code contributed by John E. Bossom <JEB>.
 */

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
      *              PTHRAD_CANCEL_ASYNCHRONOUS
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

  if ((self = pthread_getspecific (_pthread_selfThreadKey)) != NULL)
    {

      if (self->cancelState == PTHREAD_CANCEL_ENABLE)
	{

	  if (WaitForSingleObject (self->cancelEvent, 0) ==
	      WAIT_OBJECT_0)
	    {
	      /*
	       * Canceling!
	       */
	      DWORD exceptionInformation[3];

	      exceptionInformation[0] = (DWORD) (0);
	      exceptionInformation[1] = (DWORD) (0);

	      RaiseException (
			       EXCEPTION_PTHREAD_SERVICES,
			       0,
			       3,
			       exceptionInformation);
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
      *              0               successfully created semaphore,
      *              ESRCH           no thread found corresponding to 'thread',
      *
      * ------------------------------------------------------
      */
{
  int result;

  if (thread != NULL)
    {

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

/* </JEB> */

#if 0 /* Pre Bossom */

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

#endif /* Pre Bossom */
