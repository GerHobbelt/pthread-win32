/*
 * File: errn1.c
 *
 * Test Synopsis: Test thread-safety of errno
 * - 
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

/*
 * Create NUMTHREADS threads in addition to the Main thread.
 */
enum {
  NUMTHREADS = 3
};

typedef struct bag_t_ bag_t;
struct bag_t_ {
  int threadnum;
  int started;
  /* Add more per-thread state variables here */
};

static bag_t threadbag[NUMTHREADS + 1];

pthread_mutex_t stop_here = PTHREAD_MUTEX_INITIALIZER;

void *
mythread(void * arg)
{
  bag_t * bag = (bag_t *) arg;

  assert(bag == &threadbag[bag->threadnum]);
  assert(bag->started == 0);
  bag->started = 1;

  errno = bag->threadnum;

  Sleep(1000);

  pthread_mutex_lock(&stop_here);

  assert(errno == bag->threadnum);

  pthread_mutex_unlock(&stop_here);

  Sleep(1000);

  return 0;
}

int
main()
{
  int failed = 0;
  int i;
  pthread_t t[NUMTHREADS + 1];

  pthread_mutex_lock(&stop_here);
  errno = 0;

  assert((t[0] = pthread_self()) != NULL);

  for (i = 1; i <= NUMTHREADS; i++)
    {
      threadbag[i].started = 0;
      threadbag[i].threadnum = i;
      assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
    }

  /*
   * Code to control or munipulate child threads should probably go here.
   */
  Sleep(2000);
  pthread_mutex_unlock(&stop_here);

  /*
   * Give threads time to run.
   */
  Sleep(NUMTHREADS * 1000);

  /*
   * Standard check that all threads started.
   */
  for (i = 1; i <= NUMTHREADS; i++)
    { 
      failed = !threadbag[i].started;

      if (failed)
	{
	  fprintf(stderr, "Thread %d: started %d\n", i, threadbag[i].started);
	}
    }

  assert(!failed);

  /*
   * Check any results here. Set "failed" and only print ouput on failure.
   */
  for (i = 1; i <= NUMTHREADS; i++)
    { 
      /* ... */
    }

  assert(!failed);

  /*
   * Success.
   */
  return 0;
}
