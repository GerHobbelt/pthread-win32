/*
 * misc.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

int
pthread_once(pthread_once_t *once_control,
	     void (*init_routine)(void))
{
  /* A flag, allocated per invocation, that indicates if the atomic
     test-and-set occured. */
  unsigned short flag = 0;

  if (once_control == NULL || init_routine == NULL)
    {
      return EINVAL;
    }

  /* An atomic test-and-set of the "once" flag. */
  pthread_mutex_lock(&once_control->lock);
  if (once_control->flag == 0)
    {
      flag = once_control->flag = 1;
    }
  pthread_mutex_unlock(&once_control->lock);

  if (flag)
    {
      /* Run the init routine. */
      init_routine();
    }
  
  return 0;
}

/*
 * Code contributed by John E. Bossom <JEB>.
 */

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

  if ((self = pthread_getspecific (_pthread_selfThreadKey)) == NULL)
    {
      /*
       * Need to create an implicit 'self' for the currently
       * executing thread.
       */
      self = (pthread_t) calloc (1, sizeof (*self));
      if (self != NULL)
	{

	  self->implicit = 1;
	  self->detachState = PTHREAD_CREATE_DETACHED;

	  self->thread = GetCurrentThreadId ();
	  self->threadH = GetCurrentThread ();
	}

      pthread_setspecific (_pthread_selfThreadKey, self);
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
      *      This function returns zero if t1 and t2 are equal, else
      *      returns nonzero.
      *
      * RESULTS
      *              0               if t1 and t2 refer to the same thread,
      *              non-zero        t1 and t2 do not refer to the same thread
      *
      * ------------------------------------------------------
      */
{
  int result;

  result = !((t1 == t2) || (t1->thread == t2->thread));

  return (result);

}				/* pthread_equal */


int
pthreadCancelableWait (HANDLE waitHandle, DWORD abstime)
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

  if ((self = pthread_getspecific (_pthread_selfThreadKey)) != NULL)
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
                                    abstime);


  if (status == WAIT_FAILED)
    {
      result = EINVAL;

    }
  else if (status == WAIT_ABANDONED_0)
    {
      result = ETIMEDOUT;

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
              DWORD exceptionInformation[3];

              exceptionInformation[0] = (DWORD) (0);
              exceptionInformation[1] = (DWORD) (0);

              RaiseException (
                               EXCEPTION_PTHREAD_SERVICES,
                               0,
                               3,
                               exceptionInformation);
            }


          ((void *) -1);
          break;

        default:
          result = EINVAL;
          break;
        }
    }

  return (result);

}                               /* pthreadCancelableWait */


/* </JEB> */

