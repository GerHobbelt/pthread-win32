/*
 * Test for pthread_join().
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
 * Depends on API functions: pthread_create(), pthread_exit().
 */

#include "test.h"

void *
func(void * arg)
{
  Sleep(2000);

  pthread_exit(arg);

  /* Never reached. */
  exit(1);
}

int
main(int argc, char * argv[])
{
  pthread_t id;
  int result;

  /* Create a single thread and wait for it to exit. */
  assert(pthread_create(&id, NULL, func, (void *) 123) == 0);

  assert(pthread_join(id, (void **) &result) == 0);

#if ! defined (__MINGW32__) || defined (__MSVCRT__)
  assert(result == 123);
#else
# warning pthread_join not fully supported in this configuration.
  assert(result == 0);
#endif

  /* Success. */
  return 0;
}
