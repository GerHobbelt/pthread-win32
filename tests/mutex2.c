/* 
 * mutex2.c
 *
 * Declare a static mutex object, lock it, 
 * and then unlock it again.
 *
 * Depends on API functions: 
 *	pthread_mutex_lock()
 *	pthread_mutex_unlock()
 */

#include "test.h"
 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int
main()
{
  assert(pthread_mutex_lock(&mutex) == 0);

  assert(pthread_mutex_unlock(&mutex) == 0);

  return 0;
}
