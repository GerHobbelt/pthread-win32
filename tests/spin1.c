/* 
 * spin1.c
 *
 * Create a simple spinlock object, lock it, and then unlock it again.
 * This is the simplest test of the pthread mutex family that we can do.
 *
 */

#include "test.h"

pthread_spinlock_t lock;

int
main()
{
  assert(pthread_spin_init(&lock, PTHREAD_PROCESS_PRIVATE) == 0);

  assert(pthread_spin_lock(&lock) == 0);

  assert(pthread_spin_unlock(&lock) == 0);

  assert(pthread_spin_destroy(&lock) == 0);

  assert(pthread_spin_lock(&lock) == EINVAL);

  return 0;
}
