/* 
 * barrier2.c
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
 * Declare a single barrier object, wait on it, 
 * and then destroy it.
 *
 */

#include "test.h"
 
pthread_barrier_t barrier = NULL;

int
main()
{
  assert(pthread_barrier_init(&barrier, NULL, 1) == 0);

  assert(pthread_barrier_wait(&barrier) == PTHREAD_BARRIER_SERIAL_THREAD);

  assert(pthread_barrier_destroy(&barrier) == 0);

  return 0;
}
