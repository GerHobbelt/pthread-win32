/*
 * mutex8n.c
 *
 *
 *      pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999-2021 pthreads-win32 / pthreads4w contributors
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
 *
 * --------------------------------------------------------------------------
 *
 * Tests PTHREAD_MUTEX_NORMAL mutex type exercising timedlock.
 * Thread locks mutex, another thread timedlocks the mutex.
 * Timed thread should timeout.
 *
 * Depends on API functions:
 *	pthread_create()
 *	pthread_mutexattr_init()
 *	pthread_mutexattr_destroy()
 *	pthread_mutexattr_settype()
 *	pthread_mutexattr_gettype()
 *	pthread_mutex_init()
 *	pthread_mutex_destroy()
 *	pthread_mutex_lock()
 *	pthread_mutex_timedlock()
 *	pthread_mutex_unlock()
 */

#include "test.h"

static int lockCount = 0;

static pthread_mutex_t mutex;
static pthread_mutexattr_t mxAttr;

static void * locker(void * arg)
{
  struct timespec abstime, reltime = { 1, 0 };

  (void) pthread_win32_getabstime_np(&abstime, &reltime);

  assert(pthread_mutex_timedlock(&mutex, &abstime) == ETIMEDOUT);

  lockCount++;

  return 0;
}

#ifndef MONOLITHIC_PTHREAD_TESTS
int
main()
#else
int
test_mutex8n(void)
#endif
{
  pthread_t t;
  int mxType = -1;

  assert(pthread_mutexattr_init(&mxAttr) == 0);

  BEGIN_MUTEX_STALLED_ROBUST(mxAttr)
  {
	  lockCount = 0;
	  assert(pthread_mutexattr_settype(&mxAttr, PTHREAD_MUTEX_NORMAL) == 0);
	  assert(pthread_mutexattr_gettype(&mxAttr, &mxType) == 0);
	  assert(mxType == PTHREAD_MUTEX_NORMAL);

	  assert(pthread_mutex_init(&mutex, &mxAttr) == 0);

	  assert(pthread_mutex_lock(&mutex) == 0);

	  assert(pthread_create(&t, NULL, locker, NULL) == 0);

  while (lockCount < 1)
    {
      Sleep(1);
    }

	  assert(lockCount == 1);

	  assert(pthread_mutex_unlock(&mutex) == 0);

      assert(pthread_mutex_destroy(&mutex) == 0);
	  assert(mutex == NULL);
  }
  END_MUTEX_STALLED_ROBUST(mxAttr)

  assert(pthread_mutexattr_destroy(&mxAttr) == 0);

  return 0;
}

