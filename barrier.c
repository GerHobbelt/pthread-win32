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


int
pthread_barrier_init(pthread_barrier_t * barrier,
                     const pthread_barrierattr_t * attr,
                     int count)
{
  int result = 0;
  int pshared = PTHREAD_PROCESS_PRIVATE;
  pthread_barrier_t b;

  if (barrier == NULL)
    {
      return EINVAL;
    }

  b = (pthread_barrier_t) calloc(1, sizeof(*b));

  if (b == NULL)
    {
      result = ENOMEM;
      goto FAIL0;
    }

  if (attr != NULL && *attr != NULL)
    {
      pshared = (*attr)->pshared;
    }

  b->nCurrentBarrierHeight = b->nInitialBarrierHeight = count;

  result = pthread_mutex_init(&(b->mtxExclusiveAccess), NULL);
  if (0 != result)
    {
      goto FAIL1;
    }

  b->eventBarrierBreeched = CreateEvent(NULL,   /* Security attributes */
                                        TRUE,   /* Manual reset        */
                                        FALSE,  /* Initially signaled  */
                                        NULL);  /* Name                */

  if (NULL != b->eventBarrierBreeched)
    {
      goto DONE;
    }
  (void) pthread_mutex_destroy(&(b->mtxExclusiveAccess));
  result = ENOMEM;

 FAIL1:
  (void) free(b);
  b = NULL;

 FAIL0:
 DONE:
  *barrier = b;
  return(result);
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

      result = CloseHandle(b->eventBarrierBreeched);
      (void) pthread_mutex_unlock(&(b->mtxExclusiveAccess));
      if (result == TRUE)
        {
          (void) pthread_mutex_destroy(&(b->mtxExclusiveAccess));
          (void) free(b);
          result = 0;
        }
      else
        {
          *barrier = b;
          result = EINVAL;
        }
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
      if (0 == --(b->nCurrentBarrierHeight))
        {
          b->nCurrentBarrierHeight = b->nInitialBarrierHeight;
          (void) pthread_mutex_unlock(&(b->mtxExclusiveAccess));
          /*
           * This is a work-around for the FIXME below. We
           * give any threads that didn't quite get to register
           * their wait another quantum. This is temporary
           * - there is a better way to do this.
           */
          Sleep(0);
          (void) PulseEvent(b->eventBarrierBreeched);
          /*
           * Would be better if the first thread to return
           * from this routine got this value. On a single
           * processor machine that will be the last thread
           * to reach the barrier (us), most of the time.
           */
          result = PTHREAD_BARRIER_SERIAL_THREAD;
        }
      else
        {
          pthread_t self;
          int oldCancelState;

          self = pthread_self();
 
          /*
           * pthread_barrier_wait() is not a cancelation point
           * so temporarily prevent pthreadCancelableWait() from being one.
           */
          if (self->cancelType == PTHREAD_CANCEL_DEFERRED)
            {
              pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldCancelState);
            }

          (void) pthread_mutex_unlock(&(b->mtxExclusiveAccess));

          /* FIXME!!! It's possible for a thread to be left behind at a
           * barrier because of the time gap between the unlock
           * and the registration that the thread is waiting on the
           * event.
           */
          result = pthreadCancelableWait(b->eventBarrierBreeched);

          if (self->cancelType == PTHREAD_CANCEL_DEFERRED)
            {
              pthread_setcancelstate(oldCancelState, NULL);
            }
        }
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
