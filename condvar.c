/*
 * condvar.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
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

static int
ptw32_cond_check_need_init(pthread_cond_t *cond)
{
  int result = 0;

  /*
   * The following guarded test is specifically for statically
   * initialised condition variables (via PTHREAD_OBJECT_INITIALIZER).
   *
   * Note that by not providing this synchronisation we risk
   * introducing race conditions into applications which are
   * correctly written.
   *
   * Approach
   * --------
   * We know that static condition variables will not be PROCESS_SHARED
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
  EnterCriticalSection(&ptw32_cond_test_init_lock);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section.
   * If a static cv has been destroyed, the application can
   * re-initialise it only by calling pthread_cond_init()
   * explicitly.
   */
  if (*cond == (pthread_cond_t) PTW32_OBJECT_AUTO_INIT)
    {
      result = pthread_cond_init(cond, NULL);
    }
  else if (*cond == NULL)
    {
      /*
       * The cv has been destroyed while we were waiting to
       * initialise it, so the operation that caused the
       * auto-initialisation should fail.
       */
      result = EINVAL;
    }

  LeaveCriticalSection(&ptw32_cond_test_init_lock);

  return(result);
}


int
pthread_condattr_init (pthread_condattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Initializes a condition variable attributes object
      *      with default attributes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_condattr_t
      *
      *
      * DESCRIPTION
      *      Initializes a condition variable attributes object
      *      with default attributes.
      *
      *      NOTES:
      *              1)      Use to define condition variable types
      *              2)      It is up to the application to ensure
      *                      that it doesn't re-init an attribute
      *                      without destroying it first. Otherwise
      *                      a memory leak is created.
      *
      * RESULTS
      *              0               successfully initialized attr,
      *              ENOMEM          insufficient memory for attr.
      *
      * ------------------------------------------------------
      */
{
  pthread_condattr_t attr_result;
  int result = 0;

  attr_result = (pthread_condattr_t) calloc (1, sizeof (*attr_result));

  if (attr_result == NULL)
    {
      result = ENOMEM;
    }

  *attr = attr_result;

  return (result);

}                               /* pthread_condattr_init */


int
pthread_condattr_destroy (pthread_condattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Destroys a condition variable attributes object.
      *      The object can no longer be used.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_condattr_t
      *
      *
      * DESCRIPTION
      *      Destroys a condition variable attributes object.
      *      The object can no longer be used.
      *
      *      NOTES:
      *      1)      Does not affect condition variables created
      *              using 'attr'
      *
      * RESULTS
      *              0               successfully released attr,
      *              EINVAL          'attr' is invalid.
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
      free (*attr);

      *attr = NULL;
      result = 0;
    }

  return (result);

}                               /* pthread_condattr_destroy */


int
pthread_condattr_getpshared (const pthread_condattr_t * attr, int *pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Determine whether condition variables created with 'attr'
      *      can be shared between processes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_condattr_t
      *
      *      pshared
      *              will be set to one of:
      *
      *                      PTHREAD_PROCESS_SHARED
      *                              May be shared if in shared memory
      *
      *                      PTHREAD_PROCESS_PRIVATE
      *                              Cannot be shared.
      *
      *
      * DESCRIPTION
      *      Condition Variables created with 'attr' can be shared
      *      between processes if pthread_cond_t variable is allocated
      *      in memory shared by these processes.
      *      NOTES:
      *      1)      pshared condition variables MUST be allocated in
      *              shared memory.
      *
      *      2)      The following macro is defined if shared mutexes
      *              are supported:
      *                      _POSIX_THREAD_PROCESS_SHARED
      *
      * RESULTS
      *              0               successfully retrieved attribute,
      *              EINVAL          'attr' is invalid,
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
      *pshared = PTHREAD_PROCESS_PRIVATE;
      result = EINVAL;
    }

  return (result);

}                               /* pthread_condattr_getpshared */


int
pthread_condattr_setpshared (pthread_condattr_t * attr, int pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Mutexes created with 'attr' can be shared between
      *      processes if pthread_mutex_t variable is allocated
      *      in memory shared by these processes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_mutexattr_t
      *
      *      pshared
      *              must be one of:
      *
      *                      PTHREAD_PROCESS_SHARED
      *                              May be shared if in shared memory
      *
      *                      PTHREAD_PROCESS_PRIVATE
      *                              Cannot be shared.
      *
      * DESCRIPTION
      *      Mutexes creatd with 'attr' can be shared between
      *      processes if pthread_mutex_t variable is allocated
      *      in memory shared by these processes.
      *
      *      NOTES:
      *              1)      pshared mutexes MUST be allocated in shared
      *                      memory.
      *
      *              2)      The following macro is defined if shared mutexes
      *                      are supported:
      *                              _POSIX_THREAD_PROCESS_SHARED
      *
      * RESULTS
      *              0               successfully set attribute,
      *              EINVAL          'attr' or pshared is invalid,
      *              ENOSYS          PTHREAD_PROCESS_SHARED not supported,
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

}                               /* pthread_condattr_setpshared */


int
pthread_cond_init (pthread_cond_t * cond, const pthread_condattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function initializes a condition variable.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *      attr
      *              specifies optional creation attributes.
      *
      *
      * DESCRIPTION
      *      This function initializes a condition variable.
      *
      * RESULTS
      *              0               successfully created condition variable,
      *              EINVAL          'attr' is invalid,
      *              EAGAIN          insufficient resources (other than
      *                              memory,
      *              ENOMEM          insufficient memory,
      *              EBUSY           'cond' is already initialized,
      *
      * ------------------------------------------------------
      */
{
  int result = EAGAIN;
  pthread_cond_t cv = NULL;

  if (cond == NULL)
    {
      return EINVAL;
    }

  if ((attr != NULL && *attr != NULL) &&
      ((*attr)->pshared == PTHREAD_PROCESS_SHARED))
    {
      /*
       * Creating condition variable that can be shared between
       * processes.
       */
      result = ENOSYS;

      goto FAIL0;
    }

  cv = (pthread_cond_t) calloc (1, sizeof (*cv));

  if (cv == NULL)
    {
      result = ENOMEM;
      goto FAIL0;
    }

  cv->waiters = 0;
  cv->wasBroadcast = FALSE;

  if (sem_init (&(cv->sema), 0, 0) != 0)
    {
      goto FAIL0;
    }
  if (pthread_mutex_init (&(cv->waitersLock), NULL) != 0)
    {
      goto FAIL1;
    }

  cv->waitersDone = CreateEvent (
				 0,
				 (int) FALSE,  /* manualReset  */
				 (int) FALSE,  /* setSignaled  */
				 NULL);

  if (cv->waitersDone == NULL)
    {
      goto FAIL2;
    }

  result = 0;

  goto DONE;

  /*
   * -------------
   * Failure Code
   * -------------
   */
FAIL2:
  (void) pthread_mutex_destroy (&(cv->waitersLock));

FAIL1:
  (void) sem_destroy (&(cv->sema));

FAIL0:
DONE:
  *cond = cv;

  return (result);

}                               /* pthread_cond_init */


int
pthread_cond_destroy (pthread_cond_t * cond)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function destroys a condition variable
      *
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *
      * DESCRIPTION
      *      This function destroys a condition variable.
      *
      *      NOTES:
      *              1)      Safest after wakeup from 'cond', when
      *                      no other threads will wait.
      *
      * RESULTS
      *              0               successfully released condition variable,
      *              EINVAL          'cond' is invalid,
      *              EBUSY           'cond' is in use,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_cond_t cv;

  /*
   * Assuming any race condition here is harmless.
   */
  if (cond == NULL 
      || *cond == NULL)
    {
      return EINVAL;
    }

  if (*cond != (pthread_cond_t) PTW32_OBJECT_AUTO_INIT)
    {
      cv = *cond;

      if (pthread_mutex_lock(&(cv->waitersLock)) != 0)
	{
	  return EINVAL;
	}

      if (cv->waiters > 0)
	{
	  (void) pthread_mutex_unlock(&(cv->waitersLock));
	  return EBUSY;
	}

      (void) sem_destroy (&(cv->sema));
      (void) CloseHandle (cv->waitersDone);
      (void) pthread_mutex_unlock(&(cv->waitersLock));
      (void) pthread_mutex_destroy (&(cv->waitersLock));

      free(cv);
      *cond = NULL;
    }
  else
    {
      /*
       * See notes in ptw32_cond_check_need_init() above also.
       */
      EnterCriticalSection(&ptw32_cond_test_init_lock);

      /*
       * Check again.
       */
      if (*cond == (pthread_cond_t) PTW32_OBJECT_AUTO_INIT)
        {
          /*
           * This is all we need to do to destroy a statically
           * initialised cond that has not yet been used (initialised).
           * If we get to here, another thread
           * waiting to initialise this cond will get an EINVAL.
           */
          *cond = NULL;
        }
      else
        {
          /*
           * The cv has been initialised while we were waiting
           * so assume it's in use.
           */
          result = EBUSY;
        }

      LeaveCriticalSection(&ptw32_cond_test_init_lock);
    }

  return (result);
}

/*
 * Arguments for cond_wait_cleanup, since we can only pass a
 * single void * to it.
 */
typedef struct {
  pthread_mutex_t * mutexPtr;
  pthread_cond_t cv;
  int * resultPtr;
} ptw32_cond_wait_cleanup_args_t;

static void
ptw32_cond_wait_cleanup(void * args)
{
  ptw32_cond_wait_cleanup_args_t * cleanup_args = (ptw32_cond_wait_cleanup_args_t *) args;
  pthread_mutex_t * mutexPtr = cleanup_args->mutexPtr;
  pthread_cond_t cv = cleanup_args->cv;
  int * resultPtr = cleanup_args->resultPtr;
  int lastWaiter = FALSE;

  /*
   * Whether we got here legitimately or because of an error we
   * indicate that we are no longer waiting. The alternative
   * will result in never signaling the broadcasting thread.
   */
  if (pthread_mutex_lock (&(cv->waitersLock)) == 0)
    {
      /*
       * The waiter is responsible for decrementing
       * its count, protected by an internal mutex.
       */

      cv->waiters--;

      lastWaiter = cv->wasBroadcast && (cv->waiters == 0);

      if (lastWaiter)
        {
          cv->wasBroadcast = FALSE;
        }

      (void) pthread_mutex_unlock (&(cv->waitersLock));
    }

  /*
   * If we are the last waiter on this broadcast
   * let the thread doing the broadcast proceed
   */
  if (lastWaiter && !SetEvent (cv->waitersDone))
    {
      *resultPtr = EINVAL;
    }

  /*
   * We must always regain the external mutex, even when
   * errors occur, because that's the guarantee that we give
   * to our callers.
   *
   * Note that the broadcasting thread may already own the lock.
   * The standard actually requires that the signaling thread hold
   * the lock at the time that it signals if the developer wants
   * predictable scheduling behaviour. It's up to the developer.
   * In that case all waiting threads will block here until
   * the broadcasting thread releases the lock, having been
   * notified by the last waiting thread (SetEvent call above).
   */
  (void) pthread_mutex_lock (mutexPtr);
}

static int
ptw32_cond_timedwait (pthread_cond_t * cond, 
                      pthread_mutex_t * mutex,
                      const struct timespec *abstime)
{
  int result = 0;
  pthread_cond_t cv;
  ptw32_cond_wait_cleanup_args_t cleanup_args;

  if (cond == NULL || *cond == NULL)
    {
      return EINVAL;
    }

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static condition variable. We check
   * again inside the guarded section of ptw32_cond_check_need_init()
   * to avoid race conditions.
   */
  if (*cond == (pthread_cond_t) PTW32_OBJECT_AUTO_INIT)
    {
      result = ptw32_cond_check_need_init(cond);
    }

  if (result != 0 && result != EBUSY)
    {
      return result;
    }

  cv = *cond;

  /*
   * It's not OK to increment cond->waiters while the caller locked 'mutex',
   * there may be other threads just waking up (with 'mutex' unlocked)
   * and cv->... data is not protected.
   */
  if (pthread_mutex_lock(&(cv->waitersLock)) != 0)
    {
      return EINVAL;
    }

  cv->waiters++;

  if (pthread_mutex_unlock(&(cv->waitersLock)) != 0)
    {
      return EINVAL;
    }

  /*
   * We keep the lock held just long enough to increment the count of
   * waiters by one (above).
   * Note that we can't keep it held across the
   * call to sem_wait since that will deadlock other calls
   * to pthread_cond_signal
   */
  cleanup_args.mutexPtr = mutex;
  cleanup_args.cv = cv;
  cleanup_args.resultPtr = &result;

  pthread_cleanup_push (ptw32_cond_wait_cleanup, (void *) &cleanup_args);

  if ((result = pthread_mutex_unlock (mutex)) == 0)
    {
      /*
       * Wait to be awakened by
       *              pthread_cond_signal, or
       *              pthread_cond_broadcast, or
       *              a timeout
       *
       * Note: 
       *      ptw32_sem_timedwait is a cancelation point,
       *      hence providing the
       *      mechanism for making pthread_cond_wait a cancelation
       *      point. We use the cleanup mechanism to ensure we
       *      re-lock the mutex and decrement the waiters count
       *      if we are canceled.
       */
      if (ptw32_sem_timedwait (&(cv->sema), abstime) == -1)
	{
	  result = errno;
	}
    }

  pthread_cleanup_pop (1);  /* Always cleanup */

  /*
   * "result" can be modified by the cleanup handler.
   * Specifically, if we are the last waiting thread and failed
   * to notify the broadcast thread to proceed.
   */
  return (result);

}                               /* ptw32_cond_timedwait */


int
pthread_cond_wait (pthread_cond_t * cond,
		   pthread_mutex_t * mutex)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function waits on a condition variable until
      *      awakened by a signal or broadcast.
      *
      *      Caller MUST be holding the mutex lock; the
      *      lock is released and the caller is blocked waiting
      *      on 'cond'. When 'cond' is signaled, the mutex
      *      is re-acquired before returning to the caller.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *      mutex
      *              pointer to an instance of pthread_mutex_t
      *
      *
      * DESCRIPTION
      *      This function waits on a condition variable until
      *      awakened by a signal or broadcast.
      *
      *      NOTES:
      *      1)      The function must be called with 'mutex' LOCKED
      *               by the calling thread, or undefined behaviour
      *              will result.
      *
      *      2)      This routine atomically releases 'mutex' and causes
      *              the calling thread to block on the condition variable.
      *              The blocked thread may be awakened by 
      *                      pthread_cond_signal or 
      *                      pthread_cond_broadcast.
      *
      * Upon successful completion, the 'mutex' has been locked and 
      * is owned by the calling thread.
      *
      *
      * RESULTS
      *              0               caught condition; mutex released,
      *              EINVAL          'cond' or 'mutex' is invalid,
      *              EINVAL          different mutexes for concurrent waits,
      *              EINVAL          mutex is not held by the calling thread,
      *
      * ------------------------------------------------------
      */
{
  /* The NULL abstime arg means INFINITE waiting. */
  return(ptw32_cond_timedwait(cond, mutex, NULL));
}                               /* pthread_cond_wait */


int
pthread_cond_timedwait (pthread_cond_t * cond, 
                        pthread_mutex_t * mutex,
                        const struct timespec *abstime)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function waits on a condition variable either until
      *      awakened by a signal or broadcast; or until the time
      *      specified by abstime passes.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *      mutex
      *              pointer to an instance of pthread_mutex_t
      *
      *      abstime
      *              pointer to an instance of (const struct timespec)
      *
      *
      * DESCRIPTION
      *      This function waits on a condition variable either until
      *      awakened by a signal or broadcast; or until the time
      *      specified by abstime passes.
      *
      *      NOTES:
      *      1)      The function must be called with 'mutex' LOCKED
      *               by the calling thread, or undefined behaviour
      *              will result.
      *
      *      2)      This routine atomically releases 'mutex' and causes
      *              the calling thread to block on the condition variable.
      *              The blocked thread may be awakened by 
      *                      pthread_cond_signal or 
      *                      pthread_cond_broadcast.
      *
      *
      * RESULTS
      *              0               caught condition; mutex released,
      *              EINVAL          'cond', 'mutex', or abstime is invalid,
      *              EINVAL          different mutexes for concurrent waits,
      *              EINVAL          mutex is not held by the calling thread,
      *              ETIMEDOUT       abstime ellapsed before cond was signaled.
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if (abstime == NULL)
    {
      result = EINVAL;
    }
  else
    {
      result = ptw32_cond_timedwait(cond, mutex, abstime);
    }

  return(result);
}                               /* pthread_cond_timedwait */


int
pthread_cond_signal (pthread_cond_t * cond)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function signals a condition variable, waking
      *      one waiting thread.
      *      If SCHED_FIFO or SCHED_RR policy threads are waiting
      *      the highest priority waiter is awakened; otherwise,
      *      an unspecified waiter is awakened.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *
      * DESCRIPTION
      *      This function signals a condition variable, waking
      *      one waiting thread.
      *      If SCHED_FIFO or SCHED_RR policy threads are waiting
      *      the highest priority waiter is awakened; otherwise,
      *      an unspecified waiter is awakened.
      *
      *      NOTES:
      *      1)      Use when any waiter can respond and only one need
      *              respond (all waiters being equal).
      *
      *      2)      This function MUST be called under the protection 
      *              of the SAME mutex that is used with the condition
      *              variable being signaled; OTHERWISE, the condition
      *              variable may be signaled between the test of the
      *              associated condition and the blocking
      *              pthread_cond_signal.
      *              This can cause an infinite wait.
      *
      * RESULTS
      *              0               successfully signaled condition,
      *              EINVAL          'cond' is invalid,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_cond_t cv;

  if (cond == NULL || *cond == NULL)
    {
      return EINVAL;
    }

  cv = *cond;

  /*
   * No-op if the CV is static and hasn't been initialised yet.
   * Assuming that race conditions are harmless.
   */
  if (cv == (pthread_cond_t) PTW32_OBJECT_AUTO_INIT)
    {
      return 0;
    }

  /*
   * If there aren't any waiters, then this is a no-op.
   * Assuming that race conditions are harmless.
   */
  if (cv->waiters > 0)
    {
      result = sem_post (&(cv->sema));
    }

  return (result);

}                               /* pthread_cond_signal */

int
pthread_cond_broadcast (pthread_cond_t * cond)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function broadcasts the condition variable,
      *      waking all current waiters.
      *
      * PARAMETERS
      *      cond
      *              pointer to an instance of pthread_cond_t
      *
      *
      * DESCRIPTION
      *      This function signals a condition variable, waking
      *      all waiting threads.
      *
      *      NOTES:
      *      1)      This function MUST be called under the protection
      *              of the SAME mutex that is used with the condition
      *              variable being signaled; OTHERWISE, the condition
      *              variable may be signaled between the test of the
      *              associated condition and the blocking pthread_cond_wait.
      *              This can cause an infinite wait.
      *
      *      2)      Use when more than one waiter may respond to
      *              predicate change or if any waiting thread may
      *              not be able to respond
      *
      * RESULTS
      *              0               successfully signalled condition to all
      *                              waiting threads,
      *              EINVAL          'cond' is invalid
      *              ENOSPC          a required resource has been exhausted,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  int wereWaiters = FALSE;
  pthread_cond_t cv;

  if (cond == NULL || *cond == NULL)
    {
      return EINVAL;
    }

  cv = *cond;

  /*
   * No-op if the CV is static and hasn't been initialised yet.
   * Assuming that any race condition is harmless.
   */
  if (cv == (pthread_cond_t) PTW32_OBJECT_AUTO_INIT)
    {
      return 0;
    }

  if (pthread_mutex_lock(&(cv->waitersLock)) == EINVAL)
    {
      return EINVAL;
    }

  cv->wasBroadcast = TRUE;
  wereWaiters = (cv->waiters > 0);

  if (wereWaiters)
    {
      /*
       * Wake up all waiters
       */

#ifdef NEED_SEM

      result = (ptw32_increase_semaphore( &cv->sema, cv->waiters )
		? 0
		: EINVAL);

#else /* NEED_SEM */

      result = (ReleaseSemaphore( cv->sema, cv->waiters, NULL )
		? 0
		: EINVAL);

#endif /* NEED_SEM */

    }

  (void) pthread_mutex_unlock(&(cv->waitersLock));

  if (wereWaiters && result == 0)
    {
      /*
       * Wait for all the awakened threads to acquire their part of
       * the counting semaphore
       */
      if (WaitForSingleObject (cv->waitersDone, INFINITE)
          == WAIT_OBJECT_0)
        {
          result = 0;
        }
      else
        {
          result = EINVAL;
        }

    }

  return (result);

}
