/*
 * mutex.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 */

#include "pthread.h"
#include "implement.h"

int
pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutex_attr_t *attr)
{
  if (mutex == NULL)
    {
      return EINVAL;
    }

  /* Create a critical section. */
  InitializeCriticalSection(mutex);

  return 0;
}

int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  if (mutex == NULL)
    {
      return EINVAL;
    }

  DeleteCriticalSection(mutex);
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
pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int process_shared)
{
  /* This function is not supported. */
  return ENOSYS;
}

int
pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
			     int *process_shared)
{
  /* This function is not supported. */
  return ENOSYS;
}
  
int
pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr,
			      int protocol)
{
  /* This function is not supported. */
  return ENOSYS;
}

int
pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr,
			     int *protocol)
{
  /* This function is not supported. */
  return ENOSYS;
}

int
pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr,
				 int ceiling)
{
  /* This function is not supported. */
  return ENOSYS;
}

int
pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr,
				 int *ceiling)
{
  /* This function is not supported. */
  return ENOSYS;
}

int
pthread_mutex_lock(pthread_mutex_t *mutex)
{
  EnterCriticalSection(mutex);
  return 0;
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex)
{
  LeaveCriticalSection(mutex);
  return 0;
}

int
pthread_mutex_trylock(pthread_mutex_t *mutex)
{
  /* Typically evaluates to 31. */
  int numbits = (sizeof(DWORD) * 8) - 1;

  if ((GetVersion() >> numbits) != 1)
    {
      /* We're not on Windows NT; return ENOSYS. */
      return ENOSYS;
    }
      
  if (mutex == NULL)
    {
      return EINVAL;
    }

  return (TryEnterCriticalSection(mutex) != TRUE) ? EBUSY : 0;
}
