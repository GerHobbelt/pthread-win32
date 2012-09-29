/*
 * mutex.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 */

#include <errno.h>

#include "pthread.h"
#include "implement.h"

int
pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
  if (mutex == NULL)
    {
      return EINVAL;
    }

  /* Create a critical section. */
  InitializeCriticalSection(&mutex->cs);

  /* Mark as valid. */
  mutex->valid = 1;

  return 0;
}

int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  if (mutex == NULL)
    {
      return EINVAL;
    }

  DeleteCriticalSection(&mutex->cs);
  
  /* Mark as invalid. */
  mutex->valid = 0;

  return 0;
}

int
pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
  if (attr == NULL)
    {
      /* This is disallowed. */
      return EINVAL;
    }

  /* None of the optional attributes are supported yet. */
  return 0;
}

int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
  /* Nothing to do. */
  return 0;
}

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  if (!mutex->valid)
    {
      pthread_mutex_init(mutex, NULL);
    }
  EnterCriticalSection(&mutex->cs);
  return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  if (!mutex->valid)
    {
      return EINVAL;
    }
  LeaveCriticalSection(&mutex->cs);
  return 0;
}

int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  if (_pthread_try_enter_critical_section == NULL)
    {
      /* TryEnterCriticalSection does not exist in the OS; return ENOSYS. */
      return ENOSYS;
    }
      
  if (mutex == NULL)
    {
      return EINVAL;
    }

  if (!mutex->valid)
    {
      pthread_mutex_init(mutex, NULL);
    }

  return ((*_pthread_try_enter_critical_section)(&mutex->cs) != TRUE) ? EBUSY : 0;
}
