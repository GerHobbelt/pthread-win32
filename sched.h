/*
 * Module: sched.h
 *
 * Purpose:
 *      Provides an implementation of POSIX realtime extensions
 *      as defined in 
 *
 *              POSIX 1003.1b-1993      (POSIX.1b)
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
#ifndef _SCHED_H
#define _SCHED_H

#if !defined( PTW32_HEADER )
#define PTW32_HEADER

#ifdef _UWIN
#   define HAVE_STRUCT_TIMESPEC 1
#   define HAVE_SIGNAL_H        1
#   undef HAVE_CONFIG_H
#   pragma comment(lib, "pthread")
#endif

#endif /* PTW32_HEADER */

#if defined(__MINGW32__) || defined(_UWIN)
/* For pid_t */
#  include <sys/types.h>
/* Required by Unix 98 - including sched.h makes time.h available */
#  include <time.h>
#else
typedef DWORD pid_t;
#endif

#if ! defined(HAVE_STRUCT_TIMESPEC) && ! defined(PTW32_TIMESPEC)
#define PTW32_TIMESPEC
struct timespec {
	long tv_sec;
	long tv_nsec;
};
#endif /* HAVE_STRUCT_TIMESPEC */


/* Thread scheduling policies */
enum {
  SCHED_OTHER = 0,
  SCHED_FIFO,
  SCHED_RR,
  SCHED_MIN   = SCHED_OTHER,
  SCHED_MAX   = SCHED_RR
};

struct sched_param {
  int sched_priority;
};

#ifdef __cplusplus
extern "C"
{
#endif                          /* __cplusplus */

int sched_yield (void);

int sched_get_priority_min (int policy);

int sched_get_priority_max (int policy);

int sched_setscheduler (pid_t pid, int policy);

int sched_getscheduler (pid_t pid);

int sched_rr_get_interval(pid_t pid, struct timespec * interval);

#ifdef __cplusplus
}                               /* End of extern "C" */
#endif                          /* __cplusplus */


#endif                          /* !_SCHED_H */
