/*
 * mutex.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 */

#include "pthread.h"
#include "implement.h"

static int
insert_attr(pthread_mutexattr_t *attr)
{
  /* Add this attribute object to a list. */

  /* FIXME: implement using some dynamic scheme. */
  return 0;
}

static int
is_attr(pthread_mutexattr_t *attr)
{
  /* Return 0 if present, 1 otherwise. */

  /* FIXME: implement. For now, pretend the attribute is always okay, unless
     it is NULL. */

  return (attr == NULL) ? 1 : 0;
}

static int
remove_attr(pthread_mutexattr_t *attr)
{
  /* Remove this attribute object from the list. */

  /* FIXME: implement. */
  return 0;
}

int
pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutex_attr_t *attr)
{
  /* FIXME: Attributes are currently ignored. */
  
  if ((is_attr(attr) != 0) || (mutex == NULL))
    {
      return EINVAL;
    }

  /* Create an unnamed mutex with default security and one which is
     initially not held by the thread.  Default security (arg1 = NULL)
     is portable to Win9x and NT, as Win9x has no security model. */

  if ((*mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
    {
      /* This is almost certainly due to a lack of resources. */
      return ENOMEM;
    }

  return 0;
}

int
pthread_mutex_destroy(pthread_mutex_t *mutex)
{
  if ((mutex == NULL) || (CloseHandle(*mutex) != TRUE))
    {
      /* This is almost certainly due to an invalid handle. */
      return EINVAL;
    }

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

  attr->ptr = malloc(sizeof(_pthread_mutexattr_t));
  if (attr->ptr == NULL)
    {
      return ENOMEM;
    }
  
  (_pthread_mutexattr_t *) (attr->ptr).proc_shared = PTHREAD_PROCESS_PRIVATE;
  return 0;
}

int
pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  free(attr->ptr);
  return 0;
}

int
pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int process_shared)
{
  /* Ensure the value for this attribute is within the legal range. */
  if ((process_shared != PTHREAD_PROCESS_PRIVATE) ||
      (process_shared != PTHREAD_PROCESS_SHARED))
    {
      return EINVAL;
    }

  /* Ensure attr points to a valid attribute object. */
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  /* Everything is okay. */
  (_pthread_mutexattr_t *) (attr->ptr)->proc_shared = process_shared;
  return 0;
}

int
pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr,
			     int *process_shared)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  /* Everything is okay. */
  *process_shared = (_pthread_mutexattr_t *) (attr->ptr)->proc_shared;
  return 0;
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
pthread_lock(pthread_mutex_t *mutex)
{
  switch (WaitForSingleObject(*mutex, INFINITE))
    {
    case WAIT_ABANDONED_0:
      /* Thread holding the mutex abandoned it.  Fall through. */
    case WAIT_FAILED:
      /* This is probably due to an invalid handle. */
      return EINVAL;
    case WAIT_OBJECT_0:
      /* We're good. */
      return 0;
    }
  /* Not reached. */
}

int
pthread_unlock(pthread_mutex_t *mutex)
{
  if (ReleaseMutex(*mutex) != TRUE)
    {
      /* This is probably due to an invalid handle. */
      return EINVAL;
    }
  else
    {
      return 0;
    }

  /* Not reached. */
}

int
pthread_trylock(pthread_mutex_t *mutex)
{
  /* If the mutex is already held, return EBUSY. */
  switch (WaitForSingleObject(*mutex, 0))
    {
    case WAIT_ABANDONED_0:
      /* Thread holding the mutex abandoned it.  Fall through. */
    case WAIT_FAILED:
      /* This is probably due to an invalid handle. */
      return EINVAL;
    case WAIT_TIMEOUT:
      /* We couldn't get the lock. */
      return EBUSY;
    case WAIT_OBJECT_0:
      /* We're good. */
      return 0;
    }
  /* Not reached. */
}
