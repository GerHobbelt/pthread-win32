/*
 * mutex.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998 Ben Elliston and Ross Johnson
 * Copyright (C) 1999,2000,2001 Ross Johnson
 *
 * Contact Email: rpj@ise.canberra.edu.au
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

#ifndef _UWIN
#   include <process.h>
#endif
#ifndef NEED_FTIME
#include <sys/timeb.h>
#endif
#include "pthread.h"
#include "implement.h"


static INLINE int
ptw32_mutex_check_need_init(pthread_mutex_t *mutex)
{
  int result = 0;

  /*
   * The following guarded test is specifically for statically
   * initialised mutexes (via PTHREAD_MUTEX_INITIALIZER).
   *
   * Note that by not providing this synchronisation we risk
   * introducing race conditions into applications which are
   * correctly written.
   *
   * Approach
   * --------
   * We know that static mutexes will not be PROCESS_SHARED
   * so we can serialise access to internal state using
   * Win32 Critical Sections rather than Win32 Mutexes.
   *
   * If using a single global lock slows applications down too much,
   * multiple global locks could be created and hashed on some random
   * value associated with each mutex, the pointer perhaps. At a guess,
   * a good value for the optimal number of global locks might be
   * the number of processors + 1.
   *
   */
  EnterCriticalSection(&ptw32_mutex_test_init_lock);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section
   * and only initialise if the mutex is valid (not been destroyed).
   * If a static mutex has been destroyed, the application can
   * re-initialise it only by calling pthread_mutex_init()
   * explicitly.
   */
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    {
      result = pthread_mutex_init(mutex, NULL);
    }
  else if (*mutex == NULL)
    {
      /*
       * The mutex has been destroyed while we were waiting to
       * initialise it, so the operation that caused the
       * auto-initialisation should fail.
       */
      result = EINVAL;
    }

  LeaveCriticalSection(&ptw32_mutex_test_init_lock);

  return(result);
}

int
pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL)
    {
      return EINVAL;
    }

  if (attr != NULL
      && *attr != NULL
      && (*attr)->pshared == PTHREAD_PROCESS_SHARED
      )
    {
      /*
       * Creating mutex that can be shared between
       * processes.
       */
#if _POSIX_THREAD_PROCESS_SHARED

      /*
       * Not implemented yet.
       */

#error ERROR [__FILE__, line __LINE__]: Process shared mutexes are not supported yet.

#else

      return ENOSYS;

#endif /* _POSIX_THREAD_PROCESS_SHARED */

    }

  mx = (pthread_mutex_t) calloc(1, sizeof(*mx));

  if (mx == NULL)
    {
      result = ENOMEM;
      goto FAIL0;
    }

  mx->lock_idx = PTW32_MUTEX_LOCK_IDX_INIT;
  mx->recursive_count = 0;
  mx->kind = (attr == NULL || *attr == NULL
	      ? PTHREAD_MUTEX_DEFAULT
	      : (*attr)->kind);
  mx->ownerThread = NULL;

  if( 0 != sem_init( &mx->wait_sema, 0, 0 ))
    {
      result = EAGAIN;
    }

  if (result != 0 && mx != NULL)
    {
      free(mx);
      mx = NULL;
    }

FAIL0:
  InitializeCriticalSection( &mx->wait_cs );
  *mutex = mx;

  return(result);
}

int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL
      || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * Check to see if we have something to delete.
   */
  if (*mutex != PTHREAD_MUTEX_INITIALIZER)
    {
      mx = *mutex;

      result = pthread_mutex_trylock(&mx);

      /*
       * The mutex type may not be RECURSIVE therefore trylock may return EBUSY if
       * we already own the mutex. Here we are assuming that it's OK to destroy
       * a mutex that we own and have locked recursively. Is this correct?
       *
       * For FAST mutexes we record the owner as ANONYMOUS for speed. In this
       * case we assume that the thread calling pthread_mutex_destroy() is the
       * owner, if the mutex is owned at all.
       */
      if (result == 0
	  || mx->ownerThread == (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS
	  || pthread_equal( mx->ownerThread, pthread_self() ) )
	{
	  /*
	   * FIXME!!!
	   * The mutex isn't held by another thread but we could still
	   * be too late invalidating the mutex below since another thread
	   * may already have entered mutex_lock and the check for a valid
	   * *mutex != NULL.
	   */
	  *mutex = NULL;

	  result = pthread_mutex_unlock(&mx);

	  if (result == 0)
	    {
	      (void) sem_destroy( &mx->wait_sema );
	      DeleteCriticalSection( &mx->wait_cs );
	      free(mx);
	    }
	  else
	    {
	      /*
	       * Restore the mutex before we return the error.
	       */
	      *mutex = mx;
	    }
	}
    }
  else
    {
      /*
       * See notes in ptw32_mutex_check_need_init() above also.
       */
      EnterCriticalSection(&ptw32_mutex_test_init_lock);

      /*
       * Check again.
       */
      if (*mutex == PTHREAD_MUTEX_INITIALIZER)
	{
	  /*
	   * This is all we need to do to destroy a statically
	   * initialised mutex that has not yet been used (initialised).
	   * If we get to here, another thread
	   * waiting to initialise this mutex will get an EINVAL.
	   */
	  *mutex = NULL;
	}
      else
	{
	  /*
	   * The mutex has been initialised while we were waiting
	   * so assume it's in use.
	   */
	  result = EBUSY;
	}

      LeaveCriticalSection(&ptw32_mutex_test_init_lock);
    }

  return(result);
}

int
pthread_mutexattr_init (pthread_mutexattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Initializes a mutex attributes object with default
      *      attributes.
      *
      * PARAMETERS
      *      attr
      * 	     pointer to an instance of pthread_mutexattr_t
      *
      *
      * DESCRIPTION
      *      Initializes a mutex attributes object with default
      *      attributes.
      *
      *      NOTES:
      * 	     1)      Used to define mutex types
      *
      * RESULTS
      * 	     0		     successfully initialized attr,
      * 	     ENOMEM	     insufficient memory for attr.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_mutexattr_t ma;

  ma = (pthread_mutexattr_t) calloc (1, sizeof (*ma));

  if (ma == NULL)
    {
      result = ENOMEM;
    }
  else
    {
      ma->pshared = PTHREAD_PROCESS_PRIVATE;
      ma->kind = PTHREAD_MUTEX_DEFAULT;
    }

  *attr = ma;

  return(result);
}				/* pthread_mutexattr_init */


int
pthread_mutexattr_destroy (pthread_mutexattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Destroys a mutex attributes object. The object can
      *      no longer be used.
      *
      * PARAMETERS
      *      attr
      * 	     pointer to an instance of pthread_mutexattr_t
      *
      *
      * DESCRIPTION
      *      Destroys a mutex attributes object. The object can
      *      no longer be used.
      *
      *      NOTES:
      * 	     1)      Does not affect mutexes created using 'attr'
      *
      * RESULTS
      * 	     0		     successfully released attr,
      * 	     EINVAL	     'attr' is invalid.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if (attr == NULL || *attr == NULL)
    {
      result = EINVAL;
    }
  else
    {
      pthread_mutexattr_t ma = *attr;

      *attr = NULL;
      free (ma);
    }

  return(result);
}				/* pthread_mutexattr_destroy */


int
pthread_mutexattr_getpshared (const pthread_mutexattr_t * attr,
			      int *pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Determine whether mutexes created with 'attr' can be
      *      shared between processes.
      *
      * PARAMETERS
      *      attr
      * 	     pointer to an instance of pthread_mutexattr_t
      *
      *      pshared
      * 	     will be set to one of:
      *
      * 		     PTHREAD_PROCESS_SHARED
      * 			     May be shared if in shared memory
      *
      * 		     PTHREAD_PROCESS_PRIVATE
      * 			     Cannot be shared.
      *
      *
      * DESCRIPTION
      *      Mutexes creatd with 'attr' can be shared between
      *      processes if pthread_mutex_t variable is allocated
      *      in memory shared by these processes.
      *      NOTES:
      * 	     1)      pshared mutexes MUST be allocated in shared
      * 		     memory.
      * 	     2)      The following macro is defined if shared mutexes
      * 		     are supported:
      * 			     _POSIX_THREAD_PROCESS_SHARED
      *
      * RESULTS
      * 	     0		     successfully retrieved attribute,
      * 	     EINVAL	     'attr' is invalid,
      *
      * ------------------------------------------------------
      */
{
  int result;

  if ((attr != NULL && *attr != NULL) &&
      (pshared != NULL))
    {
      *pshared = (*attr)->pshared;
      result = 0;
    }
  else
    {
      result = EINVAL;
    }

  return (result);

}				/* pthread_mutexattr_getpshared */


int
pthread_mutexattr_setpshared (pthread_mutexattr_t * attr,
			      int pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Mutexes created with 'attr' can be shared between
      *      processes if pthread_mutex_t variable is allocated
      *      in memory shared by these processes.
      *
      * PARAMETERS
      *      attr
      * 	     pointer to an instance of pthread_mutexattr_t
      *
      *      pshared
      * 	     must be one of:
      *
      * 		     PTHREAD_PROCESS_SHARED
      * 			     May be shared if in shared memory
      *
      * 		     PTHREAD_PROCESS_PRIVATE
      * 			     Cannot be shared.
      *
      * DESCRIPTION
      *      Mutexes creatd with 'attr' can be shared between
      *      processes if pthread_mutex_t variable is allocated
      *      in memory shared by these processes.
      *
      *      NOTES:
      * 	     1)      pshared mutexes MUST be allocated in shared
      * 		     memory.
      *
      * 	     2)      The following macro is defined if shared mutexes
      * 		     are supported:
      * 			     _POSIX_THREAD_PROCESS_SHARED
      *
      * RESULTS
      * 	     0		     successfully set attribute,
      * 	     EINVAL	     'attr' or pshared is invalid,
      * 	     ENOSYS	     PTHREAD_PROCESS_SHARED not supported,
      *
      * ------------------------------------------------------
      */
{
  int result;

  if ((attr != NULL && *attr != NULL) &&
      ((pshared == PTHREAD_PROCESS_SHARED) ||
       (pshared == PTHREAD_PROCESS_PRIVATE)))
    {
      if (pshared == PTHREAD_PROCESS_SHARED)
	{

#if !defined( _POSIX_THREAD_PROCESS_SHARED )

	  result = ENOSYS;
	  pshared = PTHREAD_PROCESS_PRIVATE;

#else

	  result = 0;

#endif /* _POSIX_THREAD_PROCESS_SHARED */

	}
      else
	{
	  result = 0;
	}

      (*attr)->pshared = pshared;
    }
  else
    {
      result = EINVAL;
    }

  return (result);

}				/* pthread_mutexattr_setpshared */


int
pthread_mutexattr_settype (pthread_mutexattr_t * attr,
						   int kind)
     /*
      * ------------------------------------------------------
      *
      * DOCPUBLIC
      * The pthread_mutexattr_settype() and
      * pthread_mutexattr_gettype() functions  respectively set and
      * get the mutex type  attribute. This attribute is set in  the
      * type parameter to these functions.
      *
      * PARAMETERS
      *      attr
      * 	     pointer to an instance of pthread_mutexattr_t
      *
      *      type
      * 	     must be one of:
      *
      * 		     PTHREAD_MUTEX_DEFAULT
      *
      * 		     PTHREAD_MUTEX_NORMAL
      *
      * 		     PTHREAD_MUTEX_ERRORCHECK
      *
      * 		     PTHREAD_MUTEX_RECURSIVE
      *
      * DESCRIPTION
      * The pthread_mutexattr_settype() and
      * pthread_mutexattr_gettype() functions  respectively set and
      * get the mutex type  attribute. This attribute is set in  the
      * type  parameter to these functions. The default value of the
      * type  attribute is  PTHREAD_MUTEX_DEFAULT.
      * 
      * The type of mutex is contained in the type  attribute of the
      * mutex attributes. Valid mutex types include:
      *
      * PTHREAD_MUTEX_NORMAL
      * 	 This type of mutex does  not  detect  deadlock.  A
      * 	 thread  attempting  to  relock  this mutex without
      * 	 first unlocking it will  deadlock.  Attempting  to
      * 	 unlock  a  mutex  locked  by  a  different  thread
      * 	 results  in  undefined  behavior.  Attempting	 to
      * 	 unlock  an  unlocked  mutex  results  in undefined
      * 	 behavior.
      * 
      * PTHREAD_MUTEX_ERRORCHECK
      * 	 This type of  mutex  provides	error  checking.  A
      * 	 thread  attempting  to  relock  this mutex without
      * 	 first unlocking it will return with  an  error.  A
      * 	 thread  attempting to unlock a mutex which another
      * 	 thread has locked will return	with  an  error.  A
      * 	 thread attempting to unlock an unlocked mutex will
      * 	 return with an error.
      *
      * PTHREAD_MUTEX_DEFAULT
      * 	 Same as PTHREAD_MUTEX_NORMAL.
      * 
      * PTHREAD_MUTEX_RECURSIVE
      * 	 A thread attempting to relock this  mutex  without
      * 	 first	unlocking  it  will  succeed in locking the
      * 	 mutex. The relocking deadlock which can occur with
      * 	 mutexes of type  PTHREAD_MUTEX_NORMAL cannot occur
      * 	 with this type of mutex. Multiple  locks  of  this
      * 	 mutex	require  the  same  number  of	unlocks  to
      * 	 release  the  mutex  before  another  thread	can
      * 	 acquire the mutex. A thread attempting to unlock a
      * 	 mutex which another thread has locked will  return
      * 	 with  an  error. A thread attempting to  unlock an
      * 	 unlocked mutex will return  with  an  error.  This
      * 	 type  of mutex is only supported for mutexes whose
      * 	 process	shared	       attribute	 is
      * 	 PTHREAD_PROCESS_PRIVATE.
      *
      * RESULTS
      * 	     0		     successfully set attribute,
      * 	     EINVAL	     'attr' or 'type' is invalid,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if ((attr != NULL && *attr != NULL))
    {
      switch (kind)
	{
	case PTHREAD_MUTEX_FAST_NP:
	case PTHREAD_MUTEX_RECURSIVE_NP:
	case PTHREAD_MUTEX_ERRORCHECK_NP:
	  (*attr)->kind = kind;
	  break;
	default:
	  result = EINVAL;
	  break;
	}
    }
  else
    {
      result = EINVAL;
    }
  
  return (result);
}				/* pthread_mutexattr_settype */


int
pthread_mutexattr_gettype (pthread_mutexattr_t * attr,
			   int *kind)
{
  int result = 0;

  if (attr != NULL && *attr != NULL && kind != NULL)
    {
      *kind = (*attr)->kind;
    }
  else
    {
      result = EINVAL;
    }

  return (result);
}


static INLINE int
ptw32_timed_semwait (sem_t * sem, const struct timespec * abstime)
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
      * RESULTS
      * 	     2		     abstime has passed already
      * 	     1		     abstime timed out while waiting
      * 	     0		     successfully decreased semaphore,
      * 	     -1 	     failed, error in errno.
      * ERRNO
      * 	     EINVAL	     'sem' is not a valid semaphore,
      * 	     ENOSYS	     semaphores are not supported,
      * 	     EINTR	     the function was interrupted by a signal,
      * 	     EDEADLK	     a deadlock condition was detected.
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

	    GetSystemTime(&st);
	    SystemTimeToFileTime(&st, &ft);
	    /*
	     * GetSystemTimeAsFileTime(&ft); would be faster,
	     * but it does not exist on WinCE
	     */

	    filetime_to_timespec(&ft, &currSysTime);
	  }

	  /*
	   * subtract current system time from abstime
	   */
	  milliseconds = (abstime->tv_sec - currSysTime.tv_sec) * MILLISEC_PER_SEC;
	  milliseconds += ((abstime->tv_nsec - currSysTime.tv_nsec) + (NANOSEC_PER_MILLISEC/2)) / NANOSEC_PER_MILLISEC;

#else /* NEED_FTIME */
	  _ftime(&currSysTime);

	  /*
	   * subtract current system time from abstime
	   */
	  milliseconds = (abstime->tv_sec - currSysTime.time) * MILLISEC_PER_SEC;
	  milliseconds += ((abstime->tv_nsec + (NANOSEC_PER_MILLISEC/2)) / NANOSEC_PER_MILLISEC) -
	    currSysTime.millitm;

#endif /* NEED_FTIME */


	  if (((int) milliseconds) < 0)
	    {
	      return 2;
	    }
	}

#ifdef NEED_SEM

      status = WaitForSingleObject( (*sem)->event, milliseconds );

#else /* NEED_SEM */
	    
      status = WaitForSingleObject( (*sem)->sem, milliseconds );

#endif

      if (status == WAIT_OBJECT_0)
	{

#ifdef NEED_SEM

	  ptw32_decrease_semaphore(sem);

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
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;


  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static mutex. We check
   * again inside the guarded section of ptw32_mutex_check_need_init()
   * to avoid race conditions.
   */
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    {
      if ((result = ptw32_mutex_check_need_init(mutex)) != 0)
	{
	  return(result);
	}
    }

  mx = *mutex;

  if( 0 == InterlockedIncrement( &mx->lock_idx ) )
    {
      mx->recursive_count = 1;
      mx->ownerThread = (mx->kind != PTHREAD_MUTEX_FAST_NP
			 ? pthread_self()
			 : (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS);
    }
  else
    {
      if( mx->kind != PTHREAD_MUTEX_FAST_NP &&
	  pthread_equal( mx->ownerThread, pthread_self() ) )
	{
	  (void) InterlockedDecrement( &mx->lock_idx );

	  if( mx->kind == PTHREAD_MUTEX_RECURSIVE_NP )
	    {
	      mx->recursive_count++;
	    }
	  else
	    {
	      result = EDEADLK;
	    }
	}
      else
	{
	  if ((result = sem_wait( &mx->wait_sema )) == 0)
	    {
	      mx->recursive_count = 1;
	      mx->ownerThread = (mx->kind != PTHREAD_MUTEX_FAST_NP
				 ? pthread_self()
				 : (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS);
	    }
	}
    }

  return(result);
}


int
pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *abstime)
{
  int result = 0;
  pthread_mutex_t mx;


  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static mutex. We check
   * again inside the guarded section of ptw32_mutex_check_need_init()
   * to avoid race conditions.
   */
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    {
      if ((result = ptw32_mutex_check_need_init(mutex)) != 0)
	{
	  return(result);
	}
    }

  mx = *mutex;

  if( 0 == InterlockedIncrement( &mx->lock_idx ) )
    {
      mx->recursive_count = 1;
      mx->ownerThread = (mx->kind != PTHREAD_MUTEX_FAST_NP
			 ? pthread_self()
			 : (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS);
    }
  else
    {
      if( mx->kind != PTHREAD_MUTEX_FAST_NP &&
	  pthread_equal( mx->ownerThread, pthread_self() ) )
	{
	  (void) InterlockedDecrement( &mx->lock_idx );

	  if( mx->kind == PTHREAD_MUTEX_RECURSIVE_NP )
	    {
	      mx->recursive_count++;
	    }
	  else
	    {
	      result = EDEADLK;
	    }
	}
      else
	{
	  if (abstime == NULL)
	    {
	      result = EINVAL;
	    }
	  else
	    {
	      switch (ptw32_timed_semwait( &mx->wait_sema, abstime ))
		{
		  case 0: /* We got the mutex. */
		    {
		      mx->recursive_count = 1;
		      mx->ownerThread = (mx->kind != PTHREAD_MUTEX_FAST_NP
					 ? pthread_self()
					 : (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS);
		      break;
		    }
		  case 1: /* Timedout, try a second grab. */
		    {
		      EnterCriticalSection(&mx->wait_cs);

		      /*
                       * If we timeout, it is up to us to adjust lock_idx to say
                       * we're no longer waiting. If the mutex was also unlocked
                       * while we were timing out, and we simply return ETIMEDOUT,
                       * then wait_sema would be left in a state that is not consistent
                       * with the state of lock_idx.
                       *
		       * We must check to see if wait_sema has just been posted
                       * but we can't just call sem_getvalue - we must compete for
                       * the semaphore using sem_trywait(), otherwise we would need
                       * additional critical sections elsewhere, which would make the
                       * logic too inefficient.
                       *
                       * If sem_trywait returns EAGAIN then either wait_sema
                       * was given directly to another waiting thread or
                       * another thread has called sem_*wait() before us and
                       * taken the lock. Then we MUST decrement lock_idx and return
                       * ETIMEDOUT.
                       *
                       * Otherwise we MUST return success (because we have effectively
                       * acquired the lock that would have been ours had we not
                       * timed out), and NOT decrement lock_idx.
		       *
		       * We can almost guarrantee that EAGAIN is the only
		       * possible error, so no need to test errno.
		       */

		      if (-1 == sem_trywait( &mx->wait_sema ))
			{
			  (void) InterlockedDecrement( &mx->lock_idx );
			  result = ETIMEDOUT;
			}

		      LeaveCriticalSection(&mx->wait_cs);
		      break;
		    }
		  case 2: /* abstime had passed before we started to wait. */
		    {
		      result = ETIMEDOUT;
		      break;
		    }
		  default:
		    {
		      result = errno;
		      break;
		    }
		}
	    }
	}
    }

  return(result);
}


int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  mx = *mutex;

  /* 
   * If the thread calling us holds the mutex then there is no
   * race condition. If another thread holds the
   * lock then we shouldn't be in here.
   */
  if (mx != PTHREAD_MUTEX_INITIALIZER)
    {
      if (mx->ownerThread == (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS
	  || pthread_equal(mx->ownerThread, pthread_self()))
	{
	  if( mx->kind != PTHREAD_MUTEX_RECURSIVE_NP
	      || 0 == --mx->recursive_count )
	    {
	      mx->ownerThread = NULL;
	      EnterCriticalSection( &mx->wait_cs );

	      if( InterlockedDecrement( &mx->lock_idx ) >= 0 )
		{
		  /* Someone is waiting on that mutex */
		  if (sem_post( &mx->wait_sema ) != 0)
		    {
		      result = errno;
		    }
		}

	      LeaveCriticalSection( &mx->wait_cs );
	    }
	}
      else
	{
	  result = EPERM;
	}
    }
  else
    {
      result = EINVAL;
    }

  return(result);
}

int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static mutex. We check
   * again inside the guarded section of ptw32_mutex_check_need_init()
   * to avoid race conditions.
   */
  if (*mutex == PTHREAD_MUTEX_INITIALIZER)
    {
      result = ptw32_mutex_check_need_init(mutex);
    }

  mx = *mutex;

  if (result == 0)
    {
      if ( (PTW32_INTERLOCKED_LONG) PTW32_MUTEX_LOCK_IDX_INIT ==
	   ptw32_interlocked_compare_exchange((PTW32_INTERLOCKED_LPLONG) &mx->lock_idx,
					      (PTW32_INTERLOCKED_LONG) 0,
					      (PTW32_INTERLOCKED_LONG) PTW32_MUTEX_LOCK_IDX_INIT))
	{
	  mx->recursive_count = 1;
	  mx->ownerThread = (mx->kind != PTHREAD_MUTEX_FAST_NP
			     ? pthread_self()
			     : (pthread_t) PTW32_MUTEX_OWNER_ANONYMOUS);
	}
      else
	{
	  if( mx->kind != PTHREAD_MUTEX_FAST_NP &&
	      pthread_equal( mx->ownerThread, pthread_self() ) )
	    {
	      if( mx->kind == PTHREAD_MUTEX_RECURSIVE_NP )
		{
		  mx->recursive_count++;
		}
	      else
		{
		  result = EDEADLK;
		}
	    }
	  else
	    {
	      result = EBUSY;
	    }
	}
    }

  return(result);
}


