/*
 * condvar.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
 */

/*
 * Code contributed by John E. Bossom <JEB>.
 */

#include "pthread.h"
#include "implement.h"

static int
_cond_check_need_init(pthread_cond_t *cond)
{
  int result = 0;

  /*
   * The following guarded test is specifically for statically
   * initialised condition variables (via PTHREAD_COND_INITIALIZER).
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
   * We still have a problem in that we would like a per-mutex
   * lock, but attempting to create one will again lead
   * to a race condition. We are forced to use a global
   * lock in this instance.
   *
   * If using a single global lock slows applications down too much,
   * multiple global locks could be created and hashed on some random
   * value associated with each mutex, the pointer perhaps. At a guess,
   * a good value for the optimal number of global locks might be
   * the number of processors + 1.
   *
   * We need to maintain the following per-cv state independently:
   *   - cv staticinit (true iff cv is static and still
   *                    needs to be initialised)
   *   - cv valid (false iff cv has been destroyed)
   *
   * For example, in this implementation a cv initialised by
   * PTHREAD_COND_INITIALIZER will be 'valid' but uninitialised until
   * the thread attempts to use it. It can also be destroyed (made invalid)
   * before ever being used.
   */
  EnterCriticalSection(&_pthread_cond_test_init_lock);

  /*
   * We got here because staticinit tested true, possibly under race
   * conditions. Check staticinit again inside the critical section
   * and only initialise if the cv is valid (not been destroyed).
   * If a static cv has been destroyed, the application can
   * re-initialise it only by calling pthread_cond_init()
   * explicitly.
   */
  if (cond->staticinit && cond->valid)
    {
      result = pthread_cond_init(cond, NULL);
    }

  LeaveCriticalSection(&_pthread_cond_test_init_lock);

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

  if (cond == NULL)
    {
      return EINVAL;
    }

  /* 
   * Assuming any race condition here is harmless.
   */
  if (cond->valid && !cond->staticinit)
    {
      return EBUSY;
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

  cond->waiters = 0;
  cond->wasBroadcast = FALSE;

  if (_pthread_sem_init (&(cond->sema), 0, 0) != 0)
    {
      goto FAIL0;
    }
  if (pthread_mutex_init (&(cond->waitersLock), NULL) != 0)
    {
      goto FAIL1;
    }

  cond->waitersDone = CreateEvent (
                                  0,
                                  (int) FALSE,  /* manualReset  */
                                  (int) FALSE,  /* setSignaled  */
                                  NULL);

  if (cond->waitersDone == NULL)
    {
      goto FAIL2;
    }

  cond->staticinit = 0;

  /* Mark as valid. */
  cond->valid = 1;

  result = 0;

  goto DONE;

  /*
   * -------------
   * Failure Code
   * -------------
   */
FAIL2:
  (void) pthread_mutex_destroy (&(cond->waitersLock));

FAIL1:
  (void) _pthread_sem_destroy (&(cond->sema));

FAIL0:
DONE:
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

  /*
   * Assuming any race condition here is harmless.
   */
  if (cond == NULL || cond->valid == 0 || cond->staticinit == 1)
    {
      return EINVAL;
    }

  if (cond->waiters > 0)
    {
      return EBUSY;
    }

  (void) _pthread_sem_destroy (&(cond->sema));
  (void) pthread_mutex_destroy (&(cond->waitersLock));
  (void) CloseHandle (cond->waitersDone);
      
  cond->valid = 0;

  return (result);
}

static int
cond_timedwait (pthread_cond_t * cond, 
		pthread_mutex_t * mutex,
		const struct timespec *abstime)
{
  int result = 0;
  int internal_result = 0;
  int lastWaiter = FALSE;

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static condition variable. We check 'staticinit'
   * again inside the guarded section of _cond_check_need_init()
   * to avoid race conditions.
   */
  if (cond->staticinit == 1)
    {
      result = _cond_check_need_init(cond);
    }

  /*
   * OK to increment  cond->waiters because the caller locked 'mutex'
   */
  cond->waiters++;

  /*
   * We keep the lock held just long enough to increment the count of
   * waiters by one (above).
   * Note that we can't keep it held across the
   * call to _pthread_sem_wait since that will deadlock other calls
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
       *      _pthread_sem_timedwait is a cancellation point,
       *      hence providing the
       *      mechanism for making pthread_cond_wait a cancellation
       *      point. We use the cleanup mechanism to ensure we
       *      re-lock the mutex if we are cancelled.
       */
      pthread_cleanup_push (pthread_mutex_lock, mutex);

      result = _pthread_sem_timedwait (&(cond->sema), abstime);

      pthread_cleanup_pop (0);
    }

  if ((internal_result = pthread_mutex_lock (&(cond->waitersLock))) == 0)
    {
      /*
       * By making the waiter responsible for decrementing
       * its count we don't have to worry about having an internal
       * mutex.
       */
      cond->waiters--;

      lastWaiter = cond->wasBroadcast && (cond->waiters == 0);

      internal_result = pthread_mutex_unlock (&(cond->waitersLock));
    }

  if (result == 0 && internal_result == 0)
    {
      if (lastWaiter)
        {
          /*
           * If we are the last waiter on this broadcast
           * let the thread doing the broadcast proceed
           */
          if (!SetEvent (cond->waitersDone))
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

}                               /* cond_timedwait */


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
  return(cond_timedwait(cond, mutex, NULL));
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
      result = cond_timedwait(cond, mutex, abstime);
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

  if (cond == NULL || cond->valid == 0)
    {
      return EINVAL;
    }

  /*
   * No-op if the CV is static and hasn't been initialised yet.
   */
  if (cond->staticinit == 1)
    {
      return 0;
    }

  /*
   * If there aren't any waiters, then this is a no-op.
   */
  if (cond->waiters > 0)
    {
      result = _pthread_sem_post (&(cond->sema));
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
  int i;

  if (cond == NULL || cond->valid == 0)
    {
      return EINVAL;
    }

  cond->wasBroadcast = TRUE;

  /*
   * No-op if the CV is static and hasn't been initialised yet.
   */
  if (cond->staticinit == 1)
    {
      return 0;
    }

  /*
   * Wake up all waiters
   */
  for (i = cond->waiters; i > 0 && result == 0; i--)
    {

      result = _pthread_sem_post (&(cond->sema));
    }

  if (cond->waiters > 0 && result == 0)
    {
      /*
       * Wait for all the awakened threads to acquire their part of
       * the counting semaphore
       */
      if (WaitForSingleObject (cond->waitersDone, INFINITE) !=
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

