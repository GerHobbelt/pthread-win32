/* 
 * barrier1.c
 *
 * Create a barrier object and then destroy it.
 *
 */

#include "test.h"

pthread_barrier_t barrier = NULL;

int
main()
{
  assert(barrier == NULL);

  assert(pthread_barrier_init(&barrier, NULL, 1) == 0);

  assert(barrier != NULL);

  assert(pthread_barrier_destroy(&barrier) == 0);

  assert(barrier == NULL);

  return 0;
}
