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


#if defined(_MSC_VER) && defined(__cplusplus)
#include <eh.h>
#else
#include <new.h>
#endif

#if defined(_MSC_VER) || defined(__cplusplus)

#include "test.h"

/*
 * Create NUMTHREADS threads in addition to the Main thread.
 */
enum {
  NUMTHREADS = 20
};

int caught = 0;
pthread_mutex_t caughtLock = PTHREAD_MUTEX_INITIALIZER;

#if defined(_MSC_VER) && !defined(__cplusplus)

LONG unhandledExceptionFilter (EXCEPTION_POINTERS *ep)
{
  if (ep->ExceptionRecord->ExceptionCode == 0x1)
    {
      pthread_mutex_lock(&caughtLock);
      caught++;
      pthread_mutex_unlock(&caughtLock);
    }

  return EXCEPTION_CONTINUE_EXECUTION;
}

#elif defined(__cplusplus)

void
terminateFunction ()
{
  pthread_mutex_lock(&caughtLock);
  caught++;
#if 1
  {
     FILE * fp = fopen("pthread.log", "a");
     fprintf(fp, "Caught = %d\n", caught);
     fclose(fp);
  }
#endif
  pthread_mutex_unlock(&caughtLock);
  pthread_exit((void *) 0);
}

#endif

void *
exceptionedThread(void * arg)
{
  int dummy = 0x1;

  {
#if defined(_MSC_VER) && !defined(__cplusplus)

    RaiseException(dummy, 0, 0, NULL);

#elif defined(__cplusplus)

    (void) set_terminate(&terminateFunction);

    throw dummy;

#endif
  }

  return (void *) 100;
}

int
main()
{
  int i;
  pthread_t mt;
  pthread_t et[NUMTHREADS];

  assert((mt = pthread_self()) != NULL);

  {
#if defined(_MSC_VER) && !defined(__cplusplus)
    LPTOP_LEVEL_EXCEPTION_FILTER oldHandler;
    oldHandler = SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER) unhandledExceptionFilter);
#endif

    for (i = 0; i < NUMTHREADS; i++)
      {
        assert(pthread_create(&et[i], NULL, exceptionedThread, NULL) == 0);
      }

#if defined(_MSC_VER) && !defined(__cplusplus)
    (void) SetUnhandledExceptionFilter(oldHandler);
#endif

    Sleep(30000);
  }

  printf("Caught = %d\n", caught);
  assert(caught == NUMTHREADS);

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
