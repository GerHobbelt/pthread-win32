/*
 * File: exception1.c
 *
 * Test Synopsis: Test passing of exceptions back to the application.
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

#if defined(_MSC_VER) || defined(__cplusplus)

#include "test.h"

/*
 * Create NUMTHREADS threads in addition to the Main thread.
 */
enum {
  NUMTHREADS = 4
};

void *
exceptionedThread(void * arg)
{
  int result = ((int)PTHREAD_CANCELED + 1);
  int one = 1;
  int zero = 0;
  int dummy;

  /* Set to async cancelable */

  assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);

  assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);

  Sleep(100);

#if defined(_MSC_VER)
  __try
  {
    dummy = one/zero;
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    result = ((int)PTHREAD_CANCELED + 2);
  }
#elif defined(__cplusplus)
  try
  {
    dummy = one/zero;
  }
  catch (...)
  {
    result = ((int)PTHREAD_CANCELED + 2);
  }
#endif

  return (void *) result;
}

void *
canceledThread(void * arg)
{
  int result = ((int)PTHREAD_CANCELED + 1);
  int dummy;
  int count;

  /* Set to async cancelable */

  assert(pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL) == 0);

  assert(pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL) == 0);

#if defined(_MSC_VER)
  __try
  {
    /*
     * We wait up to 10 seconds, waking every 0.1 seconds,
     * for a cancelation to be applied to us.
     */
    for (count = 0; count < 100; count++)
      Sleep(100);
  }
  __except (EXCEPTION_EXECUTE_HANDLER)
  {
    result = ((int)PTHREAD_CANCELED + 2);
  }
#elif defined(__cplusplus)
  try
  {
    /*
     * We wait up to 10 seconds, waking every 0.1 seconds,
     * for a cancelation to be applied to us.
     */
    for (count = 0; count < 100; count++)
      Sleep(100);
  }
  catch (...)
  {
    result = ((int)PTHREAD_CANCELED + 2);
  }
#endif

  return (void *) result;
}

int
main()
{
  int failed = 0;
  int i;
  pthread_t mt;
  pthread_t et[NUMTHREADS];
  pthread_t ct[NUMTHREADS];

  assert((mt = pthread_self()) != NULL);

  for (i = 0; i < NUMTHREADS; i++)
    {
      assert(pthread_create(&et[i], NULL, exceptionedThread, NULL) == 0);
      assert(pthread_create(&ct[i], NULL, canceledThread, NULL) == 0);
    }

  /*
   * Code to control or munipulate child threads should probably go here.
   */
  Sleep(500);

  for (i = 0; i < NUMTHREADS; i++)
    {
      assert(pthread_cancel(ct[i]) == 0);
    }

  /*
   * Give threads time to run.
   */
  Sleep(NUMTHREADS * 100);

  /*
   * Check any results here. Set "failed" and only print output on failure.
   */
  failed = 0;
  for (i = 0; i < NUMTHREADS; i++)
    {
      int fail = 0;
      int result = 0;

	/* Canceled thread */
      assert(pthread_join(ct[i], (void *) &result) == 0);
      fail = (result != (int) PTHREAD_CANCELED);

      failed = (failed || fail);

      /* Exception thread */
      assert(pthread_join(et[i], (void *) &result) == 0);
      fail = (result != ((int) PTHREAD_CANCELED + 2));

      failed = (failed || fail);
    }

  assert(!failed);

  /*
   * Success.
   */
  return 0;
}

#else /* defined(_MSC_VER) || defined(__cplusplus) */

int
main()
{
  fprintf(stderr, "Test N/A for this compiler environment.\n");
  return 0;
}

#endif /* defined(_MSC_VER) || defined(__cplusplus) */
