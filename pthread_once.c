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

/*
 * NOTES:
 * pthread_once() performs a very simple task. So why is this implementation
 * so complicated?
 *
 * The original implementation WAS very simple, but it relied on Windows random
 * priority boosting to resolve starvation problems. Windows priority boosting
 * does not occur for realtime priority classes (levels 16 to 31).
 *
 * You can check back to previous versions of code in the CVS repository or
 * search the mailing list archives for discussion.
 *
 * Version A
 * ---------
 * Waiting threads would resume and suspend again using Sleep(0) until the
 * init_routine had completed, but a higher priority waiter could hog the CPU and
 * starve the initter thread until Windows randomly boosted it's priority, or forever
 * for realtime applications.
 *
 * Version B
 * ---------
 * This was fixed by introducing a per once_control manual-reset event that is
 * created and destroyed dynamically only if there are waiters. The design did not
 * need global critical sections. Each once_control remained independent. A waiter
 * could be confident that if the event was not null then it did not need to create
 * the event.
 *
 * Version C
 * ---------
 * Since a change in ABI would result from version B, it was decided to take
 * the opportunity and make pthread_once() fully compliant with the Single Unix
 * Specification (version 3 at the time). This required allowing the init_routine
 * to be a cancelation point. A cancelation meant that at least some waiting threads
 * if any had to be woken so that one might become the new initter thread.
 * Waiters could no longer simply assume that, if the event was not null, it did
 * not need to create an event.
 *
 * Also, the cancelled init thread needed to set the event, and the
 * new init thread (the winner of the race between any newly arriving threads and
 * waking waiters) would need to reset it again. In the meantime, threads could be
 * happily looping around until they either suspended on the reset event, or exited
 * because the init thread had completed. It was also once again possible for a higher
 * priority waiter to starve the init thread.
 * 
 * Version D
 * ---------
 * There were now two options considered:
 * - use an auto-reset event; OR
 * - add our own priority boosting.
 *
 * An auto-reset event would stop threads from looping ok, but it makes threads
 * dependent on earlier threads to successfully set the event in turn when it's time
 * to wake up, and this serialises threads unecessarily on MP systems. It also adds
 * an extra kernel call for each waking thread. If one waiter wakes and dies (async
 * cancelled or killed) before it can set the event, then all remaining waiters are
 * stranded.
 *
 * Priority boosting is a standard method for solving priority inversion and
 * starvation problems. Furthermore, all of the priority boost logic can
 * be restricted to the post cancellation tracks. That is, it need not slow
 * the normal cancel-free behaviour. Threads remain independent of other threads.
 *
 * The implementation below adds only a few local (to the thread) integer comparisons
 * to the normal track through the routine and additional bus locking/cache line
 * syncing operations have been avoided altogether in the uncontended track.
 */

#include "pthread.h"
#include "implement.h"


static void PTW32_CDECL
ptw32_once_init_routine_cleanup(void * arg)
{
  pthread_once_t * once_control = (pthread_once_t *) arg;

  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, (LONG)PTW32_ONCE_CANCELLED);
  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->started, (LONG)PTW32_FALSE);

  if (InterlockedExchangeAdd((LPLONG)&once_control->event, 0L)) /* MBR fence */
    {
      int lasterror = GetLastError ();
      int lastWSAerror = WSAGetLastError ();

      /*
       * There are waiters, wake some up.
       */
      if (!SetEvent(once_control->event))
	{
	  SetLastError (lasterror);
	  WSASetLastError (lastWSAerror);
	}
    }
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
  int lasterror;
  int lastWSAerror;
  int restoreLastError;
  LONG state;
  pthread_t self;
  HANDLE w32Thread = 0;

  if (once_control == NULL || init_routine == NULL)
    {
      result = EINVAL;
      goto FAIL0;
    }
  else
    {
      result = 0;
    }

  /*
   * We want to be invisible to GetLastError() outside of this routine.
   */
  lasterror = GetLastError ();
  lastWSAerror = WSAGetLastError ();
  restoreLastError = PTW32_FALSE;

  while (!((state = InterlockedExchangeAdd((LPLONG)&once_control->state, 0L)) /* Atomic Read */
	   & (LONG)PTW32_ONCE_DONE))
    {
      LONG cancelled = (state & PTW32_ONCE_CANCELLED);

      if (cancelled)
	{
	  /* Boost priority momentarily */
	  if (!w32Thread)
	    {
	      self = pthread_self();
	      w32Thread = ((ptw32_thread_t *)self.p)->threadH;
	    }
	  /*
	   * Prevent pthread_setschedparam() from changing our priority while we're boosted.
	   */
	  pthread_mutex_lock(&((ptw32_thread_t *)self.p)->threadLock);
	  SetThreadPriority(w32Thread, THREAD_PRIORITY_HIGHEST);
	}

      if (!PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->started, (LONG)PTW32_TRUE))
	{
	  if (cancelled)
	    {
	      /*
	       * The previous initter was cancelled.
	       * We now have a new initter (us) and we need to make the rest wait again.
	       * Furthermore, we're running at max priority until after we've reset the event
	       * so we will not be starved by any other threads that may now be looping
	       * around.
	       */
	      if (InterlockedExchangeAdd((LPLONG)&once_control->event, 0L)) /* MBR fence */
		{
		  if (!ResetEvent(once_control->event))
		    {
		      restoreLastError = PTW32_TRUE;
		    }
		}

	      /*
	       * Any threads entering the wait section and getting out again before
	       * the event is reset and the CANCELLED state is cleared will, at worst,
	       * just go around again or, if they suspend and we (the initter) completes before
	       * they resume, they will see state == DONE and leave immediately.
	       */
	      PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, (LONG)PTW32_ONCE_CLEAR);

	      /*
	       * Restore priority. We catch any changes to this thread's priority
	       * only if they were done through the POSIX API (i.e. pthread_setschedparam)
	       */
	      SetThreadPriority(w32Thread, ((ptw32_thread_t *)self.p)->sched_priority);
	      pthread_mutex_unlock(&((ptw32_thread_t *)self.p)->threadLock);
	    }

#ifdef _MSC_VER
#pragma inline_depth(0)
#endif

	  pthread_cleanup_push(ptw32_once_init_routine_cleanup, (void *) once_control);
	  (*init_routine)();
	  pthread_cleanup_pop(0);

#ifdef _MSC_VER
#pragma inline_depth()
#endif

	  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->state, (LONG)PTW32_ONCE_DONE);

	  /*
	   * we didn't create the event.
	   * it is only there if there is someone waiting.
	   * Avoid using the global event_lock but still prevent SetEvent
	   * from overwriting any 'lasterror' if the event is closed before we
	   * are done with it.
	   */
	  if (InterlockedExchangeAdd((LPLONG)&once_control->event, 0L)) /* MBR fence */
	    {
	      if (!SetEvent(once_control->event))
		{
		  restoreLastError = PTW32_TRUE;
		}
	    }
	}
      else
	{
	  HANDLE tmpEvent;

	  if (cancelled)
	    {
	      /*
	       * Restore priority. We catch any changes to this thread's priority
	       * only if they were done through the POSIX API (i.e. pthread_setschedparam.
	       */
	      SetThreadPriority(w32Thread, ((ptw32_thread_t *)self.p)->sched_priority);
	      pthread_mutex_unlock(&((ptw32_thread_t *)self.p)->threadLock);
	    }

	  /*
	   * wait for init.
	   * while waiting, create an event to wait on
	   */

	  if (1 == InterlockedIncrement((LPLONG)&once_control->eventUsers))
	    {
	      /*
	       * RE CANCELLATION:
	       * If we are the first thread after the initter thread, and the init_routine is cancelled
	       * while we're suspended at this point in the code:-
	       * - state will not get set to PTW32_ONCE_DONE;
	       * - cleanup will not see an event and cannot set it;
	       * - therefore, we will eventually resume, create an event and wait on it;
	       * cleanup will set state == CANCELLED before checking for an event, so that
	       * we will see it and avoid waiting (as for state == DONE). We will go around again and
	       * we may then become the initter.
	       * If we are still the only other thread when we get to the end of this block, we will
	       * have closed the event (good). If another thread beats us to be initter, then we will
	       * re-enter here (good). In case the old event is reused, the event is always reset by
	       * the new initter before clearing the CANCELLED state, causing any threads that are
	       * cycling around the loop to wait again.
	       * The initter thread is guaranteed to be at equal or higher priority than any waiters
	       * so no waiters will starve the initter, which might otherwise cause us to loop
	       * forever.
	       */
	      tmpEvent = CreateEvent(NULL, PTW32_TRUE, PTW32_FALSE, NULL);
	      if (PTW32_INTERLOCKED_COMPARE_EXCHANGE((PTW32_INTERLOCKED_LPLONG)&once_control->event,
						     (PTW32_INTERLOCKED_LONG)tmpEvent,
						     (PTW32_INTERLOCKED_LONG)0))
		{
		  CloseHandle(tmpEvent);
		}
	    }

	  /*
	   * Check 'state' again in case the initting thread has finished or cancelled
	   * and left before seeing that there was an event to trigger.
	   */

	  switch (InterlockedExchangeAdd((LPLONG)&once_control->state, 0L))
	    {
	    case PTW32_ONCE_CLEAR:
	      {
		/* Neither DONE nor CANCELLED */
		if (WAIT_FAILED == WaitForSingleObject(once_control->event, INFINITE))
		  {
		    restoreLastError = PTW32_TRUE;
		    /*
		     * If the wait failed it's probably because the event is invalid.
		     * That's possible after a cancellation (but rare) if we got through the
		     * event create block above while a woken thread was suspended between
		     * the decrement and exchange below and then resumed before we could wait.
		     * So we'll yield.
		     */
		    Sleep(0);
		  }
		break;
	      }
	    case PTW32_ONCE_CANCELLED:
	      {
		if (once_control->started)
		  {
		    /* The new initter hasn't cleared the cancellation yet, so give the
		     * processor to a more productive thread. */
		    Sleep(0);
		  }
		break;
	      }
	    }

	  /* last one out shut off the lights */
	  if (0 == InterlockedDecrement((LPLONG)&once_control->eventUsers))
	    {
	      /* we were last */
	      if ((tmpEvent = (HANDLE)
		   PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->event,
					      (LONG)0)))
		{
		  CloseHandle(tmpEvent);
		}
	    }
	}
    }

  if (restoreLastError)
    {
      SetLastError (lasterror);
      WSASetLastError (lastWSAerror);
    }

  /*
   * ------------
   * Failure Code
   * ------------
   */
FAIL0:
  return (result);

}				/* pthread_once */
