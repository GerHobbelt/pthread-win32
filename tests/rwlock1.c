/* 
 * rwlock1.c
 *
 *
 * Pthreads-win32 - POSIX Threads Library for Win32
 * Copyright (C) 1998 Ben Elliston and Ross Johnson
 * Copyright (C) 1999,2000,2001 Ross Johnson
 *
 * Contact Email: rpj@ise.canberra.edu.au
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * --------------------------------------------------------------------------
 *
 * Create a simple rwlock object, lock it, and then unlock it again.
 *
 * Depends on API functions:
 * 	pthread_rwlock_init()
 *	pthread_rwlock_lock()
 *	pthread_rwlock_unlock()
 *	pthread_rwlock_destroy()
 */

#include "test.h"

pthread_rwlock_t rwlock = NULL;

int
main()
{
  assert(rwlock == NULL);

  assert(pthread_rwlock_init(&rwlock, NULL) == 0);

  assert(rwlock != NULL);

  assert(pthread_rwlock_destroy(&rwlock) == 0);

  assert(rwlock == NULL);

  return 0;
}
