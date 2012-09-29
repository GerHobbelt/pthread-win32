/*
 * Module: sched.h
 *
 * Purpose:
 *      Provides an implementation of POSIX realtime extensions
 *      as defined in 
 *
 *              POSIX 1003.1b-1993      (POSIX.1b)
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2005 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@callisto.canberra.edu.au
 * 
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *      http://sources.redhat.com/pthreads-win32/contributors.html
 * 
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation; either
 *      version 2 of the License, or (at your option) any later version.
 * 
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 * 
 *      You should have received a copy of the GNU Lesser General Public
 *      License along with this library in the file COPYING.LIB;
 *      if not, write to the Free Software Foundation, Inc.,
 *      59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */
#ifndef _SCHED_H
#define _SCHED_H

#undef PTE_LEVEL

#if defined(_POSIX_SOURCE)
#define PTE_LEVEL 0
/* Early POSIX */
#endif

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309
#undef PTE_LEVEL
#define PTE_LEVEL 1
/* Include 1b, 1c and 1d */
#endif

#if defined(INCLUDE_NP)
#undef PTE_LEVEL
#define PTE_LEVEL 2
/* Include Non-Portable extensions */
#endif

#define PTE_LEVEL_MAX 3

#if !defined(PTE_LEVEL)
#define PTE_LEVEL PTE_LEVEL_MAX
/* Include everything */
#endif


#if __GNUC__ && ! defined (__declspec)
# error Please upgrade your GNU compiler to one that supports __declspec.
#endif

/*
 * When building the DLL code, you should define PTE_BUILD so that
 * the variables/functions are exported correctly. When using the DLL,
 * do NOT define PTE_BUILD, and then the variables/functions will
 * be imported correctly.
 */
#ifndef PTE_STATIC_LIB
#  ifdef PTE_BUILD
#    define PTE_DLLPORT __declspec (dllexport)
#  else
#    define PTE_DLLPORT __declspec (dllimport)
#  endif
#else
#  define PTE_DLLPORT
#endif

/*
 * This is a duplicate of what is in the autoconf config.h,
 * which is only used when building the pthread-win32 libraries.
 */

#ifndef PTE_CONFIG_H
#  if defined(WINCE)
#    define NEED_ERRNO
#    define NEED_SEM
#  endif
#  if defined(_UWIN) || defined(__MINGW32__)
#    define HAVE_MODE_T
#  endif
#endif

/*
 *
 */

#if PTE_LEVEL >= PTE_LEVEL_MAX
#ifdef NEED_ERRNO
#include "need_errno.h"
#else
#include <errno.h>
#endif
#endif /* PTE_LEVEL >= PTE_LEVEL_MAX */

#if defined(__MINGW32__) || defined(_UWIN)
#if PTE_LEVEL >= PTE_LEVEL_MAX
/* For pid_t */
#  include <sys/types.h>
/* Required by Unix 98 */
#  include <time.h>
#endif /* PTE_LEVEL >= PTE_LEVEL_MAX */
#else
typedef int pid_t;
#endif

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

PTE_DLLPORT int __cdecl sched_yield (void);

PTE_DLLPORT int __cdecl sched_get_priority_min (int policy);

PTE_DLLPORT int __cdecl sched_get_priority_max (int policy);

PTE_DLLPORT int __cdecl sched_setscheduler (pid_t pid, int policy);

PTE_DLLPORT int __cdecl sched_getscheduler (pid_t pid);

/*
 * Note that this macro returns ENOTSUP rather than
 * ENOSYS as might be expected. However, returning ENOSYS
 * should mean that sched_get_priority_{min,max} are
 * not implemented as well as sched_rr_get_interval.
 * This is not the case, since we just don't support
 * round-robin scheduling. Therefore I have chosen to
 * return the same value as sched_setscheduler when
 * SCHED_RR is passed to it.
 */
#define sched_rr_get_interval(_pid, _interval) \
  ( errno = ENOTSUP, (int) -1 )


#ifdef __cplusplus
}                               /* End of extern "C" */
#endif                          /* __cplusplus */

#undef PTE_LEVEL
#undef PTE_LEVEL_MAX

#endif                          /* !_SCHED_H */

