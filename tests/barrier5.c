/*
 * barrier5.c
 *
 * Declare a single barrier object, set up a sequence of
 * barrier points to prove lockstepness, and then destroy it.
 *
 */

#include "test.h"

enum {
  NUMTHREADS = 16,
  ITERATIONS = 10000
};
 
pthread_barrier_t barrier = NULL;
pthread_mutex_t mx = PTHREAD_MUTEX_INITIALIZER;

int barrierReleases[ITERATIONS + 1];

void *
func(void * barrierHeight)
{
  int i;
  int result;

  for (i = 1; i < ITERATIONS; i++)
    {
      result = pthread_barrier_wait(&barrier);

      assert(pthread_mutex_lock(&mx) == 0);
      barrierReleases[i]++;
      assert(pthread_mutex_unlock(&mx) == 0);
      /*
       * Confirm the correct number of releases from the previous
       * barrier. We can't do the current barrier yet because there may
       * still be threads waking up.
       */
      if (result == PTHREAD_BARRIER_SERIAL_THREAD)
        {
          assert(pthread_mutex_lock(&mx) == 0);
//printf("Releases bucket %d = %d\n", i - 1, barrierReleases[i - 1]);
//fflush(stdout);
          assert(barrierReleases[i - 1] == (int) barrierHeight);
          barrierReleases[i + 1] = 0;
          assert(pthread_mutex_unlock(&mx) == 0);
        }
      else if (result != 0)
        {
          printf("Barrier failed: result = %s\n", error_string[result]);
          fflush(stdout);
          return NULL;
        }
    }

  return NULL;
}

int
main()
{
  int i, j;
  pthread_t t[NUMTHREADS + 1];

  for (j = 1; j <= NUMTHREADS; j++)
    {
      printf("Barrier height = %d\n", j);

      barrierReleases[0] = j;
      barrierReleases[1] = 0;

      assert(pthread_barrier_init(&barrier, NULL, j) == 0);

      for (i = 1; i <= j; i++)
        {
          assert(pthread_create(&t[i], NULL, func, (void *) j) == 0);
        }

      for (i = 1; i <= j; i++)
        {
          assert(pthread_join(t[i], NULL) == 0);
        }

      assert(barrierReleases[ITERATIONS - 1] == j);
      assert(barrierReleases[ITERATIONS] == 0);

      assert(pthread_barrier_destroy(&barrier) == 0);
    }

  assert(pthread_mutex_destroy(&mx) == 0);

  return 0;
}
