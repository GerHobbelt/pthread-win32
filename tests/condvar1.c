/*
 * File: condvar1.c
 *
 * Test Synopsis:
 * - Test basic function of condition variable code.
 *
 * Test Method (Validation or Falsification):
 * - Validation
 *
 * Requirements Tested:
 * - 
 *
 * Features Tested:
 * - 
 *
 * Cases Tested:
 * - 
 *
 * Description:
 * - 
 *
 * Environment:
 * - 
 *
 * Input:
 * - 
 *
 * Output:
 * - 
 *
 * Assumptions:
 * - 
 *
 * Pass Criteria:
 * - 
 *
 * Fail Criteria:
 * - 
 */

#include "test.h"

typedef struct cvthing_t_ cvthing_t;

struct cvthing_t_ {
  pthread_cond_t notbusy;
  pthread_mutex_t lock;
  int busy;
  int count;
};

static cvthing_t cvthing;

static enum {
  NUMTHREADS = 10
};

static pthread_key_t key;

void *
mythread(void * arg)
{
  assert(pthread_mutex_lock(&cvthing.lock) == 0);

  cvthing.count++;

  while (cvthing.busy)
    {
      assert(pthread_cond_wait(&cvthing.notbusy, &cvthing.lock) == 0);
    }

  assert(cvthing.busy == 0);

  cvthing.count--;

  assert(pthread_mutex_unlock(&cvthing.lock) == 0);

  return 0;
}

int
main()
{
  pthread_t t[NUMTHREADS];
  int result[NUMTHREADS];
  
  assert((t[0] = pthread_self()) != NULL);

  assert(pthread_cond_init(&cvthing, NULL) == 0);

  for (i = 1; i < NUMTHREADS; i++)
    {
      assert(pthread_create(&t[i], NULL, mythread, (void *) i) == 0);
    }

  while (cvthing.count < NUMTHREADS)
    {}

  assert(pthread_mutex_lock(&cvthing.lock) == 0);
  cvthing.busy = 0;
  assert(pthread_cond_signal(&cvthing.notbusy) == 0);
  assert(pthread_mutex_unlock(&cvthing.lock) == 0);

  for (i = 1; i < NUMTHREADS; i++)
    {
      assert(pthread_join(t[i], (void *) &result[i]) == 0);
    }

  assert(cvthing.count == 0);

  assert(pthread_cond_destroy(&cvthing) == 0);

  return 0;
}







