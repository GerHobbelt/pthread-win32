/*
 * benchtest1.c
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
 * Measure time taken to complete an elementary operation.
 *
 * - Mutex
 *   Single thread iteration over lock/unlock for each mutex type.
 */

#include "test.h"
#include <sys/timeb.h>

#ifdef __GNUC__
#include <stdlib.h>
#endif

#include "benchtest.h"

#define PTW32_MUTEX_TYPES
#define ITERATIONS      10000000L

pthread_mutex_t mx;
pthread_mutexattr_t ma;
struct _timeb currSysTimeStart;
struct _timeb currSysTimeStop;
long durationMilliSecs;
long overHeadMilliSecs = 0;

#define GetDurationMilliSecs(_TStart, _TStop) ((_TStop.time*1000+_TStop.millitm) \
                                               - (_TStart.time*1000+_TStart.millitm))

/*
 * Dummy use of j, otherwise the loop may be removed by the optimiser
 * when doing the overhead timing with an empty loop.
 */
#define TESTSTART \
  { int i, j = 0, k = 0; _ftime(&currSysTimeStart); for (i = 0; i < ITERATIONS; i++) { j++;

#define TESTSTOP \
  }; _ftime(&currSysTimeStop); if (j + k == i) j++; }


void
runTest (char * testNameString, int mType)
{
#ifdef PTW32_MUTEX_TYPES
  assert(pthread_mutexattr_settype(&ma, mType) == 0);
#endif
  assert(pthread_mutex_init(&mx, &ma) == 0);

  TESTSTART
  assert(pthread_mutex_lock(&mx) == 0);
  assert(pthread_mutex_unlock(&mx) == 0);
  TESTSTOP

  assert(pthread_mutex_destroy(&mx) == 0);

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;

  printf( "%-40s %15ld %15.3f\n",
	    testNameString,
          durationMilliSecs,
          (float) durationMilliSecs * 1E3 / ITERATIONS);
}


int
main (int argc, char *argv[])
{
  CRITICAL_SECTION cs;
  old_mutex_t ox;

  pthread_mutexattr_init(&ma);

  printf( "========================================================================\n");
  printf( "\nLock plus unlock on an unlocked mutex.\n%ld iterations\n\n",
          ITERATIONS);
  printf( "%-40s %15s %15s\n",
	    "Test",
	    "Total(msec)",
	    "average(usec)");
  printf( "------------------------------------------------------------------------\n");

  /*
   * Time the loop overhead so we can subtract it from the actual test times.
   */

  TESTSTART
  assert(1 == 1);
  assert(1 == 1);
  TESTSTOP

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
  overHeadMilliSecs = durationMilliSecs;


  InitializeCriticalSection(&cs);

  TESTSTART
  assert((EnterCriticalSection(&cs), 1) == 1);
  assert((LeaveCriticalSection(&cs), 1) == 1);
  TESTSTOP

  DeleteCriticalSection(&cs);

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;

  printf( "%-40s %15ld %15.3f\n",
	    "Simple Critical Section",
          durationMilliSecs,
          (float) durationMilliSecs * 1E3 / ITERATIONS);


  old_mutex_use = OLD_WIN32CS;
  assert(old_mutex_init(&ox, NULL) == 0);

  TESTSTART
  assert(old_mutex_lock(&ox) == 0);
  assert(old_mutex_unlock(&ox) == 0);
  TESTSTOP

  assert(old_mutex_destroy(&ox) == 0);

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;

  printf( "%-40s %15ld %15.3f\n",
	    "PT Mutex using a Critical Section (WNT)",
          durationMilliSecs,
          (float) durationMilliSecs * 1E3 / ITERATIONS);


  old_mutex_use = OLD_WIN32MUTEX;
  assert(old_mutex_init(&ox, NULL) == 0);

  TESTSTART
  assert(old_mutex_lock(&ox) == 0);
  assert(old_mutex_unlock(&ox) == 0);
  TESTSTOP

  assert(old_mutex_destroy(&ox) == 0);

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;

  printf( "%-40s %15ld %15.3f\n",
	    "PT Mutex using a Win32 Mutex (W9x)",
          durationMilliSecs,
          (float) durationMilliSecs * 1E3 / ITERATIONS);

  printf( "........................................................................\n");

  /*
   * Now we can start the actual tests
   */
#ifdef PTW32_MUTEX_TYPES
  runTest("PTHREAD_MUTEX_DEFAULT (W9x,WNT)", PTHREAD_MUTEX_DEFAULT);

  runTest("PTHREAD_MUTEX_NORMAL (W9x,WNT)", PTHREAD_MUTEX_NORMAL);

  runTest("PTHREAD_MUTEX_ERRORCHECK (W9x,WNT)", PTHREAD_MUTEX_ERRORCHECK);

  runTest("PTHREAD_MUTEX_RECURSIVE (W9x,WNT)", PTHREAD_MUTEX_RECURSIVE);
#else
  runTest("Non-blocking lock", 0);
#endif

  printf( "========================================================================\n");

  /*
   * End of tests.
   */

  pthread_mutexattr_destroy(&ma);

  return 0;
}
