/*
 * barrier.c
 *
 * Description:
 * This translation unit implements spin locks primitives.
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


#ifdef __MINGW32__
#define _LONG long
#define _LPLONG long*
#else
#define _LONG PVOID
#define _LPLONG PVOID*
#endif

int
pthread_barrier_init(pthread_barrier_t * barrier,
                     const pthread_barrierattr_t * attr,
                     unsigned int count)
{
  int pshared = PTHREAD_PROCESS_PRIVATE;
  pthread_barrier_t b;
  pthread_mutexattr_t ma;

  if (barrier == NULL || count == 0)
    {
      return EINVAL;
    }

  if (NULL != (b = (pthread_barrier_t) calloc(1, sizeof(*b))))
    {
      if (attr != NULL && *attr != NULL)
        {
          pshared = (*attr)->pshared;
        }

      b->nCurrentBarrierHeight = b->nInitialBarrierHeight = count;
      b->iStep = 0;
	b->nSerial = 0;

      if (0 == pthread_mutexattr_init(&ma) &&
          0 == pthread_mutexattr_setpshared(&ma, pshared))
        {
          if (0 == pthread_mutex_init(&(b->mtxExclusiveAccess), &ma))
            {
              pthread_mutexattr_destroy(&ma);
              if (0 == sem_init(&(b->semBarrierBreeched[0]), pshared, 0))
                {
                  if (0 == sem_init(&(b->semBarrierBreeched[1]), pshared, 0))
                    {
                      *barrier = b;
                      return 0;
                    }
                  (void) sem_destroy(&(b->semBarrierBreeched[0]));
                }
              (void) pthread_mutex_destroy(&(b->mtxExclusiveAccess));
            }
          pthread_mutexattr_destroy(&ma);
        }
      (void) free(b);
    }

  return ENOMEM;
}

int
pthread_barrier_destroy(pthread_barrier_t *barrier)
{
  int result = 0;
  pthread_barrier_t b;

  if (barrier == NULL || *barrier == (pthread_barrier_t) PTW32_OBJECT_INVALID)
    {
      return EINVAL;
    }

  b = *barrier;
  
  result = pthread_mutex_trylock(&(b->mtxExclusiveAccess));

  if (0 == result)
    {
      /*
       * FIXME!!!
       * The mutex isn't held by another thread but we could still
       * be too late invalidating the barrier below since another thread
       * may alredy have entered barrier_wait and the check for a valid
       * *barrier != NULL.
       */
      *barrier = NULL;
      (void) pthread_mutex_unlock(&(b->mtxExclusiveAccess));

      (void) sem_destroy(&(b->semBarrierBreeched[1]));
      (void) sem_destroy(&(b->semBarrierBreeched[0]));
      (void) pthread_mutex_destroy(&(b->mtxExclusiveAccess));
      (void) free(b);
    }

  return(result);
}

int
pthread_barrier_wait(pthread_barrier_t *barrier)
{
  int result;
  pthread_barrier_t b;

  if (barrier == NULL || *barrier == (pthread_barrier_t) PTW32_OBJECT_INVALID)
    {
      return EINVAL;
    }

  b = *barrier;

  result = pthread_mutex_lock(&(b->mtxExclusiveAccess));

  if (0 == result)
    {
      int step = b->iStep;
      unsigned int serial = b->nSerial;

      if (0 == --(b->nCurrentBarrierHeight))
        {
          (void) pthread_mutex_unlock(&(b->mtxExclusiveAccess));
          /*
           * All other threads in this barrier set have at least passed
           * through the decrement and test above, so its safe to
           * raise the barrier to its full height.
           */
          b->iStep = 1 - step;
          b->nCurrentBarrierHeight = b->nInitialBarrierHeight;
          (void) sem_post_multiple(&(b->semBarrierBreeched[step]), b->nInitialBarrierHeight - 1);
        }
      else
        {
          pthread_t self;
          int oldCancelState;

          (void) pthread_mutex_unlock(&(b->mtxExclusiveAccess));

          self = pthread_self();
 
          /*
           * pthread_barrier_wait() is not a cancelation point
           * so temporarily prevent pthreadCancelableWait() from being one.
           */
          if (self->cancelType == PTHREAD_CANCEL_DEFERRED)
            {
              pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
            }

          /*
           * There is no race condition between the semaphore wait and post
           * because we are using two alternating semas and all threads have
           * entered barrier_wait and checked nCurrentBarrierHeight before this
           * barrier's sema can be posted. Any threads that have not quite
           * entered sem_wait below when the multiple_post above is called
           * will nevertheless continue through the sema (and the barrier).
           * We have fulfilled our synchronisation function.
           */
          result = sem_wait(&(b->semBarrierBreeched[step]));

          if (self->cancelType == PTHREAD_CANCEL_DEFERRED)
            {
              pthread_setcancelstate(oldCancelState, NULL);
            }
        }

      /*
       * The first thread to return from the wait will be the
       * PTHREAD_BARRIER_SERIAL_THREAD.
       */
      result = ((_LONG) serial == InterlockedCompareExchange((_LPLONG) &(b->nSerial),
                                                             (_LONG) ((serial + 1)
                                                                      & 0x7FFFFFFF),
                                                             (_LONG) serial)
                ? PTHREAD_BARRIER_SERIAL_THREAD
                : 0);
    }

  return(result);
}


int
pthread_barrierattr_init (pthread_barrierattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Initializes a barrier attributes object with default
      *      attributes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_barrierattr_t
      *
      *
      * DESCRIPTION
      *      Initializes a barrier attributes object with default
      *      attributes.
      *
      *      NOTES:
      *              1)      Used to define barrier types
      *
      * RESULTS
      *              0               successfully initialized attr,
      *              ENOMEM          insufficient memory for attr.
      *
      * ------------------------------------------------------
      */
{
  pthread_barrierattr_t ba;
  int result = 0;
 
  ba = (pthread_barrierattr_t) calloc (1, sizeof (*ba));
 
  if (ba == NULL)
    {
      result = ENOMEM;
    }
 
  ba->pshared = PTHREAD_PROCESS_PRIVATE;
 
  *attr = ba;
 
  return (result);
 
}                               /* pthread_barrierattr_init */
 
 
int
pthread_barrierattr_destroy (pthread_barrierattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Destroys a barrier attributes object. The object can
      *      no longer be used.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_barrierattr_t
      *
      *
      * DESCRIPTION
      *      Destroys a barrier attributes object. The object can
      *      no longer be used.
      *
      *      NOTES:
      *              1)      Does not affect barrieres created using 'attr'
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
      pthread_barrierattr_t ba = *attr;
 
      *attr = NULL;
      free (ba);
 
      result = 0;
    }
 
  return (result);
 
}                               /* pthread_barrierattr_destroy */


int
pthread_barrierattr_getpshared (const pthread_barrierattr_t * attr,
                                int *pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Determine whether barriers created with 'attr' can be
      *      shared between processes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_barrierattr_t
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
      *      Mutexes creatd with 'attr' can be shared between
      *      processes if pthread_barrier_t variable is allocated
      *      in memory shared by these processes.
      *      NOTES:
      *              1)      pshared barriers MUST be allocated in shared
      *                      memory.
      *              2)      The following macro is defined if shared barriers
      *                      are supported:
      *                              _POSIX_THREAD_PROCESS_SHARED
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
 
}                               /* pthread_barrierattr_getpshared */


int
pthread_barrierattr_setpshared (pthread_barrierattr_t * attr,
                                int pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Barriers created with 'attr' can be shared between
      *      processes if pthread_barrier_t variable is allocated
      *      in memory shared by these processes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_barrierattr_t
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
      *      processes if pthread_barrier_t variable is allocated
      *      in memory shared by these processes.
      *
      *      NOTES:
      *              1)      pshared barriers MUST be allocated in shared
      *                      memory.
      *
      *              2)      The following macro is defined if shared barriers
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
 
}                               /* pthread_barrierattr_setpshared */
