/*
 * sched.c
 * 
 * Description:
 * POSIX thread functions that deal with thread scheduling.
 */

#include "pthread.h"

int
pthread_attr_setschedparam(pthread_attr_t *attr,
			   const struct sched_param *param)
{
  if (is_attr(attr) != 0 || param == NULL)
    {
      return EINVAL;
    }

  attr->priority = param->sched_priority;
  return 0;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr,
			       struct sched_param *param)
{
  if (is_attr(attr) != 0 || param == NULL)
    {
      return EINVAL;
    }
  
  param->sched_priority = attr->priority;
  return 0;
}

int sched_get_priority_max(int policy)
{
  /* This is independent of scheduling policy in Win32. */
  return THREAD_PRIORITY_HIGHEST;
}

int sched_get_priority_min(int policy)
{
  /* This is independent of scheduling policy in Win32. */
  return THREAD_PRIORITY_LOWEST;
}
