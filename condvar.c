/*
 * condvar.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
 */

/*
 * Code contributed by John E. Bossom <JEB>.
 */

#include <errno.h>
#include <string.h>

#include "pthread.h"
#include "implement.h"

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

  attr_result = calloc (1, sizeof (*attr_result));

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
  pthread_cond_t cv;

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

  if (cv != NULL)
    {
      result = ENOMEM;
      goto FAIL0;
    }

  cv->waiters = 0;
  cv->wasBroadcast = FALSE;

  if (sem_init (&(cv->sema), 0, 1) != 0)
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
  free (cv);
  cv = NULL;

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

  if (cond != NULL && *cond != NULL)
    {
      cv = *cond;

      (void) sem_destroy (&(cv->sema));
      (void) pthread_mutex_destroy (&(cv->waitersLock));
      (void) CloseHandle (cv->waitersDone);

      free (cv);

      *cond = NULL;
    }

  return (result);
}

int
pthread_cond_wait (pthread_cond_t * cond, pthread_mutex_t * mutex)
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
      * RESULTS
      *              0               caught condition; mutex released,
      *              EINVAL          'cond' or 'mutex' is invalid,
      *              EINVAL          different mutexes for concurrent waits,
      *              EINVAL          mutex is not held by the calling thread,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_cond_t cv;
  int lastWaiter;

  cv = *cond;

  /*
   * OK to increment  cv->waiters because the caller locked 'mutex'
   *
   * FIXME: This is true. However, it is technically possible to call cond_wait
   * on this cv with a different mutex. The standard leaves the result of such an
   * action as undefined. (RPJ)
   */
  cv->waiters++;

  /*
   * We keep the lock held just long enough to increment the count of
   * waiters by one (above).
   * Note that we can't keep it held across the
   * call to sem_wait since that will deadlock other calls
   * to pthread_cond_signal
   */
  if ((result = pthread_mutex_unlock (mutex)) == 0)
    {
      /*
       * Wait to be awakened by
       *              pthread_cond_signal, or
       *              pthread_cond_broadcast
       *
       * Note: 
       *      sem_wait is a cancellation point, hence providing the
       *      mechanism for making pthread_cond_wait a cancellation
       *      point. We use the cleanup mechanism to ensure we
       *  re-lock the mutex if we are cancelled.
       */
      pthread_cleanup_push (pthread_mutex_lock, mutex);

      result = sem_wait (&(cv->sema));

      pthread_cleanup_pop (0);
    }

  if ((result = pthread_mutex_lock (&(cv->waitersLock))) == 0)
    {
      /*
       * By making the waiter responsible for decrementing
       * its count we don't have to worry about having an internal
       * mutex.
       */
      cv->waiters--;

      lastWaiter = cv->wasBroadcast && (cv->waiters == 0);

      result = pthread_mutex_unlock (&(cv->waitersLock));
    }

  if (result == 0)
    {
      if (lastWaiter)
        {
          /*
           * If we are the last waiter on this broadcast
           * let the thread doing the broadcast proceed
           */
          if (!SetEvent (cv->waitersDone))
            {
              result = EINVAL;
            }
        }
    }

  /*
   * We must always regain the external mutex, even when
   * errors occur because that's the guarantee that we give
   * to our callers
   */
  (void) pthread_mutex_lock (mutex);


  return (result);

}                               /* pthread_cond_wait */


int
pthread_cond_timedwait (pthread_cond_t * cond,
                         pthread_mutex_t * mutex,
                         const struct timespec *abstime)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function initializes an unnamed semaphore. the
      *      initial value of the semaphore is 'value'
      *
      * PARAMETERS
      *      sem
      *              pointer to an instance of sem_t
      *
      *
      * DESCRIPTION
      *      This function  initializes an unnamed semaphore. The
      *      initial value of the semaphore is set to 'value'.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              EINVAL          'sem' is not a valid semaphore,
      *              ENOSPC          a required resource has been exhausted,
      *              ENOSYS          semaphores are not supported,
      *              EPERM           the process lacks appropriate privilege
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  /*
   * NOT IMPLEMENTED YET!!!
   */
  return (result);
}


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
  pthread_cond_t cv = *cond;

  /*
   * If there aren't any waiters, then this is a no-op.
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
      *      sem
      *              pointer to an instance of pthread_cond_t
      *
      *
      * DESCRIPTION
      *      This function  initializes an unnamed semaphore. The
      *      initial value of the semaphore is set to 'value'.
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
      *              0               successfully created semaphore,
      *              EINVAL          'cond' is invalid
      *              ENOSPC          a required resource has been exhausted,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;
  pthread_cond_t cv = *cond;
  int i;

  cv->wasBroadcast = TRUE;

  /*
   * Wake up all waiters
   */
  for (i = cv->waiters; i > 0 && result == 0; i--)
    {

      result = sem_post (&(cv->sema));
    }

  if (result == 0)
    {
      /*
       * Wait for all the awakened threads to acquire their part of
       * the counting semaphore
       */
      if (WaitForSingleObject (cv->waitersDone, INFINITE) !=
          WAIT_OBJECT_0)
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

/* </JEB> */


#if 0 /* Pre Bossom */

#include <errno.h>

#include <windows.h>
#include "pthread.h"

int
pthread_condattr_init(pthread_condattr_t *attr)
{
  return (attr == NULL) ? EINVAL : 0;
}

int
pthread_condattr_destroy(pthread_condattr_t *attr)
{
  return (attr == NULL) ? EINVAL : 0;
}

int
pthread_condattr_setpshared(pthread_condattr_t *attr,
			    int pshared)
{
  return (attr == NULL) ? EINVAL : ENOSYS;
}

int
pthread_condattr_getpshared(pthread_condattr_t *attr,
			    int *pshared)
{
  return (attr == NULL) ? EINVAL : ENOSYS;
}

int
pthread_cond_init(pthread_cond_t *cv, const pthread_condattr_t *attr)
{
  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* Initialize the count to 0. */
  cv->waiters_count = 0;

  /* Initialize the "mutex". FIXME: Check attributes arg. */
  pthread_mutex_init(&cv->waiters_count_lock, NULL);

  /* Create an auto-reset event. */
  cv->events[SIGNAL] = CreateEvent (NULL,     /* no security */
				    FALSE,    /* auto-reset event */
				    FALSE,    /* non-signaled initially */
				    NULL);    /* unnamed */

  /* Create a manual-reset event. */
  cv->events[BROADCAST] = CreateEvent (NULL,  /* no security */
				       TRUE,  /* manual-reset */
				       FALSE, /* non-signaled initially */
				       NULL); /* unnamed */

  return 0;
}

/* This is an internal routine that allows the functions `pthread_cond_wait' and
   `pthread_cond_timedwait' to share implementations.  The `abstime'
   parameter to this function is in millisecond units (or INFINITE). */

static int
cond_wait(pthread_cond_t *cv, pthread_mutex_t *mutex, DWORD abstime)
{
  int result, last_waiter;

  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* CANCELATION POINT */
  pthread_testcancel();

  /* Avoid race conditions. */
  pthread_mutex_lock(&cv->waiters_count_lock);
  cv->waiters_count++;
  pthread_mutex_unlock(&cv->waiters_count_lock);

  /* It's okay to release the mutex here since Win32 manual-reset
     events maintain state when used with SetEvent().  This avoids the
     "lost wakeup" bug. */

  pthread_mutex_unlock(mutex);

  /* Wait for either event to become signaled due to
     pthread_cond_signal() being called or pthread_cond_broadcast()
     being called. */
 
  result = WaitForMultipleObjects (2, cv->events, FALSE, abstime);

  pthread_mutex_lock (&cv->waiters_count_lock);
  cv->waiters_count--;
  last_waiter = cv->waiters_count == 0;
  pthread_mutex_unlock (&cv->waiters_count_lock);

  /* Some thread called pthread_cond_broadcast(). */
  if ((result == WAIT_OBJECT_0 + BROADCAST) && last_waiter)
    {
      /* We're the last waiter to be notified, so reset the manual
	 event. */
      ResetEvent(cv->events[BROADCAST]);
    }

  /* Reacquire the mutex. */
  pthread_mutex_lock(mutex);

  return 0;
}

int
pthread_cond_wait(pthread_cond_t *cv,
		  pthread_mutex_t *mutex)
{
  return cond_wait(cv, mutex, INFINITE);
}

/* Assume that our configure script will test for the existence of
   `struct timespec' and define it according to POSIX if it isn't
   found.  This will enable people to use this implementation
   without necessarily needing Cygwin32. */

int
pthread_cond_timedwait(pthread_cond_t *cv, 
		       pthread_mutex_t *mutex,
		       const struct timespec *abstime)
{
  DWORD msecs;
  
  /* Calculate the number of milliseconds in abstime. */
  msecs = abstime->tv_sec * 1000;
  msecs += abstime->tv_nsec / 1000000;

  return cond_wait(cv, mutex, msecs);
}

int 
pthread_cond_broadcast (pthread_cond_t *cv)
{
  int have_waiters;

  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* Avoid race conditions. */
  pthread_mutex_lock (&cv->waiters_count_lock);
  have_waiters = (cv->waiters_count > 0);
  pthread_mutex_unlock (&cv->waiters_count_lock);

  if (have_waiters) {
    SetEvent(cv->events[BROADCAST]);
  }

  return 0;
}

int 
pthread_cond_signal (pthread_cond_t *cv)
{
  int have_waiters;

  /* Ensure we have a valid cond_t variable. */
  if (cv == NULL)
    {
      return EINVAL;
    }

  /* Avoid race conditions. */
  pthread_mutex_lock (&cv->waiters_count_lock);
  have_waiters = (cv->waiters_count > 0);
  pthread_mutex_unlock (&cv->waiters_count_lock);

  if (have_waiters) {
    SetEvent(cv->events[SIGNAL]);
  }

  return 0;
}

int
pthread_cond_destroy(pthread_cond_t *cv)
{
  if (cv == NULL)
    {
	return EINVAL;
    }

  return pthread_mutex_destroy(&cv->waiters_count_lock);
}

#endif /* Pre Bossom */
