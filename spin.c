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
  (0 == ((long) ((_lock->u).mx) & ~(PTW32_SPIN_LOCKED | PTW32_SPIN_UNLOCKED | (long) PTW32_OBJECT_INVALID)))


int
pthread_spin_init(pthread_spinlock_t *lock, int pshared)
{
  pthread_spinlock_t s;
  int CPUs = 1;
  int result = 0;

  if (lock == NULL || *lock == NULL)
    {
      return EINVAL;
    }

  s = (pthread_spinlock_t) calloc(1, sizeof(*s));

  if (s == NULL)
    {
      return ENOMEM;
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

      s->u.interlock = PTW32_SPIN_UNLOCKED;
    }
  else
    {
      pthread_mutexattr_t ma;
      result = pthread_mutexattr_init(&ma);

      if (0 == result)
        {
          ma->pshared = pshared;
          result = pthread_mutex_init(&(s->u.mx), &ma);
        }
    }

FAIL0:
  *lock = (0 == result ? s : NULL);
  return(result);
}

int
pthread_spin_destroy(pthread_spinlock_t *lock)
{
  pthread_spinlock_t s;

  if (lock == NULL || *lock == NULL)
    {
      return EINVAL;
    }

  s = *lock;

  if (PTW32_SPIN_SPINS(s))
    {
      if ( PTW32_SPIN_UNLOCKED !=
           InterlockedCompareExchange((LPLONG) &(s->u.interlock),
                                      (LONG) PTW32_OBJECT_INVALID,
                                      (LONG) PTW32_SPIN_UNLOCKED))
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
      return pthread_mutex_destroy(&(s->u.mx));
    }
}


int
pthread_spin_lock(pthread_spinlock_t *lock)
{
  pthread_spinlock_t s;

  if (lock == NULL || *lock == NULL)
    {
      return EINVAL;
    }

  s = *lock;

  if (PTW32_SPIN_SPINS(s))
    {
      while ( PTW32_SPIN_UNLOCKED !=
              InterlockedCompareExchange((LPLONG) &(s->u.interlock),
                                         (LONG) PTW32_SPIN_LOCKED,
                                         (LONG) PTW32_SPIN_UNLOCKED) )
        {
          /* Spin */
        }
    }
  else
    {
      return pthread_mutex_lock(&(s->u.mx));
    }

  return 0;
}

int
pthread_spin_unlock(pthread_spinlock_t *lock)
{
  pthread_spinlock_t s;

  if (lock == NULL || lock == NULL)
    {
      return EINVAL;
    }

  s = *lock;

  if (PTW32_SPIN_SPINS(s))
    {
      if (PTW32_SPIN_LOCKED !=
          InterlockedCompareExchange((LPLONG) &(s->u.interlock),
                                     (LONG) PTW32_SPIN_UNLOCKED,
                                     (LONG) PTW32_SPIN_LOCKED ) )
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
      return pthread_mutex_unlock(&(s->u.mx));
    }
}

int
pthread_spin_trylock(pthread_spinlock_t *lock)
{
  pthread_spinlock_t s;

  if (lock == NULL || *lock == NULL)
    {
      return EINVAL;
    }

  s = *lock;

  if (PTW32_SPIN_SPINS(s))
    {
      if (PTW32_SPIN_UNLOCKED !=
          InterlockedCompareExchange((LPLONG) &(s->u.interlock),
                                     (LONG) PTW32_SPIN_LOCKED,
                                     (LONG) PTW32_SPIN_UNLOCKED ) )
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
      return pthread_mutex_trylock(&(s->u.mx));
    }
}
