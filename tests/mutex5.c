/* 
 * mutex5.c
 *
 *
 * --------------------------------------------------------------------------
 *
 *      Pthreads-win32 - POSIX Threads Library for Win32
 *      Copyright(C) 1998 John E. Bossom
 *      Copyright(C) 1999,2004 Pthreads-win32 contributors
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
 *
 * --------------------------------------------------------------------------
 *
 * Confirm the equality/inequality of the various mutex types,
 * and the default not-set value.
 */

#include "test.h"

static pthread_mutexattr_t mxAttr;

int
main()
{
  int mxType = -1;
  int success = 0;   /* Use to quell GNU compiler warnings. */

  assert(success = PTHREAD_MUTEX_DEFAULT == PTHREAD_MUTEX_NORMAL);
  assert(success = PTHREAD_MUTEX_DEFAULT != PTHREAD_MUTEX_ERRORCHECK);
  assert(success = PTHREAD_MUTEX_DEFAULT != PTHREAD_MUTEX_RECURSIVE);
  assert(success = PTHREAD_MUTEX_RECURSIVE != PTHREAD_MUTEX_ERRORCHECK);

  assert(success = PTHREAD_MUTEX_NORMAL == PTHREAD_MUTEX_FAST_NP);
  assert(success = PTHREAD_MUTEX_RECURSIVE == PTHREAD_MUTEX_RECURSIVE_NP);
  assert(success = PTHREAD_MUTEX_ERRORCHECK == PTHREAD_MUTEX_ERRORCHECK_NP);

  if (success == success)
    {
      assert(pthread_mutexattr_init(&mxAttr) == 0);
      assert(pthread_mutexattr_gettype(&mxAttr, &mxType) == 0);
      assert(mxType == PTHREAD_MUTEX_NORMAL);
    }

  return 0;
}
