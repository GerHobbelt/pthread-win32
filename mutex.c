/*
 * mutex.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
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

/* errno.h or a replacement file is included by pthread.h */
//#include <errno.h>

#include "pthread.h"
#include "implement.h"


static int
ptw32_mutex_check_need_init(pthread_mutex_t *mutex)
{
  int result = 0;

  /*
   * The following guarded test is specifically for statically
   * initialised mutexes (via PTHREAD_MUTEX_INITIALIZER).
   *
   * Note that by not providing this synchronisation we risk
   * introducing race conditions into applications which are
   * correctly written.
   *
   * Approach
   * --------
   * We know that static mutexes will not be PROCESS_SHARED
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
  EnterCriticalSection(&ptw32_mutex_test_init_lock);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section
   * and only initialise if the mutex is valid (not been destroyed).
   * If a static mutex has been destroyed, the application can
   * re-initialise it only by calling pthread_mutex_init()
   * explicitly.
   */
  if (*mutex == (pthread_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      result = pthread_mutex_init(mutex, NULL);
    }
  else if (*mutex == NULL)
    {
      /*
       * The mutex has been destroyed while we were waiting to
       * initialise it, so the operation that caused the
       * auto-initialisation should fail.
       */
      result = EINVAL;
    }

  LeaveCriticalSection(&ptw32_mutex_test_init_lock);

  return(result);
}

int
pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL)
    {
      return EINVAL;
    }

  mx = (pthread_mutex_t) calloc(1, sizeof(*mx));

  if (mx == NULL)
    {
      result = ENOMEM;
      goto FAIL0;
    }

  mx->mutex = 0;

  if (attr != NULL
      && *attr != NULL
      && (*attr)->pshared == PTHREAD_PROCESS_SHARED
      )
    {
      /*
       * Creating mutex that can be shared between
       * processes.
       */
#if _POSIX_THREAD_PROCESS_SHARED

      /*
       * Not implemented yet.
       */

#error ERROR [__FILE__, line __LINE__]: Process shared mutexes are not supported yet.

      mx->mutex = CreateMutex(NULL, FALSE, "FIXME FIXME FIXME");

      if (mx->mutex == 0)
	{
	  result = EAGAIN;
	}

#else

      result = ENOSYS;

#endif /* _POSIX_THREAD_PROCESS_SHARED */
    }
  else
    {
      if (ptw32_try_enter_critical_section != NULL
	  || (attr != NULL
	      && *attr != NULL
	      && (*attr)->forcecs == 1)
	  )
	{
	  /* 
	   * Create a critical section. 
	   */
	  InitializeCriticalSection(&mx->cs);

	  /*
	   * Check that it works ok - since InitializeCriticalSection doesn't
	   * return success or failure.
	   */
	  if (TryEnterCriticalSection(&mx->cs))
	    {
	      LeaveCriticalSection(&mx->cs);
	    }
	  else
	    {
	      DeleteCriticalSection(&mx->cs);
	      result = EAGAIN;
	    }
	}
      else
	{
	  /*
	   * Create a mutex that can only be used within the
	   * current process
	   */
	  mx->mutex = CreateMutex (NULL,
				   FALSE,
				   NULL);

	  if (mx->mutex == 0)
	    {
	      result = EAGAIN;
	    }
	}
    }

  if (result != 0 && mx != NULL)
    {
      free(mx);
      mx = NULL;
    }

FAIL0:
  *mutex = mx;

  return(result);
}

int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL
      || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * Check to see if we have something to delete.
   */
  if (*mutex != (pthread_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      mx = *mutex;

      if ((result = pthread_mutex_trylock(&mx)) == 0)
        {
          /*
           * FIXME!!!
           * The mutex isn't held by another thread but we could still
           * be too late invalidating the mutex below. Yet we can't do it
           * earlier in case another thread needs to unlock the mutex
           * that it's holding.
           */
          *mutex = NULL;

          pthread_mutex_unlock(&mx);

          if (mx->mutex == 0)
            {
              DeleteCriticalSection(&mx->cs);
            }
          else
            {
              result = (CloseHandle (mx->mutex) ? 0 : EINVAL);
            }

          if (result == 0)
            {
              mx->mutex = 0;
              free(mx);
            }
          else
            {
              /*
               * Restore the mutex before we return the error.
               */
              *mutex = mx;
            }
        }
    }
  else
    {
      /*
       * See notes in ptw32_mutex_check_need_init() above also.
       */
      EnterCriticalSection(&ptw32_mutex_test_init_lock);

      /*
       * Check again.
       */
      if (*mutex == (pthread_mutex_t) PTW32_OBJECT_AUTO_INIT)
        {
          /*
           * This is all we need to do to destroy a statically
           * initialised mutex that has not yet been used (initialised).
           * If we get to here, another thread
           * waiting to initialise this mutex will get an EINVAL.
           */
          *mutex = NULL;
        }
      else
        {
          /*
           * The mutex has been initialised while we were waiting
           * so assume it's in use.
           */
          result = EBUSY;
        }

      LeaveCriticalSection(&ptw32_mutex_test_init_lock);
    }

  return(result);
}

int
pthread_mutexattr_init (pthread_mutexattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Initializes a mutex attributes object with default
      *      attributes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_mutexattr_t
      *
      *
      * DESCRIPTION
      *      Initializes a mutex attributes object with default
      *      attributes.
      *
      *      NOTES:
      *              1)      Used to define mutex types
      *
      * RESULTS
      *              0               successfully initialized attr,
      *              ENOMEM          insufficient memory for attr.
      *
      * ------------------------------------------------------
      */
{
  pthread_mutexattr_t ma;
  int result = 0;

  ma = (pthread_mutexattr_t) calloc (1, sizeof (*ma));

  if (ma == NULL)
    {
      result = ENOMEM;
    }

  *attr = ma;

  return (result);

}                               /* pthread_mutexattr_init */


int
pthread_mutexattr_destroy (pthread_mutexattr_t * attr)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Destroys a mutex attributes object. The object can
      *      no longer be used.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_mutexattr_t
      *
      *
      * DESCRIPTION
      *      Destroys a mutex attributes object. The object can
      *      no longer be used.
      *
      *      NOTES:
      *              1)      Does not affect mutexes created using 'attr'
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
      pthread_mutexattr_t ma = *attr;

      *attr = NULL;
      free (ma);

      result = 0;
    }

  return (result);

}                               /* pthread_mutexattr_destroy */


int
pthread_mutexattr_getpshared (const pthread_mutexattr_t * attr,
			      int *pshared)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      Determine whether mutexes created with 'attr' can be
      *      shared between processes.
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_mutexattr_t
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
      *      processes if pthread_mutex_t variable is allocated
      *      in memory shared by these processes.
      *      NOTES:
      *              1)      pshared mutexes MUST be allocated in shared
      *                      memory.
      *              2)      The following macro is defined if shared mutexes
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

}                               /* pthread_mutexattr_getpshared */


int
pthread_mutexattr_setpshared (pthread_mutexattr_t * attr,
			      int pshared)
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

}                               /* pthread_mutexattr_setpshared */


int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static mutex. We check
   * again inside the guarded section of ptw32_mutex_check_need_init()
   * to avoid race conditions.
   */
  if (*mutex == (pthread_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      result = ptw32_mutex_check_need_init(mutex);
    }

  mx = *mutex;

  if (result == 0)
    {
      if (mx->mutex == 0)
	{
	  EnterCriticalSection(&mx->cs);
	}
      else
	{
	  result = (WaitForSingleObject(mx->mutex, INFINITE) 
		    == WAIT_OBJECT_0)
	    ? 0
	    : EINVAL;
	}
    }

  return(result);
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  mx = *mutex;

  /* 
   * If the thread calling us holds the mutex then there is no
   * race condition. If another thread holds the
   * lock then we shouldn't be in here.
   */
  if (mx != (pthread_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      if (mx->mutex == 0)
	{
	  LeaveCriticalSection(&mx->cs);
	}
      else
	{
	  result = (ReleaseMutex (mx->mutex) ? 0 : EINVAL);
	}
    }
  else
    {
      result = EINVAL;
    }

  return(result);
}

int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * We do a quick check to see if we need to do more work
   * to initialise a static mutex. We check
   * again inside the guarded section of ptw32_mutex_check_need_init()
   * to avoid race conditions.
   */
  if (*mutex == (pthread_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      result = ptw32_mutex_check_need_init(mutex);
    }

  mx = *mutex;

  if (result == 0)
    {
      if (mx->mutex == 0)
	{
	  if ((*ptw32_try_enter_critical_section)(&mx->cs) != TRUE)
	    {
	      result = EBUSY;
	    }
	}
      else
	{
	  DWORD status;

	  status = WaitForSingleObject (mx->mutex, 0);

	  if (status != WAIT_OBJECT_0)
	    {
	      result = ((status == WAIT_TIMEOUT)
			? EBUSY
			: EINVAL);
	    }
	}
    }

  return(result);
}
