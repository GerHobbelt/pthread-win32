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

#include "pthread.h"
#include "implement.h"
#include "sched.h"

static int
is_attr(const pthread_attr_t *attr)
{
  return (attr == NULL || 
	  *attr == NULL || 
	  (*attr)->valid != PTW32_ATTR_VALID) ? 1 : 0;
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


/*
 * On Windows98, THREAD_PRIORITY_LOWEST is (-2) and 
 * THREAD_PRIORITY_HIGHEST is 2, and everything works just fine.
 * 
 * On WinCE 3.0, it so happen that THREAD_PRIORITY_LOWEST is 5
 * and THREAD_PRIORITY_HIGHEST is 1 (yes, I know, it is funny:
 * highest priority use smaller numbers) and the following happens:
 * 
 * sched_get_priority_min() returns 5
 * sched_get_priority_max() returns 1
 */

#define sched_Max(a,b)  ((a)<(b)?(b):(a))
#define sched_Min(a,b)  ((a)>(b)?(b):(a))

int sched_get_priority_max(int policy)
{
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      return EINVAL;
    }

#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
  /* WinCE? */
  return sched_Max(THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);
#else
  /* This is independent of scheduling policy in Win32. */
  return sched_Max(THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_HIGHEST);
#endif
}

int sched_get_priority_min(int policy)
{
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      return EINVAL;
    }

#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
  /* WinCE? */
  return sched_Min(THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);
#else
  /* This is independent of scheduling policy in Win32. */
  return sched_Min(THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_HIGHEST);
#endif
}

int sched_yield(void)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function indicates that the calling thread is
      *      willing to give up some time slices to other threads.
      *
      * PARAMETERS
      *      N/A
      *
      *
      * DESCRIPTION
      *      This function indicates that the calling thread is
      *      willing to give up some time slices to other threads.
      *      NOTE: Since this is part of POSIX 1003.1b
      *                (realtime extensions), it is defined as returning
      *                -1 if an error occurs and sets errno to the actual
      *                error.
      *
      * RESULTS
      *              0               successfully created semaphore,
      *              ENOSYS          sched_yield not supported,
      *
      * ------------------------------------------------------
      */
{
  Sleep(0);

  return 0;
}
