/*
 * benchtest4.c
 *
 * Measure time taken to complete an elementary operation.
 *
 * - Mutex
 *   Single thread iteration over trylock/unlock for each mutex type.
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
old_mutex_t ox;
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
oldRunTest (char * testNameString, int mType)
{
}


void
runTest (char * testNameString, int mType)
{
#ifdef PTW32_MUTEX_TYPES
  pthread_mutexattr_settype(&ma, mType);
#endif
  pthread_mutex_init(&mx, &ma);

  TESTSTART
  (void) pthread_mutex_trylock(&mx);
  (void) pthread_mutex_unlock(&mx);
  TESTSTOP

  pthread_mutex_destroy(&mx);

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;

  printf( "%-40s %15ld %15.3f\n",
	    testNameString,
          durationMilliSecs,
          (float) durationMilliSecs * 1E3 / ITERATIONS);
}


int
main (int argc, char *argv[])
{
  pthread_mutexattr_init(&ma);

  printf( "========================================================================\n");
  printf( "Non-blocking mutex trylock plus unlock.\n");
  printf( "%ld iterations.\n\n", ITERATIONS);
  printf( "%-40s %15s %15s\n",
	    "Test",
	    "Total(msec)",
	    "average(usec)");
  printf( "------------------------------------------------------------------------\n");

  /*
   * Time the loop overhead so we can subtract it from the actual test times.
   */

  TESTSTART
  TESTSTOP

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop) - overHeadMilliSecs;
  overHeadMilliSecs = durationMilliSecs;

  old_mutex_use = OLD_WIN32CS;
  assert(old_mutex_init(&ox, NULL) == 0);
  TESTSTART
  (void) old_mutex_trylock(&ox);
  (void) old_mutex_unlock(&ox);
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
  (void) old_mutex_trylock(&ox);
  (void) old_mutex_unlock(&ox);
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
