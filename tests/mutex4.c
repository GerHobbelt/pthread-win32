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
 
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t locker_done = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t unlocker_done = PTHREAD_MUTEX_INITIALIZER;

static int washere = 0;

void * locker(void * arg)
{
  assert(pthread_mutex_lock(&locker_start) == 0);
  assert(pthread_mutex_lock(&mutex1) == 0);
  assert(pthread_mutex_unlock(&locker_start) == 0);

  /* Wait for unlocker to finish */
  assert(pthread_mutex_lock(&unlocker_end) == 0);
  assert(pthread_mutex_unlock(&mutex1) == 0);

  return 0;
}
 
void * unlocker(void * arg)
{
  /* Wait for locker to lock mutex1 */
  assert(pthread_mutex_lock(&unlocker_start) == 0);

  assert(pthread_mutex_unlock(&mutex1) == EPERM);

  assert(pthread_mutex_unlock(&unlocker_start) == 0);

  return 0;
}
 
int
main()
{
  pthread_t t;

  assert(pthread_mutex_lock(&locker_start) == 0);
  assert(pthread_mutex_lock(&unlocker_start) == 0);

  assert(pthread_create(&t, NULL, locker, NULL) == 0);
  assert(pthread_mutex_unlock(&locker_start) == 0);
  Sleep(0);

  assert(pthread_create(&t, NULL, unlocker, NULL) == 0);
  assert(pthread_mutex_unlock(&unlocker_start) == 0);
  Sleep(0);

  return 0;
}
