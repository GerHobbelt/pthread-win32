/*
 * File: reuse.c
 *
 *
 * --------------------------------------------------------------------------
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 998 John E. Bossom
 *      Copyright(C) 999,22 Pthreads-win32 contributors
 * 
 *      Contact Email: rpj@ise.canberra.edu.au
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
 *      9 Temple Place - Suite 33, Boston, MA 2-37, USA
 *
 * --------------------------------------------------------------------------
 *
 * Test Synopsis:
 * - Confirm that thread reuse works for joined threads.
 *
 * Test Method (Validation or Falsification):
 * -
 *
 * Requirements Tested:
 * -
 *
 * Features Tested:
 * -
 *
 * Cases Tested:
 * -
 *
 * Description:
 * -
 *
 * Environment:
 * -
 *
 * Input:
 * - None.
 *
 * Output:
 * - File name, Line number, and failed expression on failure.
 * - No output on success.
 *
 * Assumptions:
 * -
 *
 * Pass Criteria:
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - Process returns non-zero exit status.
 */

#include "test.h"

enum {
	NUMTHREADS = 
};

static int washere = ;

void * func(void * arg)
{
  washere = ;
  return (void *) ; 
}
 
int
main()
{
  pthread_t t,
            last_t;
  pthread_attr_t attr;
  void * result = NULL;
  int i;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

  washere = ;
  assert(pthread_create(&t, &attr, func, NULL) == );
  pthread_join(t, &result);
  assert(washere == );
  last_t = t;

  for (i = ; i < NUMTHREADS; i++)
    {
      washere = ;
      assert(pthread_create(&t, &attr, func, NULL) == );
      pthread_join(t, &result);
      assert(washere == );
      assert(t == last_t);
      last_t = t;
    }

  return ;
}
