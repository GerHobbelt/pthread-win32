/*
 * nonportable.c
 *
 * Description:
 * This translation unit implements non-portable thread functions.
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

/*
 * pthread_mutexattr_setforcecs_np()
 *
 * Allows an application to force the library to use
 * critical sections rather than win32 mutexes as
 * the basis for any mutexes that use "attr".
 *
 * Values for "forcecs" are defined in pthread.h
 */
int
pthread_mutexattr_setforcecs_np(pthread_mutexattr_t *attr,
				int forcecs)
{
  if (attr == NULL || *attr == NULL)
    {
      /* This is disallowed. */
      return EINVAL;
    }

  (*attr)->forcecs = forcecs;

  return 0;
}

/*
 * pthread_getw32threadhandle_np()
 *
 * Returns the win32 thread handle that the POSIX
 * thread "thread" is running as.
 *
 * Applications can use the win32 handle to set
 * win32 specific attributes of the thread.
 */
HANDLE
pthread_getw32threadhandle_np(pthread_t thread)
{
  return (thread != NULL) ? (thread->threadH) : 0;
}
