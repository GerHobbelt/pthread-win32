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

#include "pthread.h"
#include "implement.h"

static int
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
  if (*rwlock == (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
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

  return(result);
}

int
pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL)
      {
        return EINVAL;
      }

    rw = (pthread_rwlock_t) calloc(1, sizeof(*rw));

    if (rw == NULL)
      {
        result = ENOMEM;
        goto FAIL0;
      }

    if (attr != NULL
        && *attr != NULL)
      {
        result = EINVAL; /* Not supported */
        goto FAIL0;
      }

    if ((result = pthread_mutex_init(&(rw->rw_lock), NULL)) != 0)
      {
        goto FAIL1;
      }

    if ((result = pthread_cond_init(&(rw->rw_condreaders), NULL)) != 0)
      {
        goto FAIL2;
      }

    if ((result = pthread_cond_init(&(rw->rw_condwriters), NULL)) != 0)
      {
        goto FAIL3;
      }

    rw->rw_nwaitreaders = 0;
    rw->rw_nwaitwriters = 0;
    rw->rw_refcount = 0;
    rw->rw_magic = RW_MAGIC;

    result = 0;
    goto FAIL0;

FAIL3:
    (void) pthread_cond_destroy(&(rw->rw_condreaders));

FAIL2:
    (void) pthread_mutex_destroy(&(rw->rw_lock));

FAIL1:
FAIL0:
    *rwlock = rw;

    return(result);
}

int
pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    pthread_rwlock_t rw;
    int result = 0;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    if (*rwlock != (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
      {
        rw = *rwlock;

        if (pthread_mutex_lock(&(rw->rw_lock)) != 0)
          {
            return(EINVAL);
          }

        if (rw->rw_magic != RW_MAGIC)
          {
            (void) pthread_mutex_unlock(&(rw->rw_lock));
            return(EINVAL);
          }

        if (rw->rw_refcount != 0
            || rw->rw_nwaitreaders != 0
            || rw->rw_nwaitwriters != 0)
          {
            (void) pthread_mutex_unlock(&(rw->rw_lock));
            result = EBUSY;
          }
        else
          {
            /*
             * Need to NULL this before we start freeing up
             * and destroying it's components.
             */
            *rwlock = NULL;
            rw->rw_magic = 0;

            (void) pthread_mutex_unlock(&(rw->rw_lock));

            (void) pthread_cond_destroy(&(rw->rw_condreaders));
            (void) pthread_cond_destroy(&(rw->rw_condwriters));
            (void) pthread_mutex_destroy(&(rw->rw_lock));
            free(rw);
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
        if (*rwlock == (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
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

    return(result);
}

static void
_rwlock_cancelrdwait(void * arg)
{
    pthread_rwlock_t rw;

    rw = (pthread_rwlock_t) arg;
    rw->rw_nwaitreaders--;
    pthread_mutex_unlock(&(rw->rw_lock));
}

int
pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return(result);
          }
      }

    rw = *rwlock;

    if ((result = pthread_mutex_lock(&(rw->rw_lock))) != 0)
      {
        return(result);
      }

    if (rw->rw_magic != RW_MAGIC)
      {
        result = EINVAL;
        goto FAIL1;
      }

    /*
     * Give preference to waiting writers
     */
    while (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0)
      {
        rw->rw_nwaitreaders++;
        pthread_cleanup_push(_rwlock_cancelrdwait, rw);
        result = pthread_cond_wait(&(rw->rw_condreaders), &(rw->rw_lock));
        pthread_cleanup_pop(0);
        rw->rw_nwaitreaders--;

        if (result != 0)
          {
            break;
          }
      }

    if (result == 0)
      {
        rw->rw_refcount++; /* another reader has a read lock */
      }

FAIL1:
    (void) pthread_mutex_unlock(&(rw->rw_lock));

    return(result);
}

static void
_rwlock_cancelwrwait(void * arg)
{
    pthread_rwlock_t rw;

    rw = (pthread_rwlock_t) arg;
    rw->rw_nwaitwriters--;
    (void) pthread_mutex_unlock(&(rw->rw_lock));
}

int
pthread_rwlock_wrlock(pthread_rwlock_t * rwlock)
{
    int result;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return(result);
          }
      }

    rw = *rwlock;

    if (rw->rw_magic != RW_MAGIC)
      {
        return(EINVAL);
      }

    if ((result = pthread_mutex_lock(&(rw->rw_lock))) != 0)
      {
        return(result);
      }

    while (rw->rw_refcount != 0)
      {
        rw->rw_nwaitwriters++;
        pthread_cleanup_push(_rwlock_cancelwrwait, rw);
        result = pthread_cond_wait(&(rw->rw_condwriters), &(rw->rw_lock));
        pthread_cleanup_pop(0);
        rw->rw_nwaitwriters--;

        if (result != 0)
          {
            break;
          }
      }

    if (result == 0)
      {
        rw->rw_refcount = -1;
      }

    (void) pthread_mutex_unlock(&(rw->rw_lock));

    return(result);
}

int
pthread_rwlock_unlock(pthread_rwlock_t * rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    if (*rwlock == (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
      {
        /*
         * Assume any race condition here is harmless.
         */
        return(0);
      }

    rw = *rwlock;

    if ((result = pthread_mutex_lock(&(rw->rw_lock))) != 0)
      {
        return(result);
      }

    if (rw->rw_magic != RW_MAGIC)
      {
        result = EINVAL;
        goto FAIL1;
      }

    if (rw->rw_refcount > 0)
      {
        rw->rw_refcount--;             /* releasing a reader */
      }
    else if (rw->rw_refcount == -1)
      {
        rw->rw_refcount = 0;           /* releasing a writer */
      }
    else 
      {
        return(EINVAL);
      }

    result = 0;

    /*
     * Give preference to waiting writers over waiting readers
     */
    if (rw->rw_nwaitwriters > 0)
      {
        if (rw->rw_refcount == 0)
          {
            result = pthread_cond_signal(&(rw->rw_condwriters));
          }
      }
    else if (rw->rw_nwaitreaders > 0)
      {
         result = pthread_cond_broadcast(&(rw->rw_condreaders));
      }

FAIL1:
    (void) pthread_mutex_unlock(&(rw->rw_lock));

    return(result);
}
 
int
pthread_rwlock_tryrdlock(pthread_rwlock_t * rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return(result);
          }
      }

    rw = *rwlock;

    if ((result = pthread_mutex_lock(&(rw->rw_lock))) != 0)
      {
        return(result);
      }

    if (rw->rw_magic != RW_MAGIC)
      {
        result = EINVAL;
        goto FAIL1;
      }

    if (rw->rw_refcount == -1 || rw->rw_nwaitwriters > 0)
      {
        result = EBUSY;    /* held by a writer or waiting writers */
      }
    else
      {
        rw->rw_refcount++; /* increment count of reader locks */
      }

FAIL1:
    (void) pthread_mutex_unlock(&(rw->rw_lock));

    return(result);
}

int
pthread_rwlock_trywrlock(pthread_rwlock_t * rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of ptw32_rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) PTW32_OBJECT_AUTO_INIT)
      {
        result = ptw32_rwlock_check_need_init(rwlock);

        if (result != 0 && result != EBUSY)
          {
            return(result);
          }
      }

    rw = *rwlock;

    if ((result = pthread_mutex_lock(&(rw->rw_lock))) != 0)
      {
        return(result);
      }

    if (rw->rw_magic != RW_MAGIC)
      {
        result = EINVAL;
        goto FAIL1;
      }

    if (rw->rw_refcount != 0)
      {
        result = EBUSY;       /* held by either writer or reader(s) */
      }
    else
      {
        rw->rw_refcount = -1; /* available, indicate a writer has it */
      }

FAIL1:
    (void) pthread_mutex_unlock(&(rw->rw_lock));

    return(result);
}
