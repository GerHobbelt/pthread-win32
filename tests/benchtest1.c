/*
 * benchtest1.c
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
#ifdef PTHREAD_MUTEX_DEFAULT
  pthread_mutexattr_settype(&ma, mType);
#endif
  pthread_mutex_init(&mx, &ma);

  TESTSTART
  (void) pthread_mutex_lock(&mx);
  (void) pthread_mutex_unlock(&mx);
  TESTSTOP

  pthread_mutex_destroy(&mx);

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop)
    - overHeadMilliSecs;

  printf( "%-25s %15ld %15ld %15.3f\n",
          testNameString,
          ITERATIONS,
          durationMilliSecs,
          (float) durationMilliSecs * 1E3 / ITERATIONS);
}


int
main (int argc, char *argv[])
{
  pthread_mutexattr_init(&ma);

  printf( "Single thread, non-blocking mutex locks/unlocks.\n\n");
  printf( "%-25s %15s %15s %15s\n",
          "Test",
          "Iterations",
          "Total(msec)",
          "lock/unlock(usec)");

  /*
   * Time the loop overhead so we can subtract it from the actual test times.
   */

  TESTSTART
  TESTSTOP

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop)
    - overHeadMilliSecs;

  printf( "%-25s %15ld %15ld\n",
          "Overhead",
          ITERATIONS,
          durationMilliSecs);

  overHeadMilliSecs = durationMilliSecs;

  /*
   * Now we can start the actual tests
   */
#ifdef PTHREAD_MUTEX_DEFAULT
  runTest("PTHREAD_MUTEX_DEFAULT", PTHREAD_MUTEX_DEFAULT);

  runTest("PTHREAD_MUTEX_NORMAL", PTHREAD_MUTEX_NORMAL);

  runTest("PTHREAD_MUTEX_ERRORCHECK", PTHREAD_MUTEX_ERRORCHECK);

  runTest("PTHREAD_MUTEX_RECURSIVE", PTHREAD_MUTEX_RECURSIVE);
#else
  runTest("Non-blocking lock", 0);
#endif

  /*
   * End of tests.
   */

  pthread_mutexattr_destroy(&ma);

  return 0;
}
