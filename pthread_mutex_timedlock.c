/*
 * pthread_mutex_timedlock.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2003 Pthreads-win32 contributors
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

#ifndef _UWIN
//#   include <process.h>
#endif
#ifndef NEED_FTIME
#include <sys/timeb.h>
#endif
#include "pthread.h"
#include "implement.h"


static INLINE int
ptw32_timed_semwait (sem_t * sem, const struct timespec *abstime)
     /*
      * ------------------------------------------------------
      * DESCRIPTION
      *      This function waits on a POSIX semaphore. If the
      *      semaphore value is greater than zero, it decreases
      *      its value by one. If the semaphore value is zero, then
      *      the calling thread (or process) is blocked until it can
      *      successfully decrease the value or until abstime.
      *      If abstime has passed when this routine is called then
      *      it returns a result to indicate this.
      *
      *      If 'abstime' is a NULL pointer then this function will
      *      block until it can successfully decrease the value or
      *      until interrupted by a signal.
      *
      *      Unlike sem_timedwait(), this routine is not a cancelation point.
      *
      *      Unlike sem_timedwait(), this routine is non-cancelable.
      *
      * RESULTS
      *              2               abstime has passed already
      *              1               abstime timed out while waiting
      *              0               successfully decreased semaphore,
      *              -1              failed, error in errno.
      * ERRNO
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSYS          semaphores are not supported,
      *              EINTR           the function was interrupted by a signal,
      *              EDEADLK         a deadlock condition was detected.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

#ifdef NEED_FTIME

  struct timespec currSysTime;

#else /* NEED_FTIME */

  struct _timeb currSysTime;

#endif /* NEED_FTIME */

  const DWORD NANOSEC_PER_MILLISEC = 1000000;
  const DWORD MILLISEC_PER_SEC = 1000;
  DWORD milliseconds;
  DWORD status;

  if (sem == NULL)
    {
      result = EINVAL;
    }
  else
    {
      if (abstime == NULL)
	{
	  milliseconds = INFINITE;
	}
      else
	{
	  /* 
	   * Calculate timeout as milliseconds from current system time. 
	   */

	  /* get current system time */

#ifdef NEED_FTIME

	  {
	    FILETIME ft;
	    SYSTEMTIME st;

	    GetSystemTime (&st);
	    SystemTimeToFileTime (&st, &ft);
	    /*
	     * GetSystemTimeAsFileTime(&ft); would be faster,
	     * but it does not exist on WinCE
	     */

	    ptw32_filetime_to_timespec (&ft, &currSysTime);
	  }

	  /*
	   * subtract current system time from abstime
	   */
	  milliseconds =
	    (abstime->tv_sec - currSysTime.tv_sec) * MILLISEC_PER_SEC;
	  milliseconds +=
	    ((abstime->tv_nsec - currSysTime.tv_nsec) +
	     (NANOSEC_PER_MILLISEC / 2)) / NANOSEC_PER_MILLISEC;

#else /* NEED_FTIME */
	  _ftime (&currSysTime);

	  /*
	   * subtract current system time from abstime
	   */
	  milliseconds =
	    (abstime->tv_sec - currSysTime.time) * MILLISEC_PER_SEC;
	  milliseconds +=
	    ((abstime->tv_nsec +
	      (NANOSEC_PER_MILLISEC / 2)) / NANOSEC_PER_MILLISEC) -
	    currSysTime.millitm;

#endif /* NEED_FTIME */

	  if (((int) milliseconds) < 0)
	    {
	      return 2;
	    }
	}

#ifdef NEED_SEM

      status = WaitForSingleObject ((*sem)->event, milliseconds);

#else /* NEED_SEM */

      status = WaitForSingleObject ((*sem)->sem, milliseconds);

#endif

      if (status == WAIT_OBJECT_0)
	{

#ifdef NEED_SEM

	  ptw32_decrease_semaphore (sem);

#endif /* NEED_SEM */

	  return 0;
	}
      else if (status == WAIT_TIMEOUT)
	{
	  return 1;
	}
      else
	{
	  result = EINVAL;
	}
    }

  if (result != 0)
    {
      errno = result;
      return -1;
    }

  return 0;

}				/* ptw32_timed_semwait */


int
pthread_mutex_timedlock (pthread_mutex_t * mutex,
			 const struct timespec *abstime)
{
  LONG c;
  pthread_mutex_t mx;

#ifdef NEED_SEM
  errno = ENOTSUP;
  return -1;
#endif

  /*
   * Let the system deal with invalid pointers.
   */

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static mutex. We check
   * again inside the guarded section of ptw32_mutex_check_need_init()
   * to avoid race conditions.
   */
  if (*mutex >= PTHREAD_ERRORCHECK_MUTEX_INITIALIZER)
    {
      int result;

      if ((result = ptw32_mutex_check_need_init (mutex)) != 0)
	{
	  return (result);
	}
    }

  mx = *mutex;

  if (mx->kind == PTHREAD_MUTEX_NORMAL)
    {
      if ((c = (LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE(
		        (PTW32_INTERLOCKED_LPLONG) &mx->lock_idx,
		        (PTW32_INTERLOCKED_LONG) 0,
		        (PTW32_INTERLOCKED_LONG) -1)) != -1)
	{
          do
            {
              if (c == 1 ||
                  (LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE(
                           (PTW32_INTERLOCKED_LPLONG) &mx->lock_idx,
                           (PTW32_INTERLOCKED_LONG) 1,
                           (PTW32_INTERLOCKED_LONG) 0) != -1)
                {
		  switch (ptw32_timed_semwait (&mx->wait_sema, abstime))
		    {
		    case 0:	/* We got woken up so try get the lock again. */
		      {
		        break;
		      }
		    case 1:	/* Timed out. */
		    case 2:	/* abstime passed before we started to wait. */
		      {
		        return ETIMEDOUT;
		      }
		    default:
		      {
		        return errno;
		      }
		  }
		}
            }
          while ((c = (LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE(
                               (PTW32_INTERLOCKED_LPLONG) &mx->lock_idx,
                               (PTW32_INTERLOCKED_LONG) 1,
                               (PTW32_INTERLOCKED_LONG) -1)) != -1);
	}
    }
  else
    {
      pthread_t self = pthread_self();

      if ((c = (LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE(
                        (PTW32_INTERLOCKED_LPLONG) &mx->lock_idx,
		        (PTW32_INTERLOCKED_LONG) 0,
		        (PTW32_INTERLOCKED_LONG) -1)) == -1)
	{
	  mx->recursive_count = 1;
	  mx->ownerThread = self;
	}
      else
	{
	  if (pthread_equal (mx->ownerThread, self))
	    {
	      if (mx->kind == PTHREAD_MUTEX_RECURSIVE)
		{
		  mx->recursive_count++;
		}
	      else
		{
		  return EDEADLK;
		}
	    }
	  else
	    {
              do
                {
                  if (c == 1 ||
                      (LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE(
                               (PTW32_INTERLOCKED_LPLONG) &mx->lock_idx,
                               (PTW32_INTERLOCKED_LONG) 1,
                               (PTW32_INTERLOCKED_LONG) 0) != -1)
                    {
		      switch (ptw32_timed_semwait (&mx->wait_sema, abstime))
		        {
		        case 0:	/* We got woken up so try get the lock again. */
		          {
		            break;
		          }
		        case 1:	/* Timed out. */
		        case 2:	/* abstime passed before we started to wait. */
		          {
		            return ETIMEDOUT;
		          }
		        default:
		          {
		            return errno;
		          }
		      }
		    }
                }
              while ((c = (LONG) PTW32_INTERLOCKED_COMPARE_EXCHANGE(
                                   (PTW32_INTERLOCKED_LPLONG) &mx->lock_idx,
                                   (PTW32_INTERLOCKED_LONG) 1,
                                   (PTW32_INTERLOCKED_LONG) -1)) != -1);

	      mx->recursive_count = 1;
	      mx->ownerThread = self;
	    }
	}
    }

  return 0;
}
