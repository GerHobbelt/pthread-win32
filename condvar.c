/*
 * condvar.c
 *
 * Description:
 * This translation unit implements condition variables and their primitives.
 *
 * Algorithm:
 * The algorithm used in this implementation is that developed by
 * Alexander Terekhov in colaboration with Louis Thomas. The bulk
 * of the discussion is recorded in the file README.CV, which contains
 * several generations of both colaborators original algorithms. The final
 * algorithm used here is the one referred to as
 *
 *     Algorithm 8a / IMPL_SEM,UNBLOCK_STRATEGY == UNBLOCK_ALL
 * 
 * presented below in pseudo-code as it appeared:
 *
 *
 * given:
 * semBlockLock - bin.semaphore
 * semBlockQueue - semaphore
 * mtxExternal - mutex or CS
 * mtxUnblockLock - mutex or CS
 * nWaitersGone - int
 * nWaitersBlocked - int
 * nWaitersToUnblock - int
 * 
 * wait( timeout ) {
 * 
 *   [auto: register int result          ]     // error checking omitted
 *   [auto: register int nSignalsWasLeft ]
 *   [auto: register int nWaitersWasGone ]
 * 
 *   sem_wait( semBlockLock );
 *   nWaitersBlocked++;
 *   sem_post( semBlockLock );
 * 
 *   unlock( mtxExternal );
 *   bTimedOut = sem_wait( semBlockQueue,timeout );
 * 
 *   lock( mtxUnblockLock );
 *   if ( 0 != (nSignalsWasLeft = nWaitersToUnblock) ) {
 *     if ( bTimeout ) {                       // timeout (or canceled)
 *       if ( 0 != nWaitersBlocked ) {
 *         nWaitersBlocked--;
 *       }
 *       else {
 *         nWaitersGone++;                     // count spurious wakeups.
 *       }
 *     }
 *     if ( 0 == --nWaitersToUnblock ) {
 *       if ( 0 != nWaitersBlocked ) {
 *         sem_post( semBlockLock );           // open the gate.
 *         nSignalsWasLeft = 0;                // do not open the gate
 *                                             // below again.
 *       }
 *       else if ( 0 != (nWaitersWasGone = nWaitersGone) ) {
 *         nWaitersGone = 0;
 *       }
 *     }
 *   }
 *   else if ( INT_MAX/2 == ++nWaitersGone ) { // timeout/canceled or
 *                                             // spurious semaphore :-)
 *     sem_wait( semBlockLock );
 *     nWaitersBlocked -= nWaitersGone;     // something is going on here
 *                                          //  - test of timeouts? :-)
 *     sem_post( semBlockLock );
 *     nWaitersGone = 0;
 *   }
 *   unlock( mtxUnblockLock );
 * 
 *   if ( 1 == nSignalsWasLeft ) {
 *     if ( 0 != nWaitersWasGone ) {
 *       // sem_adjust( semBlockQueue,-nWaitersWasGone );
 *       while ( nWaitersWasGone-- ) {
 *         sem_wait( semBlockQueue );       // better now than spurious later
 *       }
 *     } sem_post( semBlockLock );          // open the gate
 *   }
 * 
 *   lock( mtxExternal );
 * 
 *   return ( bTimedOut ) ? ETIMEOUT : 0;
 * }
 * 
 * signal(bAll) {
 * 
 *   [auto: register int result         ]
 *   [auto: register int nSignalsToIssue]
 * 
 *   lock( mtxUnblockLock );
 * 
 *   if ( 0 != nWaitersToUnblock ) {        // the gate is closed!!!
 *     if ( 0 == nWaitersBlocked ) {        // NO-OP
 *       return unlock( mtxUnblockLock );
 *     }
 *     if (bAll) {
 *       nWaitersToUnblock += nSignalsToIssue=nWaitersBlocked;
 *       nWaitersBlocked = 0;
 *     }
 *     else {
 *       nSignalsToIssue = 1;
 *       nWaitersToUnblock++;
 *       nWaitersBlocked--;
 *     }
 *   }
 *   else if ( nWaitersBlocked > nWaitersGone ) { // HARMLESS RACE CONDITION!
 *     sem_wait( semBlockLock );                  // close the gate
 *     if ( 0 != nWaitersGone ) {
 *       nWaitersBlocked -= nWaitersGone;
 *       nWaitersGone = 0;
 *     }
 *     if (bAll) {
 *       nSignalsToIssue = nWaitersToUnblock = nWaitersBlocked;
 *       nWaitersBlocked = 0;
 *     }
 *     else {
 *       nSignalsToIssue = nWaitersToUnblock = 1;
 *       nWaitersBlocked--;
 *     }
 *   }
 *   else { // NO-OP
 *     return unlock( mtxUnblockLock );
 *   }
 * 
 *   unlock( mtxUnblockLock );
 *   sem_post( semBlockQueue,nSignalsToIssue );
 *   return result;
 * }
 *
 * -------------------------------------------------------------
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

static INLINE int
ptw32_cond_check_need_init (pthread_cond_t *cond)
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
  if (*cond == PTHREAD_COND_INITIALIZER)
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

  return result;
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

  return result;

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
      (void) free (*attr);

      *attr = NULL;
      result = 0;
    }

  return result;

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

  if ((attr != NULL && *attr != NULL) && (pshared != NULL))
    {
      *pshared = (*attr)->pshared;
      result = 0;
    }
  else
    {
      *pshared = PTHREAD_PROCESS_PRIVATE;
      result = EINVAL;
    }

  return result;

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

  if ((attr != NULL && *attr != NULL)
      && ((pshared == PTHREAD_PROCESS_SHARED)
          || (pshared == PTHREAD_PROCESS_PRIVATE)))
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

  return result;

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
  int result;
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
      goto DONE;
    }

  cv = (pthread_cond_t) calloc(1, sizeof (*cv));

  if (cv == NULL)
    {
      result = ENOMEM;
      goto DONE;
    }

  cv->nWaitersBlocked   = 0;
  cv->nWaitersUnblocked = 0;
  cv->nWaitersToUnblock = 0;
  cv->nWaitersGone      = 0;

  if (sem_init(&(cv->semBlockLock), 0, 1) != 0)
    {
      result = errno;
      goto FAIL0;
    }

  if (sem_init(&(cv->semBlockQueue), 0, 0) != 0)
    {
      result = errno;
      goto FAIL1;
    }

  if ((result = pthread_mutex_init(&(cv->mtxUnblockLock), 0)) != 0)
    {
      goto FAIL2;
    }

  result = 0;

  goto DONE;

  /*
   * -------------
   * Failed...
   * -------------
   */
FAIL2:
  (void) sem_destroy(&(cv->semBlockQueue));

FAIL1:
  (void) sem_destroy(&(cv->semBlockLock));

FAIL0:
  (void) free(cv);
  cv = NULL;

DONE:
  *cond = cv;

  return result;

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
  pthread_cond_t cv;
  int result = 0, result1 = 0, result2 = 0;

  /*
   * Assuming any race condition here is harmless.
   */
  if (cond == NULL 
      || *cond == NULL)
    {
      return EINVAL;
    }

  if (*cond != PTHREAD_COND_INITIALIZER)
    {
      cv = *cond;

      /*
       * Synchronize access to waiters blocked count (LEVEL-1)
       */
      if (sem_wait(&(cv->semBlockLock)) != 0)
        {
          return errno;
        }

      /*
       * Synchronize access to waiters (to)unblock(ed) counts (LEVEL-2)
       */
      if ((result = pthread_mutex_lock(&(cv->mtxUnblockLock))) != 0)
        {
          (void) sem_post(&(cv->semBlockLock));
          return result;
        }

      /*
       * Check whether cv is still busy (still has waiters)
       */
      if (cv->nWaitersBlocked - cv->nWaitersGone - cv->nWaitersUnblocked > 0)
        {
          if (sem_post(&(cv->semBlockLock)) != 0)
            {
              result = errno;
            }
          result1 = pthread_mutex_unlock(&(cv->mtxUnblockLock));
          result2 = EBUSY;
        }
      else
        {
          /*
           * Now it is safe to destroy
           */
          *cond = NULL;
          if (sem_destroy(&(cv->semBlockLock)) != 0)
            {
              result = errno;
            }
          if (sem_destroy(&(cv->semBlockQueue)) != 0)
            {
              result1 = errno;
            }
          if ((result2 = pthread_mutex_unlock(&(cv->mtxUnblockLock))) == 0)
            {
              result2 = pthread_mutex_destroy(&(cv->mtxUnblockLock));
            }

          (void) free(cv);
        }
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
      if (*cond == PTHREAD_COND_INITIALIZER)
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

    return ((result != 0) ? result : ((result1 != 0) ? result1 : result2));

}

/*
 * Arguments for cond_wait_cleanup, since we can only pass a
 * single void * to it.
 */
typedef struct {
  pthread_mutex_t * mutexPtr;
  pthread_cond_t cv;
  int * resultPtr;
  int signaled;
} ptw32_cond_wait_cleanup_args_t;

static void
ptw32_cond_wait_cleanup(void * args)
{
  ptw32_cond_wait_cleanup_args_t * cleanup_args = (ptw32_cond_wait_cleanup_args_t *) args;
  pthread_cond_t cv = cleanup_args->cv;
  int * resultPtr = cleanup_args->resultPtr;
  int nSignalsWasLeft;
  int nWaitersWasGone = 0; /* Initialised to quell warnings. */
  int result;

  /*
   * Whether we got here as a result of signal/broadcast or because of 
   * timeout on wait or thread cancellation we indicate that we are no 
   * longer waiting. The waiter is responsible for adjusting waiters 
   * (to)unblock(ed) counts (protected by unblock lock).
   */
  if ((result = pthread_mutex_lock(&(cv->mtxUnblockLock))) != 0)
    {
      *resultPtr = result;
      return;
    }

  if ( 0 != (nSignalsWasLeft = cv->nWaitersToUnblock) )
    {
      if ( !cleanup_args->signaled )
        {
          if ( 0 != cv->nWaitersBlocked )
            {
              (cv->nWaitersBlocked)--;
            }
          else
            {
              (cv->nWaitersGone)++;
            }
        }
      if ( 0 == --(cv->nWaitersToUnblock) )
        {
          if ( 0 != cv->nWaitersBlocked )
            {
              if (sem_post( &(cv->semBlockLock) ) != 0)
                {
                  *resultPtr = errno;
                  /*
                   * This is a fatal error for this CV,
                   * so we deliberately don't unlock
                   * cv->mtxUnblockLock before returning.
                   */
                  return;
                }
              nSignalsWasLeft = 0;
            }
          else if ( 0 != (nWaitersWasGone = cv->nWaitersGone) )
            {
              cv->nWaitersGone = 0;
            }
        }
    }
  else if ( INT_MAX/2 == ++(cv->nWaitersGone) )
    {
      if (sem_wait( &(cv->semBlockLock) ) != 0)
        {
          *resultPtr = errno;
          /*
           * This is a fatal error for this CV,
           * so we deliberately don't unlock
           * cv->mtxUnblockLock before returning.
           */
          return;
        }
      cv->nWaitersBlocked -= cv->nWaitersGone;
      if (sem_post( &(cv->semBlockLock) ) != 0)
        {
          *resultPtr = errno;
          /*
           * This is a fatal error for this CV,
           * so we deliberately don't unlock
           * cv->mtxUnblockLock before returning.
           */
          return;
        }
      cv->nWaitersGone = 0;
    }

  if ((result = pthread_mutex_unlock(&(cv->mtxUnblockLock))) != 0) 
    {
      *resultPtr = result;
      return;
    }

  if ( 1 == nSignalsWasLeft )
    {
      if ( 0 != nWaitersWasGone )
        {
          // sem_adjust( &(cv->semBlockQueue), -nWaitersWasGone );
          while ( nWaitersWasGone-- ) 
            {
              if (sem_wait( &(cv->semBlockQueue)) != 0 )
                {
                  *resultPtr = errno;
                  return;
                }
            }
        }
      if (sem_post(&(cv->semBlockLock)) != 0)
        {
          *resultPtr = errno;
          return;
        }
    }

  /*
   * XSH: Upon successful return, the mutex has been locked and is owned 
   * by the calling thread
   */
  if ((result = pthread_mutex_lock(cleanup_args->mutexPtr)) != 0)
    {
      *resultPtr = result;
    }

}                               /* ptw32_cond_wait_cleanup */

static INLINE int
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
  if (*cond == PTHREAD_COND_INITIALIZER)
    {
      result = ptw32_cond_check_need_init(cond);
    }

  if (result != 0 && result != EBUSY)
    {
      return result;
    }

  cv = *cond;

  if (sem_wait(&(cv->semBlockLock)) != 0)
    {
      return errno;
    }

  cv->nWaitersBlocked++;

  if (sem_post(&(cv->semBlockLock)) != 0)
    {
      return errno;
    }

  /*
   * Setup this waiter cleanup handler
   */
  cleanup_args.mutexPtr = mutex;
  cleanup_args.cv = cv;
  cleanup_args.resultPtr = &result;
  /*
   * If we're canceled, or the cancelable wait fails for any reason,
   * including a timeout, then tell the cleanup routine that we
   * have not been signaled.
   */
  cleanup_args.signaled = 0;

  pthread_cleanup_push(ptw32_cond_wait_cleanup, (void *) &cleanup_args);

  /*
   * Now we can release 'mutex' and...
   */
  if ((result = pthread_mutex_unlock(mutex)) == 0)
    {

      /*
       * ...wait to be awakened by
       *              pthread_cond_signal, or
       *              pthread_cond_broadcast, or
       *              timeout, or
       *              thread cancellation
       *
       * Note: 
       *
       *      ptw32_sem_timedwait is a cancellation point,
       *      hence providing the mechanism for making 
       *      pthread_cond_wait a cancellation point. 
       *      We use the cleanup mechanism to ensure we
       *      re-lock the mutex and adjust (to)unblock(ed) waiters 
       *      counts if we are cancelled, timed out or signalled.
       */
      if (ptw32_sem_timedwait(&(cv->semBlockQueue), abstime) != 0)
        {
          result = errno;
        }
    }

  /*
   * Not executed if we're canceled. Signaled is false if we timed out.
   */
  cleanup_args.signaled = (result == 0);

  /*
   * Always cleanup
   */
  pthread_cleanup_pop(1);

  /*
   * "result" can be modified by the cleanup handler.
   */
  return result;

}                               /* ptw32_cond_timedwait */


static INLINE int
ptw32_cond_unblock (pthread_cond_t * cond, 
                    int unblockAll)
     /*
      * Notes.
      *
      * Does not use the external mutex for synchronisation,
      * therefore semBlockLock is needed.
      * mtxUnblockLock is for LEVEL-2 synch. LEVEL-2 is the
      * state where the external mutex is not necessarily locked by
      * any thread, ie. between cond_wait unlocking and re-acquiring
      * the lock after having been signaled or a timeout or
      * cancellation.
      *
      * Uses the following CV elements:
      *   nWaitersBlocked
      *   nWaitersToUnblock
      *   nWaitersGone
      *   mtxUnblockLock
      *   semBlockLock
      *   semBlockQueue
      */
{
  int result;
  pthread_cond_t cv;
  int nSignalsToIssue;

  if (cond == NULL || *cond == NULL)
    {
      return EINVAL;
    }

  cv = *cond;

  /*
   * No-op if the CV is static and hasn't been initialised yet.
   * Assuming that any race condition is harmless.
   */
  if (cv == PTHREAD_COND_INITIALIZER)
    {
      return 0;
    }

  if ((result = pthread_mutex_lock(&(cv->mtxUnblockLock))) != 0)
    {
      return result;
    }

  if ( 0 != cv->nWaitersToUnblock )
    {
      if ( 0 == cv->nWaitersBlocked )
        {
          return pthread_mutex_unlock( &(cv->mtxUnblockLock) );
        }
      if (unblockAll)
        {
          cv->nWaitersToUnblock += (nSignalsToIssue = cv->nWaitersBlocked);
          cv->nWaitersBlocked = 0;
        }
      else
        {
          nSignalsToIssue = 1;
          cv->nWaitersToUnblock++;
          cv->nWaitersBlocked--;
        }
    }
  else if ( cv->nWaitersBlocked > cv->nWaitersGone ) 
    {
      if (sem_wait( &(cv->semBlockLock) ) != 0)
        {
          result = errno;
          (void) pthread_mutex_unlock( &(cv->mtxUnblockLock) );
          return result;
        }
      if ( 0 != cv->nWaitersGone )
        {
          cv->nWaitersBlocked -= cv->nWaitersGone;
          cv->nWaitersGone = 0;
        }
      if (unblockAll)
        {
          nSignalsToIssue = cv->nWaitersToUnblock = cv->nWaitersBlocked;
          cv->nWaitersBlocked = 0;
        }
      else
        {
          nSignalsToIssue = cv->nWaitersToUnblock = 1;
          cv->nWaitersBlocked--;
        }
    }
  else
    {
      return pthread_mutex_unlock( &(cv->mtxUnblockLock) );
    }

  if ((result = pthread_mutex_unlock( &(cv->mtxUnblockLock) )) == 0)
    {
      if (sem_post_multiple( &(cv->semBlockQueue), nSignalsToIssue ) != 0)
        {
          result = errno;
        }
    }

  return result;

}                               /* ptw32_cond_unblock */

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
      *
      *      1)      The function must be called with 'mutex' LOCKED
      *              by the calling thread, or undefined behaviour
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
  /*
   * The NULL abstime arg means INFINITE waiting.
   */
  return (ptw32_cond_timedwait(cond, mutex, NULL));

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
      *              by the calling thread, or undefined behaviour
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
  if (abstime == NULL)
    {
      return EINVAL;
    }

  return (ptw32_cond_timedwait(cond, mutex, abstime));

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
      *
      *      1)      Use when any waiter can respond and only one need
      *              respond (all waiters being equal).
      *
      * RESULTS
      *              0               successfully signaled condition,
      *              EINVAL          'cond' is invalid,
      *
      * ------------------------------------------------------
      */
{
  /*
   * The '0'(FALSE) unblockAll arg means unblock ONE waiter.
   */
  return (ptw32_cond_unblock(cond, 0));

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
      *
      *      1)      Use when more than one waiter may respond to
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
  /*
   * The '1'(TRUE) unblockAll arg means unblock ALL waiters.
   */
  return (ptw32_cond_unblock(cond, 1));

}                               /* pthread_cond_broadcast */
