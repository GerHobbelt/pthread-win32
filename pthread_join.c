/*
 * pthread_join.c
 *
 * Description:
 * This translation unit implements functions related to thread
 * synchronisation.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2004 Pthreads-win32 contributors
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

/*
 * Not needed yet, but defining it should indicate clashes with build target
 * environment that should be fixed.
 */
#include <signal.h>


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
  int result;
  pthread_t self;
  ptw32_thread_t * tp = (ptw32_thread_t *) thread.p;

  /*
   * Possibilities for the target thread on entry to pthread_join():
   *
   * 1) the target thread is detached, in which case it could be destroyed and
   * it's thread id (struct) reused before we get the reuse lock;
   * 2) the target thread is joinable, in which case it will not be destroyed
   * until it has been joined, and it's thread id cannot be reused before we
   * get access to it.
   *
   * Only (1) is a potential race condition.
   * While (1) is possibly an application programming error, pthread_join is
   * required to handle it with an error.
   * If an application attempts to join a detached thread that exits before we
   * get the reuse lock, it's thread struct could be reused for a new joinable
   * thread before we get the reuse_lock and we will then be joining the wrong
   * thread.
   *
   * To fix this properly would require a reuse count as part of the thread id
   * so that we could confirm that we are working with the same thread. This
   * option will require a major change to the API, forcing recompilation of
   * applications that use the library.
   *
   * Cheaper alternatives that do not change the API are:
   * - separate reuse stacks could be used for detached and joinable
   * threads. Threads which detach themselves will not present a race condition
   * because they will be placed on a different stack to that which provides
   * reusable structs to new joinable threads.
   * The problem with this solution is that, an application that creates
   * joinable threads which then detach themselves can cause the detach reuse
   * stack to grow indefinitely.
   *
   * - maintain a sufficiently large reuse pool and manage it as a FIFO
   * queue. This would prevent the immediate reuse of thread structs, but this
   * only provides a statistical safeguard, not a deterministic one.
   *
   * - include an entry point array at the start of each thread struct so that
   * each reuse (modulo N) returns a different thread id (pointer to the thread
   * struct). Offset 0 from this pointer will contain a validity field and an
   * offset value field (subtracted from the pointer to obtain the address of
   * thread struct). This option is similar to including a reuse counter with
   * the thread id but maintains the thread id as a simple pointer.
   *
   * As at 27/08/2004, none of the above have been implemented.
   */

  EnterCriticalSection (&ptw32_thread_reuse_lock);
  /*
   * The first test is the same as pthread_kill(thread, 0), duplicated here
   * so that we can use the reuse_lock to ensure the thread isn't destroyed
   * and reused before we've finished with the POSIX thread struct.
   */
  if (tp == NULL
      || NULL == tp->threadH
      || THREAD_PRIORITY_ERROR_RETURN == GetThreadPriority (tp->threadH))
    {
      result = ESRCH;
    }
  else if (PTHREAD_CREATE_DETACHED == tp->detachState)
    {
      result = EINVAL;
    }
  else
    {
      /* 
       * The target thread is joinable and can't be reused before we join it.
       */

      LeaveCriticalSection (&ptw32_thread_reuse_lock);

      self = pthread_self();

      if (NULL == self.p)
	{
	  result = ENOENT;
	}
      else if (0 != pthread_equal (self, thread))
	{
	  result = EDEADLK;
	}
      else
	{
	  /*
	   * Pthread_join is a cancelation point.
	   * If we are canceled then our target thread must not be
	   * detached (destroyed). This is guarranteed because
	   * pthreadCancelableWait will not return if we
	   * are canceled.
	   */
	  result = pthreadCancelableWait (tp->threadH);

	  if (0 == result)
	    {

#if ! defined (__MINGW32__) || defined (__MSVCRT__) || defined (__DMC__)

	      if (value_ptr != NULL
		  && !GetExitCodeThread (tp->threadH, (LPDWORD) value_ptr))
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
		  *value_ptr = tp->exitStatus;
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

      goto FAIL0;
    }

  LeaveCriticalSection (&ptw32_thread_reuse_lock);
FAIL0:
  return (result);

}				/* pthread_join */
