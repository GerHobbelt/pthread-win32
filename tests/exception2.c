/*
 * File: exception2.c
 *
 * Test Synopsis: Test passing of exceptions out of thread scope.
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

#if defined(_MSC_VER) && defined(__cplusplus)
#include <eh.h>
#else
#include <new.h>
#endif

#ifdef __GNUC__
#include <stdlib.h>
#endif

#include "test.h"

/*
 * Create NUMTHREADS threads in addition to the Main thread.
 */
enum {
  NUMTHREADS = 1
};


void *
exceptionedThread(void * arg)
{
  int dummy = 0x1;

#if defined(_MSC_VER) && !defined(__cplusplus)

  RaiseException(dummy, 0, 0, NULL);

#elif defined(__cplusplus)

  throw dummy;

#endif

  return (void *) 100;
}

int
main(int argc, char argv[])
{
  int i;
  pthread_t mt;
  pthread_t et[NUMTHREADS];

  if (argc <= 1)
    {
      int result;

      printf("You should see an \"abnormal termination\" message\n");
      fflush(stdout);
      result = system("exception2.exe die");
      exit(0);
    }

  assert((mt = pthread_self()) != NULL);

  for (i = 0; i < NUMTHREADS; i++)
    {
      assert(pthread_create(&et[i], NULL, exceptionedThread, NULL) == 0);
    }

  Sleep(1000);

  /*
   * Success.
   */
  return 0;
}

#else /* defined(_MSC_VER) || defined(__cplusplus) */

#include <stdio.h>

int
main()
{
  fprintf(stderr, "Test N/A for this compiler environment.\n");
  return 0;
}

#endif /* defined(_MSC_VER) || defined(__cplusplus) */
