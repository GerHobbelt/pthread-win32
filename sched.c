/*
 * sched.c
 * 
 * Description:
 * POSIX thread functions that deal with thread scheduling.
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

#define ENOSUP 0

#include <errno.h>

#include "pthread.h"
#include "implement.h"

static int
is_attr(const pthread_attr_t *attr)
{
  return (attr == NULL || 
	  *attr == NULL || 
	  (*attr)->valid != _PTHREAD_ATTR_VALID) ? 1 : 0;
}

int
pthread_attr_setschedparam(pthread_attr_t *attr,
			   const struct sched_param *param)
{
  if (is_attr(attr) != 0 || param == NULL)
    {
      return EINVAL;
    }

  (*attr)->priority = param->sched_priority;
  return 0;
}

int 
pthread_attr_getschedparam(const pthread_attr_t *attr,
			       struct sched_param *param)
{
  if (is_attr(attr) != 0 || param == NULL)
    {
      return EINVAL;
    }
  
  param->sched_priority = (*attr)->priority;
  return 0;
}

int pthread_setschedparam(pthread_t thread, int policy,
			  const struct sched_param *param)
{
  /* Validate the thread id. */
  if (thread == NULL || thread->threadH == 0)
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
  if (param->sched_priority < sched_get_priority_min(policy) ||
      param->sched_priority > sched_get_priority_max(policy))
    {
      return EINVAL;
    }

  /* This is practically guaranteed to return TRUE. */
  (void) SetThreadPriority(thread->threadH, param->sched_priority);

  return 0;
}

int pthread_getschedparam(pthread_t thread, int *policy,
			  struct sched_param *param)
{
  int prio;

  /* Validate the thread id. */
  if (thread == NULL || thread->threadH == 0)
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
  prio = GetThreadPriority(thread->threadH);
  if (prio == THREAD_PRIORITY_ERROR_RETURN)
    {
      return EINVAL;
    }
  
  param->sched_priority = prio;
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
