/*
 * File: exception3.c
 *
 * Test Synopsis: Test running of user supplied teerminate() function.
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
 * - have working pthread_create, pthread_self, pthread_mutex_lock/unlock
 *   pthread_testcancel, pthread_cancel, pthread_join
 *
 * Pass Criteria:
 * - Process returns zero exit status.
 *
 * Fail Criteria:
 * - Process returns non-zero exit status.
 */

#include "test.h"

#if defined(__cplusplus)

#if defined(_MSC_VER)
#include <eh.h>
#else
#include <new.h>
#endif

/*
 * Create NUMTHREADS threads in addition to the Main thread.
 */
enum {
  NUMTHREADS = 20
};

int caught = 0;
pthread_mutex_t caughtLock = PTHREAD_MUTEX_INITIALIZER;

void
terminateFunction ()
{
  pthread_mutex_lock(&caughtLock);
  caught++;
#if 0
  {
     FILE * fp = fopen("pthread.log", "a");
     fprintf(fp, "Caught = %d\n", caught);
     fclose(fp);
  }
#endif
  pthread_mutex_unlock(&caughtLock);
  pthread_exit((void *) 0);
}

void *
exceptionedThread(void * arg)
{
  int dummy = 0x1;

  (void) set_terminate(&terminateFunction);
  throw dummy;

  return (void *) 0;
}

int
main()
{
  int i;
  pthread_t mt;
  pthread_t et[NUMTHREADS];

  assert((mt = pthread_self()) != NULL);

  for (i = 0; i < NUMTHREADS; i++)
    {
      assert(pthread_create(&et[i], NULL, exceptionedThread, NULL) == 0);
    }

  Sleep(10000);

  assert(caught == NUMTHREADS);

  /*
   * Success.
   */
  return 0;
}

#else /* defined(__cplusplus) */

int
main()
{
  fprintf(stderr, "Test N/A for this compiler environment.\n");
  return 0;
}

#endif /* defined(__cplusplus) */
