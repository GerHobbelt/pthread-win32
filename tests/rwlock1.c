/* 
 * rwlock1.c
 *
 * Create a simple rwlock object, lock it, and then unlock it again.
 *
 * Depends on API functions:
 * 	pthread_rwlock_init()
 *	pthread_rwlock_lock()
 *	pthread_rwlock_unlock()
 *	pthread_rwlock_destroy()
 */

#include "test.h"

pthread_rwlock_t rwlock = NULL;

int
main()
{
  assert(rwlock == NULL);

  assert(pthread_rwlock_init(&rwlock, NULL) == 0);

  assert(rwlock != NULL);

  assert(pthread_rwlock_destroy(&rwlock) == 0);

  assert(rwlock == NULL);

  return 0;
}
