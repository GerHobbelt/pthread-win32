/*
 * File: 
 *
 * Test Synopsis:
 * - 
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
 * - 
 *
 * Output:
 * - 
 *
 * Assumptions:
 * - 
 *
 * Pass Criteria:
 * - 
 *
 * Fail Criteria:
 * - 
 */

#include "test.h"

/*
 * Create NUMTHREADS threads in addition to the Main thread.
 */
static enum {
  NUMTHREADS = 2
};

typedef struct bag_t_ bag_t;
struct bag_t_ {
  int threadnum;
  int washere;
  /* Add more pre-thread state variables here */
};

static bag_t threadbag[NUMTHREADS];

void *
mythread(void * arg)
{
  bag_t * bag = (bag_t *) arg;

  assert(bag == &threadbag[bag->threadnum]);
  assert(bag->washere == 0);
  bag->washere = 1;

  /* ... */

  return 0;
}

int
main()
{
  int failed = 0;
  pthread_t t[NUMTHREADS + 1];

  assert((t[0] = pthread_self()) != NULL);

  for (i = 1; i <= NUMTHREADS; i++)
    { 
      threadbag[i].washere = 0;
      threadbag[i].threadnum = i;
      assert(pthread_create(&t[i], NULL, mythread, (void *) &threadbag[i]) == 0);
    }

  /*
   * Give threads time to run.
   */
  Sleep(NUMTHREADS * 1000);

  /*
   * Standard check that all threads started.
   */
  for (i = 1; i <= NUMTHREADS; i++)
    { 
      if (threadbag[i].washere != 1)
	{
	  failed = 1;
	  fprintf(stderr, "Thread %d: washere %d\n", i, threadbag[i].washere);
	}
    }

  assert(failed == 0);

  /*
   * Check any results here. Only print ouput and set "failed" on failure.
   */
  for (i = 1; i <= NUMTHREADS; i++)
    { 
      /* ... */
    }

  assert(failed == 0);

  /*
   * Success.
   */
  return 0;
}
