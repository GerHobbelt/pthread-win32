/* 
 * spin3.c
 *
 * Thread A locks spin - thread B tries to unlock.
 * This should succeed, but it's undefined behaviour.
 *
 */

#include "test.h"

static int wasHere = 0;

static pthread_spinlock_t spin;
 
void * unlocker(void * arg)
{
  int expectedResult = (int) arg;

  wasHere++;
  assert(pthread_spin_unlock(&spin) == expectedResult);
  wasHere++;
  return NULL;
}
 
int
main()
{
  pthread_t t;
  pthread_spinattr_t ma;

  wasHere = 0;
  assert(pthread_spin_init(&spin, PTHREAD_PROCESS_PRIVATE) == 0);
  assert(pthread_spin_lock(&spin) == 0);
  assert(pthread_create(&t, NULL, unlocker, (void *) 0) == 0);
  assert(pthread_join(t, NULL) == 0);
  assert(pthread_spin_unlock(&spin) == EPERM);
  assert(wasHere == 2);

  return 0;
}
