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
 * Version E
 * ---------
 * Substituting a semaphore in place of the event achieves the same effect as an
 * auto-reset event in the post cancellation phase, and a manual-reset event in the
 * normal exit phase. The new initter thread does not need to do any post-cancellation
 * operations, and waiters only need to check that there is a new initter running
 * before starting to wait. All priority issues and adjustments disappear.
 */

#include "pthread.h"
#include "implement.h"


static void PTW32_CDECL
ptw32_once_init_routine_cleanup(void * arg)
{
  pthread_once_t * once_control = (pthread_once_t *) arg;

  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->started, (LONG)PTW32_FALSE);

  if (InterlockedExchangeAdd((LPLONG)&once_control->semaphore, 0L)) /* MBR fence */
    {
      ReleaseSemaphore(once_control->semaphore, 1, NULL);
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
  HANDLE sema;

  if (once_control == NULL || init_routine == NULL)
    {
      result = EINVAL;
      goto FAIL0;
    }
  else
    {
      result = 0;
    }

  while (!InterlockedExchangeAdd((LPLONG)&once_control->done, 0L)) /* Atomic Read */
    {
      if (!PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->started, (LONG)PTW32_TRUE))
	{

#ifdef _MSC_VER
#pragma inline_depth(0)
#endif

	  pthread_cleanup_push(ptw32_once_init_routine_cleanup, (void *) once_control);
	  (*init_routine)();
	  pthread_cleanup_pop(0);

#ifdef _MSC_VER
#pragma inline_depth()
#endif

	  (void) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->done, (LONG)PTW32_TRUE);

	  /*
	   * we didn't create the semaphore.
	   * it is only there if there is someone waiting.
	   */
	  if (InterlockedExchangeAdd((LPLONG)&once_control->semaphore, 0L)) /* MBR fence */
	    {
	      ReleaseSemaphore(once_control->semaphore, once_control->numSemaphoreUsers, NULL);
	    }
	}
      else
	{
	  InterlockedIncrement((LPLONG)&once_control->numSemaphoreUsers);

	  if (!InterlockedExchangeAdd((LPLONG)&once_control->semaphore, 0L)) /* MBR fence */
	    {
	      sema = CreateSemaphore(NULL, 0, INT_MAX, NULL);

	      if (PTW32_INTERLOCKED_COMPARE_EXCHANGE((PTW32_INTERLOCKED_LPLONG)&once_control->semaphore,
						     (PTW32_INTERLOCKED_LONG)sema,
						     (PTW32_INTERLOCKED_LONG)0))
		{
		  CloseHandle(sema);
		}
	    }

	  /*
	   * Check 'done' and 'started' again in case the initting thread has finished or cancelled
	   * and left before seeing that there was a semaphore to release.
	   */
	  if (InterlockedExchangeAdd((LPLONG)&once_control->done, 0L) /* Done immediately, or */
	      || !InterlockedExchangeAdd((LPLONG)&once_control->started, 0L) /* No initter yet, or */
	      || WaitForSingleObject(once_control->semaphore, INFINITE)) /* Done or Cancelled */
	    {
	      if (0 == InterlockedDecrement((LPLONG)&once_control->numSemaphoreUsers))
		{
		  /* we were last */
		  if ((sema = (HANDLE) PTW32_INTERLOCKED_EXCHANGE((LPLONG)&once_control->semaphore,
								  (LONG)0)))
		    {
		      CloseHandle(sema);
		    }
		}
	    }
	}
    }

  /*
   * ------------
   * Failure Code
   * ------------
   */
FAIL0:
  return (result);
}                                               /* pthread_once */ 
