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


/*
 * The following internal versions of *CriticalSection()
 * include an implementation of TryEnterCriticalSection
 * for platforms on which that function has not been
 * provided by Microsoft (eg. W95/98). This allows us
 * to use critical sections exclusively as the basis
 * of our implementation of POSIX mutex locks.
 *
 * Where TryEnterCriticalSection() is provided by the
 * platform, these routines act as wrappers with
 * minimal additional overhead. Otherwise, these
 * routines manage additional state in order to
 * properly emulate TryEnterCriticalSection().
 *
 * In any case, using of critical sections exclusively
 * should still be much faster than using Win32 mutex
 * locks as our POSIX mutex locks.
 */

static void
ptw32_InitializeCriticalSection (ptw32_cs_t * csect)
     /*
      * ------------------------------------------------------
      *
      * PARAMETERS
      *      csect
      *              pointer to an instance of ptw32_cs_t
      *
      * DESCRIPTION
      *      Internal implementation of InitializeCriticalSection.
      * 
      * ------------------------------------------------------
      */
{
  ptw32_cs_t cs = *csect;

  cs->owner = NULL;
  cs->lock_idx = -1;
  cs->entered_count = 0;
  InitializeCriticalSection(&cs->cs);
  cs->valid = 1;
}

static void
ptw32_DeleteCriticalSection (ptw32_cs_t * csect)
     /*
      * ------------------------------------------------------
      *
      * PARAMETERS
      *      csect
      *              pointer to an instance of ptw32_cs_t
      *
      * DESCRIPTION
      *      Internal implementation of DeleteCriticalSection.
      * 
      * ------------------------------------------------------
      */
{
  ptw32_cs_t cs = *csect;

  cs->valid = 0;
  DeleteCriticalSection(&cs->cs);
}

static void
ptw32_EnterCriticalSection(ptw32_cs_t * csect)
     /*
      * ------------------------------------------------------
      *
      * PARAMETERS
      *      csect
      *              pointer to an instance of ptw32_cs_t
      *
      * DESCRIPTION
      *      Internal implementation of EnterCriticalSection.
      * 
      * ------------------------------------------------------
      */
{
  ptw32_cs_t cs = *csect;

  if (!cs->valid)
    {
      return;
    }

  if (NULL != ptw32_try_enter_critical_section)
    {
      EnterCriticalSection(&cs->cs);
    }
  else
    {
      while (InterlockedIncrement(&cs->lock_idx) > 0)
        {
          InterlockedDecrement(&cs->lock_idx);
          Sleep(0);
        }
      EnterCriticalSection(&cs->cs);
      cs->entered_count++;
      cs->owner = pthread_self();
      InterlockedDecrement(&cs->lock_idx);
    }
}

static void
ptw32_LeaveCriticalSection (ptw32_cs_t * csect)
     /*
      * ------------------------------------------------------
      *
      * PARAMETERS
      *      csect
      *              pointer to an instance of ptw32_cs_t
      *
      * DESCRIPTION
      *      Internal implementation of LeaveCriticalSection.
      * 
      * ------------------------------------------------------
      */
{
  ptw32_cs_t cs = *csect;

  if (!cs->valid)
    {
      return;
    }

  if (NULL != ptw32_try_enter_critical_section)
    {
      LeaveCriticalSection(&cs->cs);
    }
  else
    {
      while (InterlockedIncrement(&cs->lock_idx) > 0)
        {
          InterlockedDecrement(&cs->lock_idx);
          Sleep(0);
        }

      LeaveCriticalSection(&cs->cs);

      cs->entered_count--;

      if (cs->entered_count == 0)
        {
          cs->owner = NULL;
        }

      InterlockedDecrement(&cs->lock_idx);
    }
}

static BOOL
ptw32_TryEnterCriticalSection (ptw32_cs_t * csect)
     /*
      * ------------------------------------------------------
      *
      * PARAMETERS
      *      csect
      *              pointer to an instance of ptw32_cs_t
      *
      * DESCRIPTION
      *      Internal implementation of TryEnterCriticalSection.
      * 
      * RETURNS
      *      FALSE              Current thread doesn't own the
      *                         lock,
      *      TRUE               Current thread owns the lock
      *                         (if the current thread already
      *                          held the lock then we recursively
      *                          enter).
      * ------------------------------------------------------
      */
{
  ptw32_cs_t cs = *csect;
  BOOL result = FALSE;

  if (!cs->valid)
    {
      return (FALSE);
    }

  if (NULL != ptw32_try_enter_critical_section)
    {
      result = (*ptw32_try_enter_critical_section)(&cs->cs);
    }
  else
    {
      pthread_t self = pthread_self();

      while (InterlockedIncrement(&cs->lock_idx) > 0)
        {
          InterlockedDecrement(&cs->lock_idx);
          Sleep(0);
        }

      if (cs->owner == NULL || pthread_equal(cs->owner, self))
        {
          /* The semantics of TryEnterCriticalSection
           * (according to the documentation at MS)
           * are that the CS is entered recursively
           * if the thread is the current owner.
           */
          EnterCriticalSection(&cs->cs);
          cs->entered_count++;
          cs->owner = self;
          result = TRUE;
        }

      InterlockedDecrement(&cs->lock_idx);
    }

  return (result);
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

  ptw32_InitializeCriticalSection(&mx->cs);
  mx->lockCount = 0;
  mx->ownerThread = NULL;

  if (attr != NULL && *attr != NULL)
    {

      mx->type = (*attr)->type;

      if ((*attr)->pshared == PTHREAD_PROCESS_SHARED)
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

#else

          result = ENOSYS;

#endif /* _POSIX_THREAD_PROCESS_SHARED */
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

  if (mutex == NULL || *mutex == NULL)
    {
      return EINVAL;
    }

  /*
   * Check to see if we have something to delete.
   */
  if (*mutex != (pthread_mutex_t) PTW32_OBJECT_AUTO_INIT)
    {
      mx = *mutex;

      result = pthread_mutex_trylock(&mx);

      if (result == 0 || result == EDEADLK)
        {
          /*
           * FIXME!!!
           * The mutex isn't held by another thread but we could still
           * be too late invalidating the mutex below. Yet we can't do it
           * earlier in case another thread needs to unlock the mutex
           * that it's holding.
           */
          *mutex = NULL;

          result = pthread_mutex_unlock(&mx);

          if (result == 0)
            {
              ptw32_DeleteCriticalSection(&mx->cs);
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

  ma->pshared = PTHREAD_PROCESS_PRIVATE;
  ma->type = PTHREAD_MUTEX_DEFAULT;

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
pthread_mutexattr_settype (pthread_mutexattr_t * attr,
                           int type)
     /*
      * ------------------------------------------------------
      *
      * PARAMETERS
      *      attr
      *              pointer to an instance of pthread_mutexattr_t
      *
      *      type
      *              must be one of:
      *
      *                      PTHREAD_MUTEX_DEFAULT
      *
      *                      PTHREAD_MUTEX_NORMAL
      *
      *                      PTHREAD_MUTEX_ERRORCHECK
      *
      *                      PTHREAD_MUTEX_RECURSIVE
      *
      * DESCRIPTION
      * The pthread_mutexattr_gettype() and
      * pthread_mutexattr_settype() functions  respectively get and
      * set the mutex type  attribute. This attribute is set in  the
      * type  parameter to these functions. The default value of the
      * type  attribute is  PTHREAD_MUTEX_DEFAULT.
      * 
      * The type of mutex is contained in the type  attribute of the
      * mutex attributes. Valid mutex types include:
      *
      * PTHREAD_MUTEX_NORMAL
      *          This type of mutex does  not  detect  deadlock.  A
      *          thread  attempting  to  relock  this mutex without
      *          first unlocking it will  deadlock.  Attempting  to
      *          unlock  a  mutex  locked  by  a  different  thread
      *          results  in  undefined  behavior.  Attempting   to
      *          unlock  an  unlocked  mutex  results  in undefined
      *          behavior.
      * 
      * PTHREAD_MUTEX_ERRORCHECK
      *          This type of  mutex  provides  error  checking.  A
      *          thread  attempting  to  relock  this mutex without
      *          first unlocking it will return with  an  error.  A
      *          thread  attempting to unlock a mutex which another
      *          thread has locked will return  with  an  error.  A
      *          thread attempting to unlock an unlocked mutex will
      *          return with an error.
      * 
      * PTHREAD_MUTEX_RECURSIVE
      *          A thread attempting to relock this  mutex  without
      *          first  unlocking  it  will  succeed in locking the
      *          mutex. The relocking deadlock which can occur with
      *          mutexes of type  PTHREAD_MUTEX_NORMAL cannot occur
      *          with this type of mutex. Multiple  locks  of  this
      *          mutex  require  the  same  number  of  unlocks  to
      *          release  the  mutex  before  another  thread   can
      *          acquire the mutex. A thread attempting to unlock a
      *          mutex which another thread has locked will  return
      *          with  an  error. A thread attempting to  unlock an
      *          unlocked mutex will return  with  an  error.  This
      *          type  of mutex is only supported for mutexes whose
      *          process        shared         attribute         is
      *          PTHREAD_PROCESS_PRIVATE.
      *
      * RESULTS
      *              0               successfully set attribute,
      *              EINVAL          'attr' or 'type' is invalid,
      *
      * ------------------------------------------------------
      */
{
  int result = 0;

  if ((attr != NULL && *attr != NULL))
    {
      switch (type)
        {
        case PTHREAD_MUTEX_DEFAULT:
        case PTHREAD_MUTEX_NORMAL:
        case PTHREAD_MUTEX_ERRORCHECK:
        case PTHREAD_MUTEX_RECURSIVE:
          (*attr)->type = type;
          break;
        default:
          result = EINVAL;
          break;
        }
    }
  else
    {
      result = EINVAL;
    }

  return (result);

}                               /* pthread_mutexattr_settype */


int
pthread_mutexattr_gettype (pthread_mutexattr_t * attr,
                           int type)
{
  int result = 0;

  if ((attr != NULL && *attr != NULL))
    {
      result = (*attr)->type;
    }
  else
    {
      result = EINVAL;
    }

  return (result);
}


int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  int result = 0;
  pthread_mutex_t mx;
  pthread_t self;


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
  self = pthread_self();

  ptw32_EnterCriticalSection(&mx->cs);

  switch (mx->type)
    {
    case PTHREAD_MUTEX_NORMAL:
      if (pthread_equal(mx->ownerThread, self))
        {
          ptw32_LeaveCriticalSection(&mx->cs);
          /*
           * Pretend to be deadlocked but release the
           * mutex if we are canceled.
           */
          pthread_cleanup_push(pthread_mutex_unlock, (void *) mutex);
          while (TRUE)
            {
              Sleep(0);
            }
          pthread_cleanup_pop(1);
        }
      break;
    case PTHREAD_MUTEX_DEFAULT:
    case PTHREAD_MUTEX_ERRORCHECK:
      if (pthread_equal(mx->ownerThread, self))
        {
          ptw32_LeaveCriticalSection(&mx->cs);
          result = EDEADLK;
        }
      break;
    case PTHREAD_MUTEX_RECURSIVE:
      /*
       * Nothing more to do.
       */
      break;
    default:
      result = EINVAL;
      break;
    }

  if (result == 0)
    {
      mx->ownerThread = self;
      mx->lockCount++;
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
      if (pthread_equal(mx->ownerThread, pthread_self()))
	{
          mx->lockCount--;

	  if (mx->lockCount == 0)
	    {
	      mx->ownerThread = NULL;
	    }
	      
          ptw32_LeaveCriticalSection(&mx->cs);
	}
      else
	{
	  result = EPERM;
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
  pthread_t self;

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
  self = pthread_self();

  if (result == 0)
    {
      /*
       * TryEnterCriticalSection is a little different to
       * the POSIX trylock semantics. Trylock returns
       * EBUSY even if the calling thread already owns
       * the mutex - it doesn't lock it recursively, even
       * if the mutex type is PTHREAD_MUTEX_RECURSIVE.
       */
      if (ptw32_TryEnterCriticalSection(&mx->cs))
        {
          /*
           * We now own the lock, but check that we don't
           * already own the mutex.
           */
          if (pthread_equal(mx->ownerThread, self))
            {
              ptw32_LeaveCriticalSection(&mx->cs);
              result = EBUSY;
            }
        }
      else
        {
          result = EBUSY;
        }
    }

  if (result == 0)
    {
      mx->ownerThread = self;
      mx->lockCount++;
    }

  return (result);
}

