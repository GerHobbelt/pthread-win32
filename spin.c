/*
 * spin.c
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

/*
 * This works because the mask that is formed exposes all but the
 * first two LSBs. If the spinlock is using a mutex rather than
 * the interlock mechanism then there will always be high bits
 * to indicate this. This is all just to save the overhead of
 * using a non-simple struct for spinlocks.
 */
#define PTW32_SPIN_SPINS(_lock) \
  (0 == (_lock->u.mx & (pthread_mutex_t) ~(PTW32_OBJECT_INVALID | PTW32_SPIN_UNLOCK | PTW32_SPIN_LOCKED)))


int
pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
  int CPUs = 1;
  int result = 0;

  if (lock == NULL)
    {
      return EINVAL;
    }

  (void) pthread_getprocessors_np(&CPUs);

  if (CPUs > 1)
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

      lock->u.interlock = PTW32_SPIN_UNLOCKED;
    }
  else
    {
      pthread_mutexattr_t ma;
      result = pthread_mutexattr_init(&ma);

      if (0 == result)
        {
          ma->pshared = pshared;
          result = pthread_mutex_init(&(lock->u.mx), &ma);
        }
    }

FAIL0:

  return(result);
}

int
pthread_spin_destroy(pthread_spinlock_t *lock)
{
  if (lock == NULL)
    {
      return EINVAL;
    }

  if (PTW32_SPIN_SPINS(lock))
    {
      if ( PTW32_SPIN_UNLOCKED !=
           InterlockedCompareExchange((LPLONG) &(lock->u.interlock),
                                      (LPLONG) PTW32_OBJECT_INVALID,
                                      (LPLONG) PTW32_SPIN_UNLOCKED))
        {
          return EINVAL;
        }
      else
        {
          return 0;
        }
    }
  else
    {
      return pthread_mutex_destroy(&(lock->u.mx));
    }
}


int
pthread_spin_lock(pthread_spinlock_t *lock)
{
  if (lock == NULL)
    {
      return EINVAL;
    }

  if (PTW32_SPIN_SPINS(lock))
    {
      while ( PTW32_SPIN_UNLOCKED !=
              InterlockedCompareExchange((LPLONG) &(lock->u.interlock),
                                         (LPLONG) PTW32_SPIN_LOCKED,
                                         (LPLONG) PTW32_SPIN_UNLOCKED) )
        {
          /* Spin */
        }
    }
  else
    {
      return pthread_mutex_lock(&(lock->u.mx));
    }

  return 0;
}

int
pthread_spin_unlock(pthread_spinlock_t *lock)
{
  if (lock == NULL)
    {
      return EINVAL;
    }

  if (PTW32_SPIN_SPINS(lock))
    {
      if (PTW32_SPIN_LOCKED !=
          InterlockedCompareExchange((LPLONG) &(lock->u.interlock),
                                     (LPLONG) PTW32_SPIN_UNLOCKED,
                                     (LPLONG) PTW32_SPIN_LOCKED ) )
        {
          return 0;
        }
      else
        {
          return EINVAL;
        }
    }
  else
    {
      return pthread_mutex_unlock(&(lock->u.mx));
    }
}

int
pthread_spin_trylock(pthread_spinlock_t *lock)
{
  if (lock == NULL)
    {
      return EINVAL;
    }

  if (PTW32_SPIN_SPINS(lock))
    {
      if (PTW32_SPIN_UNLOCKED !=
          InterlockedCompareExchange((LPLONG) &(lock->u.interlock),
                                     (LPLONG) PTW32_SPIN_LOCKED,
                                     (LPLONG) PTW32_SPIN_UNLOCKED ) )
        {
          return EBUSY;
        }
      else
        {
          return 0;
        }
    }
  else
    {
      return pthread_mutex_trylock(&(lock->u.mx));
    }
}
