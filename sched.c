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
pthread_attr_setschedpolicy(pthread_attr_t *attr,
                            int policy)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  if (policy != SCHED_OTHER)
    {
      return ENOTSUP;
    }

  return 0;
}


int
pthread_attr_getschedpolicy(pthread_attr_t *attr,
                            int * policy)
{
  if (is_attr(attr) != 0 || policy == NULL)
    {
      return EINVAL;
    }

  /*
   * Validate the policy arg.
   * Check that a policy constant wasn't passed rather than &policy.
   */
  if (policy <= (int *) SCHED_MAX)
    {
      return EINVAL;
    }

  *policy = SCHED_OTHER;

  return 0;
}


int
pthread_attr_setschedparam(pthread_attr_t *attr,
			   const struct sched_param *param)
{
  int priority;

  if (is_attr(attr) != 0 || param == NULL)
    {
      return EINVAL;
    }

  priority = param->sched_priority;

  /*
   * Validate priority level. Don't check the
   * return values of the function calls because
   * we're sure they will succeed.
   */
  if (priority < sched_get_priority_min(SCHED_OTHER) ||
      priority > sched_get_priority_max(SCHED_OTHER))
    {
      return EINVAL;
    }

  memcpy(&(*attr)->param, param, sizeof(*param));
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
  
  memcpy(param, &(*attr)->param, sizeof(*param));
  return 0;
}


int
pthread_attr_setinheritsched(pthread_attr_t * attr,
                             int inheritsched)
{
  if (is_attr(attr) != 0)
    {
      return EINVAL;
    }

  if (PTHREAD_INHERIT_SCHED != inheritsched
      && PTHREAD_EXPLICIT_SCHED != inheritsched)
    {
      return EINVAL;
    }

  (*attr)->inheritsched = inheritsched;
  return 0;
}


int
pthread_attr_getinheritsched(pthread_attr_t * attr,
                             int * inheritsched)
{
  if (is_attr(attr) != 0 || inheritsched == NULL)
    {
      return EINVAL;
    }

  *inheritsched = (*attr)->inheritsched;
  return 0;
}


int
pthread_setschedparam(pthread_t thread, int policy,
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
      return ENOTSUP;
    }

  /*
   * Validate priority level. Don't check the
   * return values of the function calls because
   * we're sure they will succeed.
   */
  if (param->sched_priority < sched_get_priority_min(SCHED_OTHER) ||
      param->sched_priority > sched_get_priority_max(SCHED_OTHER))
    {
      return EINVAL;
    }

  /* This is practically guaranteed to return TRUE. */
  (void) SetThreadPriority(thread->threadH, param->sched_priority);

  return 0;
}


int
pthread_getschedparam(pthread_t thread, int *policy,
			  struct sched_param *param)
{
  int prio;

  /* Validate the thread id. */
  if (thread == NULL || thread->threadH == 0)
    {
      return EINVAL;
    }

  /*
   * Validate the policy and param args.
   * Check that a policy constant wasn't passed rather than &policy.
   */
  if (policy <= (int *) SCHED_MAX || param == NULL)
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


int
sched_get_priority_max(int policy)
{
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      errno = EINVAL;
      return -1;
    }

  if (policy != SCHED_OTHER)
    {
      errno = ENOTSUP;
      return -1;
    }

#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
  /* WinCE? */
  return sched_Max(THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);
#else
  /* This is independent of scheduling policy in Win32. */
  return sched_Max(THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_HIGHEST);
#endif
}


int
sched_get_priority_min(int policy)
{
  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      errno = EINVAL;
      return -1;
    }

  if (policy != SCHED_OTHER)
    {
      errno = ENOTSUP;
      return -1;
    }

#if (THREAD_PRIORITY_LOWEST > THREAD_PRIORITY_NORMAL)
  /* WinCE? */
  return sched_Min(THREAD_PRIORITY_IDLE, THREAD_PRIORITY_TIME_CRITICAL);
#else
  /* This is independent of scheduling policy in Win32. */
  return sched_Min(THREAD_PRIORITY_LOWEST, THREAD_PRIORITY_HIGHEST);
#endif
}


int
sched_rr_get_interval(pid_t pid, struct timespec * interval)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function updates the timespec structure
      *      referenced by the interval argument to contain
      *      the current quantum for the process executing
      *      under the SCHED_RR policy. If a process, running under
      *      the round-robin scheduling policy, runs without
      *      blocking or yielding for more than this amount of
      *      time, it may be preempted by another runnable
      *      process (at the same priority).
      *
      *      If the PID argument is zero, the current execution
      *      time limit for the calling process is returned.
      *
      * PARAMETERS
      *      pid    Process identifier.
      *
      *
      * DESCRIPTION
      *      This function updates the timespec structure
      *      referenced by the interval argument to contain
      *      the current quantum for the process executing
      *      under the SCHED_RR policy. If a process, running under
      *      the round-robin scheduling policy, runs without
      *      blocking or yielding for more than this amount of
      *      time, it may be preempted by another runnable
      *      process (at the same priority).
      *
      *      If the PID argument is zero, the current execution
      *      time limit for the calling process is returned.
      *
      * RESULTS
      *      We don't support SCHED_RR so this routine always fails.
      *
      *      NOTE: Since this is part of POSIX 1003.1b
      *                (realtime extensions), it is defined as returning
      *                -1 if an error occurs and sets errno to the actual
      *                error.
      *
      *      errno:  ESRCH           Pid doesn't refer to a valid process.
      *              EPERM           The process referenced by pid does
      *                              not allow query access.
      *              ENOTSUP         SCHED_RR is not supported.
      *
      * ------------------------------------------------------
      */
{
  /*
   * Provide other valid side-effects
   * such as EPERM and ESRCH errors.
   */
  if (0 != pid)
    {
      DWORD selfPid = GetCurrentProcessId();

      if (pid != selfPid)
        {
          HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

          if (NULL == h)
            {
              errno = (GetLastError() == (0xFF & ERROR_ACCESS_DENIED)
                       ? EPERM
                       : ESRCH );

              return -1;
            }

          (void) CloseHandle(h);
        }
    }

  /*
   * We can't return ENOSYS and stay strictly compliant with the
   * standard. We don't support round-robin scheduling so
   * we return ENOTSUP instead. This is consistent with
   * routines which return ENOTSUP if SCHED_RR is given
   * as the policy parameter.
   */
  errno = ENOTSUP;
  return -1;
}


int
sched_setscheduler(pid_t pid, int policy)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function returns sets the scheduling policy and
      *      scheduling parameters of the process specified by pid
      *      to policy and the parameters specified in the
      *      sched_param structure pointed to by param, respectively.
      *      The value of the sched_priority member in the sched_param
      *      structure is any integer within the inclusive priority
      *      range for the scheduling policy specified by policy.
      *      If the value of pid is negative, the behaviour of the
      *      sched_setscheduler() function is unspecified.
      *
      * PARAMETERS
      *      pid        Identifier of the process in which the
      *                 policy will be set.
      *      policy     The new policy value.
      *
      *
      * DESCRIPTION
      *      This function returns sets the scheduling policy and
      *      scheduling parameters of the process specified by pid
      *      to policy and the parameters specified in the
      *      sched_param structure pointed to by param, respectively.
      *      The value of the sched_priority member in the sched_param
      *      structure is any integer within the inclusive priority
      *      range for the scheduling policy specified by policy.
      *      If the value of pid is negative, the behaviour of the
      *      sched_setscheduler() function is unspecified.
      *
      * RESULTS
      *      SCHED_OTHER            on success this is the only possible
      *                             value that can be returned.
      *
      *      NOTE: Since this is part of POSIX 1003.1b
      *                (realtime extensions), it is defined as returning
      *                -1 if an error occurs and sets errno to the actual
      *                error.
      *
      *      errno:  ESRCH           'pid' doesn't refer to an existing
      *                              process.
      *              EPERM           The process referenced by pid does
      *                              not allow set access.
      *              EINVAL          'policy' is not a valid policy value.
      *              ENOSYS          the policy specified is not supported.
      *
      * ------------------------------------------------------
      */
{
  /*
   * Win32 only has one policy which we call SCHED_OTHER.
   * However, we try to provide other valid side-effects
   * such as EPERM and ESRCH errors.
   */
  if (0 != pid)
    {
      DWORD selfPid = GetCurrentProcessId();

      if (pid != selfPid)
        {
          HANDLE h = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);

          if (NULL == h)
            {
              errno = (GetLastError() == (0xFF & ERROR_ACCESS_DENIED)
                       ? EPERM
                       : ESRCH );

              return -1;
            }

          (void) CloseHandle(h);
        }
    }

  if (policy < SCHED_MIN || policy > SCHED_MAX)
    {
      errno = EINVAL;
      return -1;
    }

  if (SCHED_OTHER != policy)
    {
      errno = ENOSYS;
      return -1;
    }

  /*
   * Don't set anything because there is nothing to set.
   * Just return the current (the only possible) value.
   */
  return SCHED_OTHER;
}


int
sched_getscheduler(pid_t pid)
     /*
      * ------------------------------------------------------
      * DOCPUBLIC
      *      This function returns the scheduling policy of the
      *      process specified by pid. If the value of pid is
      *      negative, the behaviour of the sched_getscheduler()
      *      function is unspecified.
      *
      * PARAMETERS
      *      pid    Process identifier.
      *
      *
      * DESCRIPTION
      *      This function returns the scheduling policy of the
      *      process specified by pid. If the value of pid is
      *      negative, the behaviour of the sched_getscheduler()
      *      function is unspecified.
      *
      * RESULTS
      *      SCHED_OTHER            on success this is the only possible
      *                             value that can be returned.
      *
      *      NOTE: Since this is part of POSIX 1003.1b
      *                (realtime extensions), it is defined as returning
      *                -1 if an error occurs and sets errno to the actual
      *                error.
      *
      *      errno:  ESRCH           Pid doesn't refer to a valid process.
      *              EPERM           The process referenced by pid does
      *                              not allow query access.
      *
      * ------------------------------------------------------
      */
{
  /*
   * Win32 only has one policy which we call SCHED_OTHER.
   * However, we try to provide other valid side-effects
   * such as EPERM and ESRCH errors.
   */
  if (0 != pid)
    {
      DWORD selfPid = GetCurrentProcessId();

      if (pid != selfPid)
        {
          HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);

          if (NULL == h)
            {
              errno = (GetLastError() == (0xFF & ERROR_ACCESS_DENIED)
                       ? EPERM
                       : ESRCH );

              return -1;
            }

          (void) CloseHandle(h);
        }
    }

  return SCHED_OTHER;
}


int
sched_yield(void)
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
      *              0               always succeeds
      *
      * ------------------------------------------------------
      */
{
  Sleep(0);

  return 0;
}
