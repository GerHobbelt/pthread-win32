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
_rwlock_check_need_init(pthread_rwlock_t *rwlock)
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
  EnterCriticalSection(&_pthread_rwlock_test_init_lock);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section
   * and only initialise if the mutex is valid (not been destroyed).
   * If a static mutex has been destroyed, the application can
   * re-initialise it only by calling pthread_mutex_init()
   * explicitly.
   */
  if (*rwlock == (pthread_rwlock_t) _PTHREAD_OBJECT_AUTO_INIT)
    {
      result = pthread_rwlock_init(rwlock, NULL);
    }

  LeaveCriticalSection(&_pthread_rwlock_test_init_lock);

  return(result);
}

int pthread_rwlock_init(pthread_rwlock_t *rwlock, const pthread_rwlockattr_t *attr)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL)
      {
        return EINVAL;
      }

    rw = *rwlock;

    rw = (pthread_rwlock_t) calloc(1, sizeof(*rw));

    if (rw == NULL)
      {
        result = ENOMEM;
        goto fail0;
      }

    if (attr != NULL
        && *attr != NULL)
      {
        result = EINVAL; /* Not supported */
        goto fail0;
      }

    if ((result = pthread_mutex_init(&rw->rw_mutex, NULL)) != 0)
      {
        goto fail1;
      }

    if ((result = pthread_cond_init(&rw->rw_condreaders, NULL)) != 0)
      {
        goto fail2;
      }

    if ((result = pthread_cond_init(&rw->rw_condwriters, NULL)) != 0)
      {
        goto fail3;
      }

    rw->rw_nwaitreaders = 0;
    rw->rw_nwaitwriters = 0;
    rw->rw_refcount = 0;
    rw->rw_magic = RW_MAGIC;

    result = 0;
    goto fail0;

fail3:
    pthread_cond_destroy(&rw->rw_condreaders);

fail2:
    pthread_mutex_destroy(&rw->rw_mutex);

fail1:
fail0:
    *rwlock = rw;

    return(result); /* an errno value */
}

int pthread_rwlock_destroy(pthread_rwlock_t *rwlock)
{
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL || (*rwlock)->rw_magic != RW_MAGIC)
      {
        return(EINVAL);
      }

    if (*rwlock == (pthread_rwlock_t) _PTHREAD_OBJECT_AUTO_INIT)
      {
        /*
         * Destroy a static declared R/W lock that has never been
         * initialised.
         */
        *rwlock = NULL;
        return(0);
      }

    rw = *rwlock;

    if (rw->rw_refcount != 0
        || rw->rw_nwaitreaders != 0
        || rw->rw_nwaitwriters != 0)
      {
        return(EBUSY);
      }

    pthread_mutex_destroy(&rw->rw_mutex);
    pthread_cond_destroy(&rw->rw_condreaders);
    pthread_cond_destroy(&rw->rw_condwriters);
    rw->rw_magic = 0;
    free(rw);
    *rwlock = NULL;

    return(0);
}

static void _rwlock_cancelrdwait(void *arg)
{
    pthread_rwlock_t rw;

    rw = arg;
    rw->rw_nwaitreaders--;
    pthread_mutex_unlock(&rw->rw_mutex);
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of _rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) _PTHREAD_OBJECT_AUTO_INIT)
      {
        result = _rwlock_check_need_init(rwlock);
      }

    rw = *rwlock;

    if (rw->rw_magic != RW_MAGIC)
      {
        return(EINVAL);
      }

    if ((result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
      {
        return(result);
      }

    /* give preference to waiting writers */
    while (rw->rw_refcount < 0 || rw->rw_nwaitwriters > 0)
      {
        rw->rw_nwaitreaders++;
        pthread_cleanup_push(_rwlock_cancelrdwait, rw);
        result = pthread_cond_wait(&rw->rw_condreaders, &rw->rw_mutex);
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

    pthread_mutex_unlock(&rw->rw_mutex);

    return(result);
}

static void _rwlock_cancelwrwait(void *arg)
{
    pthread_rwlock_t rw;

    rw = arg;
    rw->rw_nwaitwriters--;
    pthread_mutex_unlock(&rw->rw_mutex);
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwlock)
{
    int result;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of _rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) _PTHREAD_OBJECT_AUTO_INIT)
      {
        result = _rwlock_check_need_init(rwlock);
      }

    rw = *rwlock;

    if (rw->rw_magic != RW_MAGIC)
        return(EINVAL);

    if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
        return(result);

    while (rw->rw_refcount != 0) {
        rw->rw_nwaitwriters++;
        pthread_cleanup_push(_rwlock_cancelwrwait, rw);
        result = pthread_cond_wait(&rw->rw_condwriters, &rw->rw_mutex);
        pthread_cleanup_pop(0);
        rw->rw_nwaitwriters--;
        if (result != 0)
            break;
    }
    if (result == 0)
        rw->rw_refcount = -1;

    pthread_mutex_unlock(&rw->rw_mutex);
    return(result);
}

int pthread_rwlock_unlock(pthread_rwlock_t *rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return(EINVAL);
      }

    if (*rwlock == (pthread_rwlock_t) _PTHREAD_OBJECT_AUTO_INIT)
      {
        return(0);
      }

    rw = *rwlock;

    if (rw->rw_magic != RW_MAGIC)
      {
        return(EINVAL);
      }

    if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
      {
        return(result);
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
            result = pthread_cond_signal(&rw->rw_condwriters);
          }
      }
    else if (rw->rw_nwaitreaders > 0)
      {
         result = pthread_cond_broadcast(&rw->rw_condreaders);
      }

    pthread_mutex_unlock(&rw->rw_mutex);
    return(result);
}
 
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of _rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) _PTHREAD_OBJECT_AUTO_INIT)
      {
        result = _rwlock_check_need_init(rwlock);
      }

    rw = *rwlock;

    if (rw->rw_magic != RW_MAGIC)
      {
        return(EINVAL);
      }

    if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
      {
        return(result);
      }

    if (rw->rw_refcount == -1 || rw->rw_nwaitwriters > 0)
      {
        result = EBUSY;    /* held by a writer or waiting writers */
      }
    else
      {
        rw->rw_refcount++; /* increment count of reader locks */
      }

    pthread_mutex_unlock(&rw->rw_mutex);
    return(result);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwlock)
{
    int result = 0;
    pthread_rwlock_t rw;

    if (rwlock == NULL || *rwlock == NULL)
      {
        return EINVAL;
      }

    /*
     * We do a quick check to see if we need to do more work
     * to initialise a static rwlock. We check
     * again inside the guarded section of _rwlock_check_need_init()
     * to avoid race conditions.
     */
    if (*rwlock == (pthread_rwlock_t) _PTHREAD_OBJECT_AUTO_INIT)
      {
        result = _rwlock_check_need_init(rwlock);
      }

    rw = *rwlock;

    if (rw->rw_magic != RW_MAGIC)
      {
        return(EINVAL);
      }

    if ( (result = pthread_mutex_lock(&rw->rw_mutex)) != 0)
      {
        return(result);
      }

    if (rw->rw_refcount != 0)
      {
        result = EBUSY;       /* held by either writer or reader(s) */
      }
    else
      {
        rw->rw_refcount = -1; /* available, indicate a writer has it */
      }

    pthread_mutex_unlock(&rw->rw_mutex);
    return(result);
}
