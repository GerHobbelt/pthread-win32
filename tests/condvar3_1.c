/*
 * File: condvar3_1.c
 *
 * Test Synopsis:
 * - Test timeout of multiple waits on a CV with some signaled.
 *
 * Test Method (Validation or Falsification):
 * - Validation
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
 * - Because some CVs are never signaled, we expect their waits to time out.
 *   Some are signaled, the rest time out. Pthread_cond_destroy() will fail
 *   unless all are accounted for, either signaled or timedout.
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
 * - pthread_cond_timedwait returns ETIMEDOUT.
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - pthread_cond_timedwait does not return ETIMEDOUT.
 * - Process returns non-zero exit status.
 */

#include "test.h"
#include <sys/timeb.h>

static pthread_cond_t cv;
static pthread_mutex_t mutex;
static struct timespec abstime = { 0, 0 };
static int timedout = 0;
static int signaled = 0;
static int awoken = 0;

enum {
  NUMTHREADS = 60
};

void *
mythread(void * arg)
{
  int result;

  assert(pthread_mutex_lock(&mutex) == 0);

  result = pthread_cond_timedwait(&cv, &mutex, &abstime);
  if (result == ETIMEDOUT)
    {
      timedout++;
    }
  else
    {
      awoken++;
    }

  assert(pthread_mutex_unlock(&mutex) == 0);

  return arg;
}

#include "../implement.h"

int
main()
{
  int i;
  pthread_t t[NUMTHREADS + 1];
  int result = 0;
  struct _timeb currSysTime;
  const DWORD NANOSEC_PER_MILLISEC = 1000000;

  assert(pthread_cond_init(&cv, NULL) == 0);

  assert(pthread_mutex_init(&mutex, NULL) == 0);

  /* get current system time */
  _ftime(&currSysTime);

  abstime.tv_sec = currSysTime.time;
  abstime.tv_nsec = NANOSEC_PER_MILLISEC * currSysTime.millitm;

  abstime.tv_sec += 5;

  assert(pthread_mutex_lock(&mutex) == 0);

  for (i = 1; i <= NUMTHREADS; i++)
    {
      assert(pthread_create(&t[i], NULL, mythread, (void *) i) == 0);
    }

  assert(pthread_mutex_unlock(&mutex) == 0);

  for (i = NUMTHREADS/3; i <= 2*NUMTHREADS/3; i++)
    {
      assert(pthread_cond_signal(&cv) == 0);
      signaled++;
    }

  for (i = 1; i <= NUMTHREADS; i++)
    {
      assert(pthread_join(t[i], (void **) &result) == 0);
	assert(result == i);
    }

  assert(signaled == awoken);
  assert(timedout == NUMTHREADS - signaled);

  {
  int result = pthread_cond_destroy(&cv);
  if (result != 0)
    {
      fprintf(stderr, "Result = %s\n", error_string[result]);
	fprintf(stderr, "\tWaitersBlocked = %ld\n", cv->nWaitersBlocked);
	fprintf(stderr, "\tWaitersUnblocked = %ld\n", cv->nWaitersUnblocked);
	fprintf(stderr, "\tWaitersGone = %ld\n", cv->nWaitersGone);
	fprintf(stderr, "\tWaitersToUnblock = %ld\n", cv->nWaitersToUnblock);
	fflush(stderr);
    }
  assert(result == 0);
  }

  return 0;
}
