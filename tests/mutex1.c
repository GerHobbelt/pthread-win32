/* 
 * mutex1.c
 *
 * Create a simple mutex object, lock it, and then unlock it again.
 * This is the simplest test of the pthread mutex family that we can do.
 *
 * Depends on API functions:
 * 	pthread_mutex_init()
 *	pthread_mutex_lock()
 *	pthread_mutex_unlock()
 *	pthread_mutex_destroy()
 */

#include "test.h"

pthread_mutex_t mutex1;

int
main()
{
  assert(pthread_mutex_init(&mutex1, NULL) == 0);

  assert(pthread_mutex_lock(&mutex1) == 0);

  assert(pthread_mutex_unlock(&mutex1) == 0);

  assert(pthread_mutex_destroy(&mutex1) == 0);

  return 0;
}
