#include <pthread.h>
 
pthread_mutex_t mutex1;
 
main()
{
  pthread_mutex_init(&mutex1, NULL);
  pthread_mutex_trylock(&mutex1);
  pthread_mutex_unlock(&mutex1);

  return 0;
}
