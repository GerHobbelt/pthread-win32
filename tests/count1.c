/*
 * count1.c
 *
 * Written by Ben Elliston <bje@cygnus.com>.
 *
 * Description:
 * Test some basic assertions about the number of threads at runtime.
 */

#include "test.h"

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
  int i;
  int maxThreads = sizeof(threads) / sizeof(pthread_t);

  /*
   * Spawn ten threads. Each thread should increment the numThreads
   * variable, sleep for one second, decrement the variable and then
   * exit. The final result of numThreads should be 1 again.
   */
  for (i = 0; i < maxThreads; i++)
    {
      assert(pthread_create(&threads[i], NULL, myfunc, 0) == 0);
    }
  
  /*
   * Wait for all the threads to exit.
   */
  for (i = 0; i < maxThreads; i++)
    {
      assert(pthread_join(threads[i], NULL) == 0);
      assert(pthread_mutex_lock(&lock) == 0);
      numThreads--;
      assert(pthread_mutex_unlock(&lock) == 0);
    }

  /* 
   * Check the number of live threads.
   */
  assert(numThreads == 1);
  
  /*
   * Success.
   */
  return 0;
}
