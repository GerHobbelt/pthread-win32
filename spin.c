/*
 * spin.c
 *
 * Description:
 * This translation unit implements spin lock primitives.
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

static INLINE int
ptw32_spinlock_check_need_init(pthread_spinlock_t *lock)
{
  int result = 0;

  /*
   * The following guarded test is specifically for statically
   * initialised spinlocks (via PTHREAD_SPINLOCK_INITIALIZER).
   *
   * Note that by not providing this synchronisation we risk
   * introducing race conditions into applications which are
   * correctly written.
   */
  EnterCriticalSection(&ptw32_spinlock_test_init_lock);

  /*
   * We got here possibly under race
   * conditions. Check again inside the critical section
   * and only initialise if the spinlock is valid (not been destroyed).
   * If a static spinlock has been destroyed, the application can
   * re-initialise it only by calling pthread_spin_init()
   * explicitly.
   */
  if (*lock == PTHREAD_SPINLOCK_INITIALIZER)
    {
      result = pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
    }
  else if (*lock == NULL)
    {
      /*
       * The spinlock has been destroyed while we were waiting to
       * initialise it, so the operation that caused the
       * auto-initialisation should fail.
       */
      result = EINVAL;
    }

  LeaveCriticalSection(&ptw32_spinlock_test_init_lock);

  return(result);
}


int
pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
  pthread_spinlock_t s;
  int result = 0;

  if (lock == NULL)
    {
      return EINVAL;
    }

  s = (pthread_spinlock_t) calloc(1, sizeof(*s));

  if (s == NULL)
    {
      return ENOMEM;
    }

  if (0 != pthread_getprocessors_np(&(s->u.cpus)))
    {
      s->u.cpus = 1;
    }

  if (s->u.cpus > 1)
    {
      if (pshared == PTHREAD_PROCESS_SHARED)
        {
          /*
           * Creating spinlock that can be shared between
           * processes.
           */
#if _POSIX_THREAD_PROCESS_SHARED

          /*
           * Not implemented yet.
           */

#error ERROR [__FILE__, line __LINE__]: Process shared spin locks are not supported yet.


#else

          result = ENOSYS;
          goto FAIL0;

#endif /* _POSIX_THREAD_PROCESS_SHARED */

        }

      s->interlock = PTW32_SPIN_UNLOCKED;
    }
  else
    {
      pthread_mutexattr_t ma;
      result = pthread_mutexattr_init(&ma);

      if (0 == result)
        {
          ma->pshared = pshared;
          result = pthread_mutex_init(&(s->u.mutex), &ma);
          if (0 == result)
            {
              s->interlock = PTW32_SPIN_USE_MUTEX;
            }
        }
    }

FAIL0:
  *lock = (0 == result ? s : NULL);
  return(result);
}

int
pthread_spin_destroy(pthread_spinlock_t *lock)
{
  register pthread_spinlock_t s;

  if (lock == NULL || *lock == NULL)
    {
      return EINVAL;
    }

  if ((s = *lock) != PTHREAD_SPINLOCK_INITIALIZER)
    {
      if (s->interlock == PTW32_SPIN_USE_MUTEX)
        {
          return pthread_mutex_destroy(&(s->u.mutex));
        }

      if ( (_LONG) PTW32_SPIN_UNLOCKED ==
           InterlockedCompareExchange((_LPLONG) &(s->interlock),
                                      (_LONG) PTW32_OBJECT_INVALID,
                                      (_LONG) PTW32_SPIN_UNLOCKED))
        {
          return 0;
        }

      return EINVAL;
    }
  else
    {
      int result = 0;

      /*
       * See notes in ptw32_spinlock_check_need_init() above also.
       */
      EnterCriticalSection(&ptw32_spinlock_test_init_lock);

      /*
       * Check again.
       */
      if (*lock == PTHREAD_SPINLOCK_INITIALIZER)
        {
          /*
           * This is all we need to do to destroy a statically
           * initialised spinlock that has not yet been used (initialised).
           * If we get to here, another thread
           * waiting to initialise this mutex will get an EINVAL.
           */
          *lock = NULL;
        }
      else
        {
          /*
           * The spinlock has been initialised while we were waiting
           * so assume it's in use.
           */
          result = EBUSY;
        }

      LeaveCriticalSection(&ptw32_spinlock_test_init_lock);
      return(result);
    }
}

/*
 * NOTE: For speed, these routines don't check if "lock" is valid.
 */
int
pthread_spin_lock(pthread_spinlock_t *lock)
{
  register pthread_spinlock_t s = *lock;

  if (s == PTHREAD_SPINLOCK_INITIALIZER)
    {
      int result;

      if ((result = ptw32_spinlock_check_need_init(lock)) != 0)
        {
          return(result);
        }
    }

  while ( (_LONG) PTW32_SPIN_LOCKED ==
          InterlockedCompareExchange((_LPLONG) &(s->interlock),
                                     (_LONG) PTW32_SPIN_LOCKED,
                                     (_LONG) PTW32_SPIN_UNLOCKED) )
    {}

  if (s->interlock == PTW32_SPIN_LOCKED)
    {
      return 0;
    }
  else if (s->interlock == PTW32_SPIN_USE_MUTEX)
    {
      return pthread_mutex_lock(&(s->u.mutex));
    }

  return EINVAL;
}

int
pthread_spin_unlock(pthread_spinlock_t *lock)
{
  register pthread_spinlock_t s = *lock;

  if (s->interlock == PTW32_SPIN_USE_MUTEX)
    {
      return pthread_mutex_unlock(&(s->u.mutex));
    }

  if ((_LONG) PTW32_SPIN_LOCKED ==
      InterlockedCompareExchange((_LPLONG) &(s->interlock),
                                 (_LONG) PTW32_SPIN_UNLOCKED,
                                 (_LONG) PTW32_SPIN_LOCKED ) )
    {
      return 0;
    }

  return EINVAL;
}

int
pthread_spin_trylock(pthread_spinlock_t *lock)
{
  pthread_spinlock_t s = *lock;

  if (s == PTHREAD_SPINLOCK_INITIALIZER)
    {
      int result;

      if ((result = ptw32_spinlock_check_need_init(lock)) != 0)
        {
          return(result);
        }
    }

  if ((_LONG) PTW32_SPIN_UNLOCKED ==
      InterlockedCompareExchange((_LPLONG) &(s->interlock),
                                 (_LONG) PTW32_SPIN_LOCKED,
                                 (_LONG) PTW32_SPIN_UNLOCKED ) )
    {
      return 0;
    }

  if (s->interlock == PTW32_SPIN_USE_MUTEX)
    {
      return pthread_mutex_trylock(&(s->u.mutex));
    }

  return EINVAL;
}
