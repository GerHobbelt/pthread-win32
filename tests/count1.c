/*
 * count1.c
 *
 * Written by Ben Elliston <bje@cygnus.com>.
 *
 * Description:
 * Test some basic assertions about the number of threads at runtime.
 */

#include <windows.h>
#include <pthread.h>

static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t threads[10];
static unsigned numThreads = 1;

void *
myfunc(void *arg)
{
  pthread_mutex_lock(&lock);
  numThreads++;
  pthread_mutex_unlock(&lock);

  Sleep(1000);
  return 0;
}
int
main()
{
  int i, result;
  int maxThreads = sizeof(threads) / sizeof(pthread_t);

  /* Spawn ten threads. Each thread should increment the numThreads
     variable, sleep for one second, decrement the variable and then
     exit. The final result of numThreads should be 1 again. */
  for (i = 0; i < maxThreads; i++)
    {
      result = pthread_create(&threads[i], NULL, myfunc, 0);
      if (result != 0)
	{
	  return 1;
	}
    }
  
  /* Wait for all the threads to exit. */
  for (i = 0; i < maxThreads; i++)
    {
      pthread_join(threads[i], NULL);
      pthread_mutex_lock(&lock);
      numThreads--;
      pthread_mutex_unlock(&lock);
    }

  /* Check the number of live threads. */
  if (numThreads != 1)
    {
      return 1;
    }
  
  /* Success. */
  return 0;
}
