/* 
 * barrier2.c
 *
 * Declare a single barrier object, wait on it, 
 * and then destroy it.
 *
 */

#include "test.h"
 
pthread_barrier_t barrier = NULL;

int
main()
{
  assert(pthread_barrier_init(&barrier, NULL, 1) == 0);

  assert(pthread_barrier_wait(&barrier) == PTHREAD_BARRIER_SERIAL_THREAD);

  assert(pthread_barrier_destroy(&barrier) == 0);

  return 0;
}
