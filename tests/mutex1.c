/* 
 * mutex1.c
 *
 * Create a simple mutex object, lock it, and then unlock it again.
 * This is the simplest test of the pthread mutex family that we can do.
 */

#include <pthread.h>

pthread_mutex_t mutex1;

main()
{
  pthread_mutex_init(&mutex1, NULL);
  pthread_mutex_lock(&mutex1);
  pthread_mutex_unlock(&mutex1);
}
