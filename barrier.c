/*
 * barrier.c
 *
 * Description:
 * This translation unit implements barrier primitives.
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
  pthread_barrier_t b;

  if (barrier == NULL || count == 0)
    {
      return EINVAL;
    }

  if (NULL != (b = (pthread_barrier_t) calloc(1, sizeof(*b))))
    {
      b->pshared = (attr != NULL && *attr != NULL
                    ? (*attr)->pshared
                    : PTHREAD_PROCESS_PRIVATE);

      b->nCurrentBarrierHeight = b->nInitialBarrierHeight = count;
      b->iStep = 0;

      /*
       * Two semaphores are used in the same way as two stepping
       * stones might be used in crossing a stream. Once all
       * threads are safely on one stone, the other stone can
       * be moved ahead, and the threads can start moving to it.
       * If some threads decide to eat their lunch before moving
       * then the other threads have to wait.
       */
      if (0 == sem_init(&(b->semBarrierBreeched[0]), b->pshared, 0))
        {
          if (0 == sem_init(&(b->semBarrierBreeched[1]), b->pshared, 0))
            {
              *barrier = b;
              return 0;
            }
          (void) sem_destroy(&(b->semBarrierBreeched[0]));
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
  *barrier = NULL;

  if (0 == (result = sem_destroy(&(b->semBarrierBreeched[0]))))
    {
      if (0 == (result = sem_destroy(&(b->semBarrierBreeched[1]))))
        {
          (void) free(b);
          return 0;
        }
      (void) sem_init(&(b->semBarrierBreeched[0]),
                        b->pshared,
                        0);
    }

  *barrier = b;
  return(result);
}


int
pthread_barrier_wait(pthread_barrier_t *barrier)
{
  int result;
  int step;
  pthread_barrier_t b;

  if (barrier == NULL || *barrier == (pthread_barrier_t) PTW32_OBJECT_INVALID)
    {
      return EINVAL;
    }

  b = *barrier;
  step = b->iStep;

  if (0 == InterlockedDecrement((long *) &(b->nCurrentBarrierHeight)))
    {
      /* Must be done before posting the semaphore. */
      b->nCurrentBarrierHeight = b->nInitialBarrierHeight;

      /*
       * There is no race condition between the semaphore wait and post
       * because we are using two alternating semas and all threads have
       * entered barrier_wait and checked nCurrentBarrierHeight before this
       * barrier's sema can be posted. Any threads that have not quite
       * entered sem_wait below when the multiple_post has completed
       * will nevertheless continue through the semaphore (barrier)
       * and will not be left stranded.
       */
      result = (b->nInitialBarrierHeight > 1
                ? sem_post_multiple(&(b->semBarrierBreeched[step]),
                                    b->nInitialBarrierHeight - 1)
                : 0);
    }
  else
    {
      BOOL switchCancelState;
      int oldCancelState;
      pthread_t self = pthread_self();

      /*
       * This routine is not a cancelation point, so temporarily
       * prevent sem_wait() from being one.
       * PTHREAD_CANCEL_ASYNCHRONOUS threads can still be canceled.
       */
      switchCancelState = (self->cancelType == PTHREAD_CANCEL_DEFERRED &&
                           0 == pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,
                                                       &oldCancelState));

      result = sem_wait(&(b->semBarrierBreeched[step]));

      if (switchCancelState)
        {
          (void) pthread_setcancelstate(oldCancelState, NULL);
        }
    }

  /*
   * The first thread across will be the PTHREAD_BARRIER_SERIAL_THREAD.
   * It also sets up the alternate semaphore as the next barrier.
   */
  if (0 == result)
    {
      result = ((_LONG) step ==
                InterlockedCompareExchange((_LPLONG) &(b->iStep),
                                           (_LONG) (1L - step),
                                           (_LONG) step)
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
