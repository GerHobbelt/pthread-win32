/*
 * rwlock.c
 *
 * Description:
 * This translation unit implements read/write lock primitives.
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

#include <errno.h>
#include <limits.h>

#include "pthread.h"
#include "implement.h"

static INLINE int
ptw32_rwlock_check_need_init(pthread_rwlock_t *rwlock)
{
  int result = 0;

  /*
   * The following guarded test is specifically for statically
   * initialised rwlocks (via PTHREAD_RWLOCK_INITIALIZER).
   *
   * Note that by not providing this synchronisation we risk
   * introducing race conditions into applications which are
   * correctly written.
   *
   * Approach
   * --------
   * We know that static rwlocks will not be PROCESS_SHARED
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
  EnterCriticalSection(&ptw32_rwlock_test_init_lock);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section
   * and only initialise if the rwlock is valid (not been destroyed).
   * If a static rwlock has been destroyed, the application can
   * re-initialise it only by calling pthread_rwlock_init()
   * explicitly.
   */
  if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
    {
      result = pthread_rwlock_init(rwlock, NULL);
    }
  else if (*rwlock == NULL)
    {
      /*
       * The rwlock has been destroyed while we were waiting to
       * initialise it, so the operation that caused the
       * auto-initialisation should fail.
       */
      result = EINVAL;
    }

  LeaveCriticalSection(&ptw32_rwlock_test_init_lock);

  return result;
}

int
pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
    int result;
    pthread_rwlock_t rwl = 0;

    if (rwlock == NULL)
      {
        return EINVAL;
      }

    if (attr != NULL && *attr != NULL)
      {
        result = EINVAL; /* Not supported */
        goto DONE;
      }

    rwl = (pthread_rwlock_t) calloc(1, sizeof(*rwl));

    if (rwl == NULL)
      {
        result = ENOMEM;
        goto DONE;
      }

    rwl->nSharedAccessCount = 0;
    rwl->nExclusiveAccessCount = 0;
    rwl->nCompletedSharedAccessCount = 0;

    result = pthread_mutex_init(&rwl->mtxExclusiveAccess, NULL);
    if (result != 0)
      {
        goto FAIL0;
      }

    result = pthread_mutex_init(&rwl->mtxSharedAccessCompleted, NULL);
    if (result != 0)
      {
        goto FAIL1;
      }

    result = pthread_cond_init(&rwl->cndSharedAccessCompleted, NULL);
    if (result != 0)
      {
        goto FAIL2;
      }

    rwl->nMagic = PTW32_RWLOCK_MAGIC;

    result = 0;
    goto DONE;

FAIL2:
    (void) pthread_mutex_destroy(&(rwl->mtxSharedAccessCompleted));

FAIL1:
    (void) pthread_mutex_destroy(&(rwl->mtxExclusiveAccess));

FAIL0:
    (void) free(rwl);
    rwl = NULL;

DONE:
    *rwlock = rwl;

    return result;
}

int
pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    pthread_rwlock_t rwl;
    int result = 0, result1 = 0, result2 = 0;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    if (*rwlock != PTHREAD_RWLOCK_INITIALIZER)
      {
        rwl = *rwlock;

        if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
          {
            return EINVAL;
          }

        if ((result = pthread_mutex_lock(&(rwl->mtxExclusiveAccess))) != 0)
          {
            return result;
          }

        if ((result = pthread_mutex_lock(&(rwl->mtxSharedAccessCompleted))) != 0)
          {
            (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
            return result;
          }

        /*
         * Check whether any threads own/wait for the lock (wait for ex.access);
         * report "BUSY" if so.
         */
        if (rwl->nExclusiveAccessCount > 0
            || rwl->nSharedAccessCount > rwl->nCompletedSharedAccessCount)
          {
            result = pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted));
            result1 = pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
            result2 = EBUSY;
          }
        else 
          {
            rwl->nMagic = 0;

            if ((result = pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted))) != 0)
              {
                pthread_mutex_unlock(&rwl->mtxExclusiveAccess);
                return result;
              }

            if ((result = pthread_mutex_unlock(&(rwl->mtxExclusiveAccess))) != 0)
              {
                return result;
              }

            *rwlock = NULL; /* Invalidate rwlock before anything else */
            result = pthread_cond_destroy(&(rwl->cndSharedAccessCompleted));
            result1 = pthread_mutex_destroy(&(rwl->mtxSharedAccessCompleted));
            result2 = pthread_mutex_destroy(&(rwl->mtxExclusiveAccess));
            (void) free(rwl);
          }
      }
    else
      {
        /*
         * See notes in ptw32_rwlock_check_need_init() above also.
         */
        EnterCriticalSection(&ptw32_rwlock_test_init_lock);

        /*
         * Check again.
         */
        if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
          {
            /*
             * This is all we need to do to destroy a statically
             * initialised rwlock that has not yet been used (initialised).
             * If we get to here, another thread
             * waiting to initialise this rwlock will get an EINVAL.
             */
            *rwlock = NULL;
          }
        else
          {
            /*
             * The rwlock has been initialised while we were waiting
             * so assume it's in use.
             */
            result = EBUSY;
          }

        LeaveCriticalSection(&ptw32_rwlock_test_init_lock);
      }

    return ((result != 0) ? result : ((result1 != 0) ? result1 : result2));
}

int
pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int result;
    pthread_rwlock_t rwl;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return result;
          }
      }

    rwl = *rwlock;

    if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
      {
        return EINVAL;
      }

    if ((result = pthread_mutex_lock(&(rwl->mtxExclusiveAccess))) != 0)
      {
        return result;
      }

    if (++rwl->nSharedAccessCount == INT_MAX)
      {
        if ((result = pthread_mutex_lock(&(rwl->mtxSharedAccessCompleted))) != 0)
          {
            (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
            return result;
          }

        rwl->nSharedAccessCount -= rwl->nCompletedSharedAccessCount;
        rwl->nCompletedSharedAccessCount = 0;

        if ((result = pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted))) != 0)
          {
            (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
            return result;
          }
      }

    return (pthread_mutex_unlock(&(rwl->mtxExclusiveAccess)));
}

static void
ptw32_rwlock_cancelwrwait(void * arg)
{
    pthread_rwlock_t rwl = (pthread_rwlock_t) arg;

    rwl->nSharedAccessCount = -rwl->nCompletedSharedAccessCount;
    rwl->nCompletedSharedAccessCount = 0;

    (void) pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted));
    (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
}

int
pthread_rwlock_wrlock(pthread_rwlock_t * rwlock)
{
    int result;
    pthread_rwlock_t rwl;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return result;
          }
      }

    rwl = *rwlock;

    if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
      {
        return EINVAL;
      }

    if ((result = pthread_mutex_lock(&(rwl->mtxExclusiveAccess))) != 0)
      {
        return result;
      }

    if ((result = pthread_mutex_lock(&(rwl->mtxSharedAccessCompleted))) != 0)
      {
        (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
        return result;
      }

    if (rwl->nExclusiveAccessCount == 0) 
      {
        if (rwl->nCompletedSharedAccessCount > 0) 
          {
            rwl->nSharedAccessCount -= rwl->nCompletedSharedAccessCount;
            rwl->nCompletedSharedAccessCount = 0;
          }

        if (rwl->nSharedAccessCount > 0) 
          {
            /*
             * pthread_rwlock_wrlock() is not a cancelation point
             * so temporarily prevent pthread_cond_wait() from being one.
             */
            pthread_t self = pthread_self();
            int oldCancelState;

            rwl->nCompletedSharedAccessCount = -rwl->nSharedAccessCount;

            if (self->cancelType == PTHREAD_CANCEL_DEFERRED)
              {
                 pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
              }

            /* Could still be PTHREAD_CANCEL_ASYNCHRONOUS. */
            pthread_cleanup_push(ptw32_rwlock_cancelwrwait, (void*)rwl);

            do 
              {
                result = pthread_cond_wait(&(rwl->cndSharedAccessCompleted), 
                                           &(rwl->mtxSharedAccessCompleted));
              }
            while (result == 0 && rwl->nCompletedSharedAccessCount < 0);

            if (self->cancelType == PTHREAD_CANCEL_DEFERRED)
              {
                pthread_setcancelstate(oldCancelState, NULL);
              }

            pthread_cleanup_pop ((result != 0) ? 1 : 0);

            if (result == 0)
              {
                rwl->nSharedAccessCount = 0;
              }
          }
      }

    if (result == 0)
      {
        rwl->nExclusiveAccessCount++;
      }

    return result;
}

int
pthread_rwlock_unlock(pthread_rwlock_t * rwlock)
{
    int result, result1;
    pthread_rwlock_t rwl;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
      {
        /*
         * Assume any race condition here is harmless.
         */
        return 0;
      }

    rwl = *rwlock;

    if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
      {
        return EINVAL;
      }

    if (rwl->nExclusiveAccessCount == 0) 
      {
        if ((result = pthread_mutex_lock(&(rwl->mtxSharedAccessCompleted))) != 0)
          {
            return result;
          }

        if (++rwl->nCompletedSharedAccessCount == 0)
          {
            result = pthread_cond_signal(&(rwl->cndSharedAccessCompleted));
          }

        result1 = pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted));
      }
    else 
      {
        rwl->nExclusiveAccessCount--;

        result = pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted));
        result1 = pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));

      }
 
    return ((result != 0) ? result : result1);
}

int
pthread_rwlock_tryrdlock(pthread_rwlock_t * rwlock)
{
    int result;
    pthread_rwlock_t rwl;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return result;
          }
      }

    rwl = *rwlock;

    if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
      {
        return EINVAL;
      }

    if ((result = pthread_mutex_trylock(&(rwl->mtxExclusiveAccess))) != 0)
      {
        return result;
      }

    if (++rwl->nSharedAccessCount == INT_MAX) 
      {
        if ((result = pthread_mutex_lock(&(rwl->mtxSharedAccessCompleted))) != 0)
          {
            (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
            return result;
          }

        rwl->nSharedAccessCount -= rwl->nCompletedSharedAccessCount;
        rwl->nCompletedSharedAccessCount = 0;

        if ((result = pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted))) != 0)
          {
            (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
            return result;
          }
      }

    return (pthread_mutex_unlock(&rwl->mtxExclusiveAccess));
}

int
pthread_rwlock_trywrlock(pthread_rwlock_t * rwlock)
{
    int result, result1;
    pthread_rwlock_t rwl;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == PTHREAD_RWLOCK_INITIALIZER)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return result;
          }
      }

    rwl = *rwlock;

    if (rwl->nMagic != PTW32_RWLOCK_MAGIC)
      {
        return EINVAL;
      }

    if ((result = pthread_mutex_trylock(&(rwl->mtxExclusiveAccess))) != 0)
      {
        return result;
      }

    if ((result = pthread_mutex_trylock(&(rwl->mtxSharedAccessCompleted))) != 0)
      {
        result1 = pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
        return ((result1 != 0) ? result1 : result);
      }

    if (rwl->nExclusiveAccessCount == 0) 
      {
        if (rwl->nCompletedSharedAccessCount > 0) 
          {
            rwl->nSharedAccessCount -= rwl->nCompletedSharedAccessCount;
            rwl->nCompletedSharedAccessCount = 0;
          }

        if (rwl->nSharedAccessCount > 0) 
          {
            if ((result = pthread_mutex_unlock(&(rwl->mtxSharedAccessCompleted))) != 0)
              {
                (void) pthread_mutex_unlock(&(rwl->mtxExclusiveAccess));
                return result;
              }

            if ((result = pthread_mutex_unlock(&(rwl->mtxExclusiveAccess))) == 0)
              {
                result = EBUSY;
              }
          }
        else
          {
            rwl->nExclusiveAccessCount = 1;
          }
      }
    else 
      {
        result = EBUSY;
      }

    return result;
}
