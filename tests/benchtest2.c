/*
 * benchtest1.c
 *
 * Measure time taken to complete an elementary operation.
 *
 * - Mutex
 *   Two threads iterate over lock/unlock for each mutex type.
 *   The two threads are forced into lock-step using two mutexes,
 *   forcing the threads to block on each lock operation. The
 *   time measured is therefore the worst case senario.
 */

#include "test.h"
#include <sys/timeb.h>

#ifdef __GNUC__
#include <stdlib.h>
#endif

#define ITERATIONS      100000L

pthread_mutex_t gate1, gate2;
pthread_mutexattr_t ma;
long durationMilliSecs;
long overHeadMilliSecs = 0;
struct _timeb currSysTimeStart;
struct _timeb currSysTimeStop;
pthread_t worker;
int running = 0;

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


void *
overheadThread(void * arg)
{
  do
    {
      sched_yield();
    }
  while (running);

  return NULL;
}


void *
workerThread(void * arg)
{
  do
    {
      (void) pthread_mutex_lock(&gate1);
      (void) pthread_mutex_lock(&gate2);
      (void) pthread_mutex_unlock(&gate1);
      sched_yield();
      (void) pthread_mutex_unlock(&gate2);
    }
  while (running);

  return NULL;
}

void
runTest (char * testNameString, int mType)
{
#ifdef PTHREAD_MUTEX_DEFAULT
  pthread_mutexattr_settype(&ma, mType);
#endif
  pthread_mutex_init(&gate1, &ma);
  pthread_mutex_init(&gate2, &ma);

  (void) pthread_mutex_lock(&gate1);
  (void) pthread_mutex_lock(&gate2);

  running = 1;

  (void) pthread_create(&worker, NULL, workerThread, NULL);

  TESTSTART
  (void) pthread_mutex_unlock(&gate1);
  sched_yield();
  (void) pthread_mutex_unlock(&gate2);
  (void) pthread_mutex_lock(&gate1);
  (void) pthread_mutex_lock(&gate2);
  TESTSTOP

  running = 0;

  (void) pthread_mutex_unlock(&gate2);
  (void) pthread_mutex_unlock(&gate1);

  (void) pthread_join(worker, NULL);

  pthread_mutex_destroy(&gate2);
  pthread_mutex_destroy(&gate1);

  durationMilliSecs = GetDurationMilliSecs(currSysTimeStart, currSysTimeStop)
    - overHeadMilliSecs;

  printf( "%-25s %15ld %15ld %15.3f\n",
          testNameString,
          ITERATIONS,
          durationMilliSecs,
          (float) durationMilliSecs * 1E3 / ITERATIONS / 4   /* Four locks/unlocks per iteration */);
}


int
main (int argc, char *argv[])
{
  pthread_mutexattr_init(&ma);

  printf( "Two threads, blocking mutex locks/unlocks.\n\n");

  printf( "%-25s %15s %15s %15s\n",
          "Test",
          "Iterations",
          "Total(msec)",
          "lock/unlock(usec)");

  /*
   * Time the loop overhead so we can subtract it from the actual test times.
   */

  running = 1;

  (void) pthread_create(&worker, NULL, overheadThread, NULL);

  TESTSTART
  sched_yield();
  TESTSTOP

  running = 0;

  (void) pthread_join(worker, NULL);

  pthread_mutex_destroy(&gate2);
  pthread_mutex_destroy(&gate1);

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
  runTest("Blocking locks", 0);
#endif

  /*
   * End of tests.
   */

  pthread_mutexattr_destroy(&ma);

  return 0;
}
