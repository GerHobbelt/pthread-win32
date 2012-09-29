#include <pthread.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int
main()
{
  pthread_mutex_lock(&mutex);
  /* do stuff */
  pthread_mutex_unlock(&mutex);
  return 0;
}
