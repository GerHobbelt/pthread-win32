/*
 * mutex.c
 *
 * Description:
 * This translation unit implements mutual exclusion (mutex) primitives.
 */

#include "pthread.h"

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
