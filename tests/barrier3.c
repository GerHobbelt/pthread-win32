/*
 * barrier3.c
 *
 * Declare a single barrier object with barrier attribute, wait on it, 
 * and then destroy it.
 *
 */

#include "test.h"
 
pthread_barrier_t barrier = NULL;
static int result = 1;

void * func(void * arg)
{
  return (void *) pthread_barrier_wait(&barrier);
}
 
int
main()
{
  pthread_t t;
  pthread_barrierattr_t ba;

  assert(pthread_barrierattr_init(&ba) == 0);
  assert(pthread_barrierattr_setpshared(&ba, PTHREAD_PROCESS_PRIVATE) == 0);
  assert(pthread_barrier_init(&barrier, &ba, 1) == 0);

  assert(pthread_create(&t, NULL, func, NULL) == 0);

  assert(pthread_join(t, (void **) &result) == 0);

  assert(result == PTHREAD_BARRIER_SERIAL_THREAD);

  assert(pthread_barrier_destroy(&barrier) == 0);
  assert(pthread_barrierattr_destroy(&ba) == 0);

  return 0;
}
