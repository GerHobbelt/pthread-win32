/* 
 * mutex4.c
 *
 * Thread A locks mutex - thread B tries to unlock.
 *
 * Depends on API functions: 
 *	pthread_mutex_lock()
 *	pthread_mutex_trylock()
 *	pthread_mutex_unlock()
 */

#include "test.h"

static int wasHere = 0;

static pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

void * locker(void * arg)
{
  wasHere++;
  assert(pthread_mutex_lock(&mutex1) == 0);
  Sleep(1000);
  assert(pthread_mutex_unlock(&mutex1) == 0);

  wasHere++;
  return 0;
}
 
void * unlocker(void * arg)
{
  wasHere++;

  /* Wait for locker to lock mutex1 */
  Sleep(500);

  assert(pthread_mutex_unlock(&mutex1) == EPERM);

  wasHere++;
  return 0;
}
 
int
main()
{
  pthread_t t;

  assert(pthread_create(&t, NULL, locker, NULL) == 0);
  assert(pthread_create(&t, NULL, unlocker, NULL) == 0);
  Sleep(2000);

  assert(wasHere == 4);

  return 0;
}
