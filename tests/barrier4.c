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
static int result1 = -1;
static int result2 = -1;
static int serialThreadCount = 0;
static int otherThreadCount = 0;

void * func(void * arg)
{
  int result = pthread_barrier_wait(&barrier);

  assert(pthread_mutex_lock(&mx) == 0);
  if (result == PTHREAD_BARRIER_SERIAL_THREAD)
    {
      serialThreadCount++;
    }
  else
    {
      otherThreadCount++;
    }
  assert(pthread_mutex_lock(&mx) == 0);

  return NULL;
}
 
int
main()
{
  pthread_t t[NUMTHREADS + 1];

  assert(pthread_barrier_init(&barrier, NULL, NUMTHREADS) == 0);

  for (i = 0; i < NUMTHREADS; i++)
    {
      assert(pthread_create(&t[i], NULL, func, NULL) == 0);
    }

  for (i = 0; i < NUMTHREADS; i++)
    {
      assert(pthread_join(t[i], NULL) == 0);
    }

  assert(serialThreadCount == 1);

  assert(pthread_barrier_destroy(&barrier) == 0);

  assert(pthread_mutex_destroy(&mx) == 0);

  return 0;
}
