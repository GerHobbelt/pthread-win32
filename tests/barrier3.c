/*
 * barrier3.c
 *
 * Declare a single barrier object, multiple wait on it, 
 * and then destroy it.
 *
 */

#include "test.h"
 
pthread_barrier_t barrier = NULL;
static int result1 = -1;
static int result2 = -1;

void * func(void * arg)
{
  return (void *) pthread_barrier_wait(&barrier);
}
 
int
main()
{
  pthread_t t;

  assert(pthread_barrier_init(&barrier, NULL, 2) == 0);

  assert(pthread_create(&t, NULL, func, NULL) == 0);

  result1 = pthread_barrier_wait(&barrier);

  assert(pthread_join(t, &result2) == 0);

  assert(result1 != result2);
  assert(result1 == 0 || result1 == PTHREAD_BARRIER_SERIAL_THREAD);
  assert(result2 == 0 || result2 == PTHREAD_BARRIER_SERIAL_THREAD);

  assert(pthread_barrier_destroy(&barrier) == 0);

  return 0;
}
