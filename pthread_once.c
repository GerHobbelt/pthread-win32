/*
 * pthread_once.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@callisto.canberra.edu.au
 * 
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include "pthread.h"
#include "implement.h"


static void
ptw32_once_init_routine_cleanup(void * arg)
{
  pthread_once_t * once_control = (pthread_once_t *) arg;

  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, (LONG)PTW32_ONCE_CANCELLED);
  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->started, (LONG)PTW32_FALSE);

  EnterCriticalSection(&ptw32_once_event_lock);
  if (once_control->event)
    {
      /*
       * There are waiters, wake some up
       * We're deliberately not using PulseEvent. It's iffy, and deprecated.
       */
      SetEvent(once_control->event);
    }
  LeaveCriticalSection(&ptw32_once_event_lock);
}


int
pthread_once (pthread_once_t * once_control, void (*init_routine) (void))
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
	 *      pthread_once() is not a cancelation point, but the init_routine
	 *      can be. If it's cancelled then the effect on the once_control is
	 *      as if pthread_once had never been entered.
	 *
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

  while (!(InterlockedExchangeAdd((LPLONG)&once_control->state, 0L) /* Atomic Read */
	   & (LONG)PTW32_ONCE_DONE))
    {
      if (!PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->started, (LONG)PTW32_TRUE))
	{
	  /*
	   * Clear residual state from a cancelled init_routine
	   * (and DONE still hasn't been set of course).
	   */
	  if (PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, (LONG)PTW32_ONCE_CLEAR)
	      & PTW32_ONCE_CANCELLED)
	    {
	      /*
	       * The previous initter was cancelled.
	       * We now have a new initter (us) and we need to make the rest wait again.
	       */
	      EnterCriticalSection(&ptw32_once_event_lock);
	      if (once_control->event)
		{
		  ResetEvent(once_control->event);
		}
	      LeaveCriticalSection(&ptw32_once_event_lock);

	      /*
	       * Any threads entering the wait section and getting out again before
	       * the CANCELLED state can be cleared and the event is reset will, at worst, just go
	       * around again or, if they suspend and we (the initter) completes before they resume,
	       * they will see state == DONE and leave immediately.
	       */
	    }

	  pthread_cleanup_push(ptw32_once_init_routine_cleanup, (void *) once_control);
	  (*init_routine)();
	  pthread_cleanup_pop(0);

	  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, (LONG)PTW32_ONCE_DONE);

	  /*
	   * we didn't create the event.
	   * it is only there if there is someone waiting
	   */
	  EnterCriticalSection(&ptw32_once_event_lock);
	  if (once_control->event)
	    {
	      SetEvent(once_control->event);
	    }
	  LeaveCriticalSection(&ptw32_once_event_lock);
	}
      else
	{
	  /*
	   * wait for init.
	   * while waiting, create an event to wait on
	   */

	  EnterCriticalSection(&ptw32_once_event_lock);
	  once_control->eventUsers++;

	  /*
	   * RE CANCELLATION:
	   * If we are the first thread after the initter thread, and the init_routine is cancelled
	   * while we're suspended at this point in the code:-
	   * - state will not get set to PTW32_ONCE_DONE;
	   * - cleanup will not see an event and cannot set it;
	   * - therefore, we will eventually resume, create an event and wait on it, maybe forever;
	   * Remedy: cleanup must set state == CANCELLED before checking for an event, so that
	   * we will see it and avoid waiting (as for state == DONE). We will go around again and
	   * we may become the initter.
	   * If we are still the only other thread when we get to the end of this block, we will
	   * have closed the event (good). If another thread beats us to be initter, then we will
	   * re-enter here (good). In case the old event is reused, the event is always reset by
	   * the new initter after clearing the CANCELLED state, causing any threads that are
	   * cycling around the loop to wait again.
	   */

	  if (!once_control->event)
	    {
	      once_control->event = CreateEvent(NULL, PTW32_TRUE, PTW32_FALSE, NULL);
	    }
	  LeaveCriticalSection(&ptw32_once_event_lock);

	  /*
	   * check 'state' again in case the initting thread has finished or cancelled
	   * and left before seeing that there was an event to trigger.
	   * (Now that the event IS created, if init gets finished AFTER this,
	   * then the event handle is guaranteed to be seen and triggered).
	   */

	  if (!InterlockedExchangeAdd((LPLONG)&once_control->state, 0L)) /* Atomic Read */
	    {
	      /* Neither DONE nor CANCELLED */
	      (void) WaitForSingleObject(once_control->event, INFINITE);
	    }

	  /* last one out shut off the lights */
	  EnterCriticalSection(&ptw32_once_event_lock);
	  if (0 == --once_control->eventUsers)
	    {
	      /* we were last */
	      CloseHandle(once_control->event);
	      once_control->event = 0;
	    }
	  LeaveCriticalSection(&ptw32_once_event_lock);
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

}				/* pthread_once */
