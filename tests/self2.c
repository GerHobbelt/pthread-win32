/*
 * self2.c
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
 * Test for pthread_self().
 *
 * Depends on API functions:
 *	pthread_create()
 *	pthread_self()
 *
 * Implicitly depends on:
 *	pthread_getspecific()
 *	pthread_setspecific()
 */

#include "test.h"
#include <string.h>

static pthread_t me;

void *
entry(void * arg)
{
  me = pthread_self();

  return arg;
}

int
main()
{
  pthread_t t;

  assert(pthread_create(&t, NULL, entry, NULL) == 0);

  Sleep(2000);

  /*
   * Not much more we can do here but bytewise compare t with
   * what pthread_self returned.
   */
  assert(t == me);
  assert(memcmp((const void *) t, (const void *) me, sizeof t) == 0);

  /* Success. */
  return 0;
}
