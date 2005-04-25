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

  (void) pthread_mutex_lock(&ptw32_once_control.mtx);
  once_control->done = PTW32_ONCE_CANCELLED;
  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->started, -1L);
  /*
   * Wake everyone up.
   *
   * Holding the mutex during the broadcast prevents threads being left
   * behind waiting.
   */
  (void) pthread_cond_broadcast(&ptw32_once_control.cond);
  (void) pthread_mutex_unlock(&ptw32_once_control.mtx);
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
   * Use a single global cond+mutex to manage access to all once_control objects.
   * Unlike a global mutex on it's own, the global cond+mutex allows faster
   * once_controls to overtake slower ones. Spurious wakeups may occur, but
   * can be tolerated.
   *
   * Since this is being introduced as a bug fix, the global cond+mtx also avoids
   * a change in the ABI, maintaining backwards compatibility.
   *
   * To maintain a separate mutex for each once_control object requires either
   * cleaning up the mutex (difficult to synchronise reliably), or leaving it
   * around forever. Since we can't make assumptions about how an application might
   * employ pthread_once objects, the later is considered to be unacceptable.
   *
   * once_control->done is now a multipurpose flag. It indicates either that
   * the init_routine has been completed, or the thread running it has been cancelled.
   *
   * Priority boosting is used to ensure that the init_routine thread is not
   * starved, by higher priority threads inside the while loop, before it can
   * clear the cancelled flag. The init_routine will be run at the thread's
   * normal base priority. Note that priority boosting is momentary, independent
   * for each once_control, and occurs only AFTER an init_routine cancellation.
   */

  while (!((state = InterlockedExchangeAdd((LPLONG)&once_control->done, 0L)) /* Full mem barrier read */
	   & PTW32_ONCE_DONE))
    {
      /*
       * Keep a per thread record of the cancelled state for speed. If the
       * once_control state changes before we've finished with our local copy
       * then no harm is done - in fact, we need it to complete the full priority
       * boost transaction.
       */
      LONG cancelled = (state & PTW32_ONCE_CANCELLED);

      if (cancelled)
	{
	  /* Boost priority momentarily */
	  if (!w32Thread)
	    {
	      self = pthread_self();
	      w32Thread = ((ptw32_thread_t *)self.p)->threadH;
	    }
	  /* Prevent pthread_setschedparam() changing our priority while we're boosted. */
	  (void) pthread_mutex_lock(&((ptw32_thread_t *)self.p)->threadLock);
	  SetThreadPriority(w32Thread, THREAD_PRIORITY_HIGHEST);
	}

      if (PTW32_INTERLOCKED_EXCHANGE((LPLONG) &once_control->started, 0L) == -1)
	{
	  if (cancelled)
	    {
	      /* Reset cancelled state */
	      (void) pthread_mutex_lock(&ptw32_once_control.mtx);
	      once_control->done = PTW32_ONCE_CLEAR;
	      (void) pthread_mutex_unlock(&ptw32_once_control.mtx);

	      /*
	       * Restore priority - any priority changes since the thread was created
	       * will be applied only if they were made via POSIX (i.e. pthread_setschedparam).
	       */
	      SetThreadPriority(w32Thread, ((ptw32_thread_t *)self.p)->sched_priority);
	      (void) pthread_mutex_unlock(&((ptw32_thread_t *)self.p)->threadLock);
	    }

#ifdef _MSC_VER
#pragma inline_depth(0)
#endif

	  pthread_cleanup_push(ptw32_once_init_routine_cleanup, (void*) once_control);
	  (*init_routine) ();
	  pthread_cleanup_pop(0);

#ifdef _MSC_VER
#pragma inline_depth()
#endif

	  /*
	   * Holding the mutex during the broadcast prevents threads being left
	   * behind waiting.
	   */
	  (void) pthread_mutex_lock(&ptw32_once_control.mtx);
	  once_control->done = PTW32_ONCE_DONE;
	  (void) pthread_cond_broadcast(&ptw32_once_control.cond);
	  (void) pthread_mutex_unlock(&ptw32_once_control.mtx);
	}
      else
	{
	  int oldCancelState;

	  if (cancelled)
	    {
	      /*
	       * Restore priority - any priority changes since the thread was created
	       * will be applied only if they were made via POSIX (i.e. pthread_setschedparam).
	       */
	      SetThreadPriority(w32Thread, ((ptw32_thread_t *)self.p)->sched_priority);
	      (void) pthread_mutex_unlock(&((ptw32_thread_t *)self.p)->threadLock);
	    }

	  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
	  (void) pthread_mutex_lock(&ptw32_once_control.mtx);
	  while (!once_control->done /* Neither DONE nor CANCELLED */
		 || (!(once_control->done & PTW32_ONCE_DONE)
		     && cancelled) /* Stop after one init_routine re-contest */)
	    {
	      cancelled = 0;
	      (void) pthread_cond_wait(&ptw32_once_control.cond, &ptw32_once_control.mtx);
	    }
	  (void) pthread_mutex_unlock(&ptw32_once_control.mtx);
	  pthread_setcancelstate(oldCancelState, NULL);
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
