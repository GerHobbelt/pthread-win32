/* 
 * spin2.c
 *
 * Declare a spinlock object, lock it, trylock it, 
 * and then unlock it again.
 *
 */

#include "test.h"
 
pthread_spinlock_t lock = NULL;

static int washere = 0;

void * func(void * arg)
{
  assert(pthread_spin_trylock(&lock) == EBUSY);

  washere = 1;

  return 0; 
}
 
int
main()
{
  pthread_t t;

  assert(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE) == 0);

  assert(pthread_spin_lock(&lock) == 0);

  assert(pthread_create(&t, NULL, func, NULL) == 0);
  assert(pthread_join(t, NULL) == 0);

  assert(pthread_spin_unlock(&lock) == 0);

  assert(pthread_spin_destroy(&lock) == 0);

  assert(washere == 1);

  return 0;
}
