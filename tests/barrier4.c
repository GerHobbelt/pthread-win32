/*
 * barrier4.c
 *
 * Declare a single barrier object, multiple wait on it, 
 * and then destroy it.
 *
 */

#include "test.h"

enum {
  NUMTHREADS = 16
};
 
pthread_barrier_t barrier = NULL;
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;
static int serialThreadCount = 0;
static int otherThreadCount = 0;

void *
func(void * arg)
{
  int result = pthread_barrier_wait(&barrier);

  assert(pthread_mutex_lock(&mx) == 0);

//  printf("Barrier wait returned %d [%d]\n", result, WAIT_FAILED);
//  fflush(stdout);

  if (result == PTHREAD_BARRIER_SERIAL_THREAD)
    {
      serialThreadCount++;
    }
  else if (0 == result)
    {
      otherThreadCount++;
    }
  else
    {
      printf("Barrier wait failed: error = %s\n", error_string[result]);
      fflush(stdout);
      return NULL;
    }
  assert(pthread_mutex_unlock(&mx) == 0);

  return NULL;
}

int
main()
{
  int i, j;
  pthread_t t[NUMTHREADS + 1];

  for (j = 1; j <= NUMTHREADS; j++)
    {
      printf("Barrier height = %d\n", j);

      serialThreadCount = 0;

      assert(pthread_barrier_init(&barrier, NULL, j) == 0);

      for (i = 1; i <= j; i++)
        {
          assert(pthread_create(&t[i], NULL, func, NULL) == 0);
        }

      for (i = 1; i <= j; i++)
        {
          assert(pthread_join(t[i], NULL) == 0);
        }

      assert(serialThreadCount == 1);

      assert(pthread_barrier_destroy(&barrier) == 0);
    }

  assert(pthread_mutex_destroy(&mx) == 0);

  return 0;
}
