/*
 * spin.c
 *
 * Description:
 * This translation unit implements spin lock primitives.
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998 Ben Elliston and Ross Johnson
 * Copyright (C) 1999,2000,2001 Ross Johnson
 *
 * Contact Email: rpj@ise.canberra.edu.au
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
  int cpus = 0;
  int result = 0;

  if (lock == NULL)
    {
      return EINVAL;
    }

  if (0 != ptw32_getprocessors(&cpus))
    {
      cpus = 1;
    }

  if (cpus > 1)
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

          return ENOSYS;

#endif /* _POSIX_THREAD_PROCESS_SHARED */

        }
    }

  s = (pthread_spinlock_t) calloc(1, sizeof(*s));

  if (s == NULL)
    {
      return ENOMEM;
    }

  if (cpus > 1)
    {
      s->u.cpus = cpus;
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
      (void) pthread_mutexattr_destroy(&ma);
    }

  if (0 == result)
    {
      *lock = s;
    }
  else
    {
      (void) free(s);
      *lock = NULL;
    }

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

      if ( (PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED ==
           ptw32_interlocked_compare_exchange((PTW32_INTERLOCKED_LPLONG) &(s->interlock),
                                              (PTW32_INTERLOCKED_LONG) PTW32_OBJECT_INVALID,
                                              (PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED))
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
  register pthread_spinlock_t s;

  if (*lock == PTHREAD_SPINLOCK_INITIALIZER)
    {
      int result;

      if ((result = ptw32_spinlock_check_need_init(lock)) != 0)
        {
          return(result);
        }
    }

  s = *lock;

  while ( (PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED ==
          ptw32_interlocked_compare_exchange((PTW32_INTERLOCKED_LPLONG) &(s->interlock),
                                             (PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED,
                                             (PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED) )
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

  if (s == PTHREAD_SPINLOCK_INITIALIZER)
    {
      return EPERM;
    }

  switch ((long) ptw32_interlocked_compare_exchange((PTW32_INTERLOCKED_LPLONG) &(s->interlock),
                                                    (PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED,
                                                    (PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED ))
    {
      case PTW32_SPIN_LOCKED:    return 0;
      case PTW32_SPIN_UNLOCKED:  return EPERM;
      case PTW32_SPIN_USE_MUTEX: return pthread_mutex_unlock(&(s->u.mutex));
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

  switch ((long) ptw32_interlocked_compare_exchange((PTW32_INTERLOCKED_LPLONG) &(s->interlock),
                                                    (PTW32_INTERLOCKED_LONG) PTW32_SPIN_LOCKED,
                                                    (PTW32_INTERLOCKED_LONG) PTW32_SPIN_UNLOCKED ))
    {
      case PTW32_SPIN_UNLOCKED:  return 0;
      case PTW32_SPIN_LOCKED:    return EBUSY;
      case PTW32_SPIN_USE_MUTEX: return pthread_mutex_trylock(&(s->u.mutex));
    }

  return EINVAL;
}
