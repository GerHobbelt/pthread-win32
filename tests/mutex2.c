/* 
 * mutex1.c
 *
 * Declare a static mutex object, lock it, and then unlock it again.
 */

#include <pthread.h>
#include "test.h"
 
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

main()
{
  int result;

  result = pthread_mutex_trylock(&mutex1);
  printf("pthread_mutex_trylock returned %s\n", error_string[result]);

  if (result == 0)
  {
    pthread_mutex_unlock(&mutex1);
  }

  return 0;
}
