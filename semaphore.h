/*
 * Module: semaphore.h
 *
 * Purpose:
 *	Semaphores aren't actually part of the PThreads standard.
 *	They are defined by the POSIX Standard:
 *
 *		POSIX 1003.1b-1993	(POSIX.1b)
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2012 Pthreads-win32 contributors
 *
 *      Homepage1: http://sourceware.org/pthreads-win32/
 *      Homepage2: http://sourceforge.net/projects/pthreads4w/
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
#if !defined( SEMAPHORE_H )
#define SEMAPHORE_H

#undef PTW32_SEMAPHORE_LEVEL

#if defined(_POSIX_SOURCE)
#define PTW32_SEMAPHORE_LEVEL 0
/* Early POSIX */
#endif

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309
#undef PTW32_SEMAPHORE_LEVEL
#define PTW32_SEMAPHORE_LEVEL 1
/* Include 1b, 1c and 1d */
#endif

#if defined(INCLUDE_NP)
#undef PTW32_SEMAPHORE_LEVEL
#define PTW32_SEMAPHORE_LEVEL 2
/* Include Non-Portable extensions */
#endif

#define PTW32_SEMAPHORE_LEVEL_MAX 3

#if !defined(PTW32_SEMAPHORE_LEVEL)
#define PTW32_SEMAPHORE_LEVEL PTW32_SEMAPHORE_LEVEL_MAX
/* Include everything */
#endif

#if defined(__GNUC__) && ! defined (__declspec)
# error Please upgrade your GNU compiler to one that supports __declspec.
#endif

#if defined(PTW32_STATIC_LIB) && defined(_MSC_VER) && _MSC_VER >= 1400
#  undef PTW32_STATIC_LIB
#  define PTW32_STATIC_TLSLIB
#endif

/*
 * When building the library, you should define PTW32_BUILD so that
 * the variables/functions are exported correctly. When using the library,
 * do NOT define PTW32_BUILD, and then the variables/functions will
 * be imported correctly.
 */
#if defined(PTW32_STATIC_LIB) || defined(PTW32_STATIC_TLSLIB)
#  define PTW32_DLLPORT
#elif defined(PTW32_BUILD)
#    define PTW32_DLLPORT __declspec (dllexport)
#  else
#    define PTW32_DLLPORT __declspec (dllimport)
#  endif

#if !defined(PTW32_CDECL)
# define PTW32_CDECL __cdecl
#endif

/*
 * This is more or less a duplicate of what is in the autoconf config.h,
 * which is only used when building the pthread-win32 libraries.
 */

#if !defined(PTW32_CONFIG_H)
#  if defined(WINCE)
#    undef  HAVE_CPU_AFFINITY
#    define NEED_DUPLICATEHANDLE
#    define NEED_CREATETHREAD
#    define NEED_ERRNO
#    define NEED_CALLOC
#    define NEED_FTIME
/* #    define NEED_SEM */
#    define NEED_UNICODE_CONSTS
#    define NEED_PROCESS_AFFINITY_MASK
/* This may not be needed */
#    define RETAIN_WSALASTERROR
#  elif defined(_MSC_VER)
#    if _MSC_VER >= 1900
#      define HAVE_STRUCT_TIMESPEC
#    elif _MSC_VER < 1300
#      define PTW32_CONFIG_MSVC6
#    elif _MSC_VER < 1400
#      define PTW32_CONFIG_MSVC7
#    endif
#  elif !defined(PTW32_CONFIG_MINGW) && (defined(__MINGW32__) || defined(__MINGW64__))
#    include <_mingw.h>
#    if defined(__MINGW64_VERSION_MAJOR)
#      define PTW32_CONFIG_MINGW 64
#    elif defined(__MINGW_MAJOR_VERSION) || defined(__MINGW32_MAJOR_VERSION)
#      define PTW32_CONFIG_MINGW 32
#    else
#      define PTW32_CONFIG_MINGW 1
#    endif
#    define HAVE_MODE_T
#    if PTW32_CONFIG_MINGW == 64
#      define HAVE_STRUCT_TIMESPEC
#    else
#      undef MINGW_HAS_SECURE_API
#    endif
#  elif defined(_UWIN)
#    define HAVE_MODE_T
#    define HAVE_STRUCT_TIMESPEC
#    define HAVE_SIGNAL_H
#  endif
#endif

/*
 *
 */

#if PTW32_SEMAPHORE_LEVEL >= PTW32_SEMAPHORE_LEVEL_MAX
#if defined(NEED_ERRNO)
#include "need_errno.h"
#else
#include <errno.h>
#endif
#endif /* PTW32_SEMAPHORE_LEVEL >= PTW32_SEMAPHORE_LEVEL_MAX */

#define _POSIX_SEMAPHORES

#if defined(__cplusplus)
extern "C"
{
#endif				/* __cplusplus */

#if !defined(HAVE_MODE_T)
typedef unsigned int mode_t;
#endif


typedef struct sem_t_ * sem_t;

PTW32_DLLPORT int PTW32_CDECL sem_init (sem_t * sem,
					int pshared,
					unsigned int value);

PTW32_DLLPORT int PTW32_CDECL sem_destroy (sem_t * sem);

PTW32_DLLPORT int PTW32_CDECL sem_trywait (sem_t * sem);

PTW32_DLLPORT int PTW32_CDECL sem_wait (sem_t * sem);

PTW32_DLLPORT int PTW32_CDECL sem_timedwait (sem_t * sem,
					     const struct timespec * abstime);

PTW32_DLLPORT int PTW32_CDECL sem_post (sem_t * sem);

PTW32_DLLPORT int PTW32_CDECL sem_post_multiple (sem_t * sem,
						 int count);

PTW32_DLLPORT int PTW32_CDECL sem_open (const char * name,
					int oflag,
					mode_t mode,
					unsigned int value);

PTW32_DLLPORT int PTW32_CDECL sem_close (sem_t * sem);

PTW32_DLLPORT int PTW32_CDECL sem_unlink (const char * name);

PTW32_DLLPORT int PTW32_CDECL sem_getvalue (sem_t * sem,
					    int * sval);

#if defined(__cplusplus)
}				/* End of extern "C" */
#endif				/* __cplusplus */

#undef PTW32_SEMAPHORE_LEVEL
#undef PTW32_SEMAPHORE_LEVEL_MAX

#endif				/* !SEMAPHORE_H */
