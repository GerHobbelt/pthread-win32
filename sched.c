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

int pthread_setschedparam(pthread_t thread, int policy,
			  const struct sched_param *param)
{
  /* Validate the thread id. */
  if (_PTHREAD_VALID(thread) < 0)
    {
      return EINVAL;
    }

  /* Validate the scheduling policy. */
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      return EINVAL;
    }

  /* Ensure the policy is SCHED_OTHER. */
  if (policy != SCHED_OTHER)
    {
      return ENOSUP;
    }

  /* Validate priority level. */
  if (param->sched_policy < sched_get_priority_min(policy) ||
      param->sched_policy > sched_get_priority_max(policy))
    {
      return EINVAL;
    }

  /* This is practically guaranteed to return TRUE. */
  (void) SetThreadPriority(thread->win32handle, param->sched_policy);
  return 0;
}

int pthread_getschedparam(pthread_t thread, int *policy,
			  struct sched_param *param)
{
  int prio;

  /* Validate the thread id. */
  if (_PTHREAD_VALID(thread) != 0)
    {
      return EINVAL;
    }

  /* Validate the param structure. */
  if (param == NULL)
    {
      return EINVAL;
    }

  /* Fill out the policy. */
  *policy = SCHED_OTHER;

  /* Fill out the sched_param structure. */
  prio = GetThreadPriority(thread->win32handle);
  if (prio == THREAD_PRIORITY_ERROR_RETURN)
    {
      return EINVAL;
    }
  
  param->sched_policy = prio;
  return 0;
}

int sched_get_priority_max(int policy)
{
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      return EINVAL;
    }

  /* This is independent of scheduling policy in Win32. */
  return THREAD_PRIORITY_HIGHEST;
}

int sched_get_priority_min(int policy)
{
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      return EINVAL;
    }

  /* This is independent of scheduling policy in Win32. */
  return THREAD_PRIORITY_LOWEST;
}
