/*
 * File: 
 *
 * Test Synopsis:
 * - 
 *
 * Test Method (Validation or Falsification):
 * - 
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

#include <pthread.h>
#include <stdio.h>

pthread_key_t key;
pthread_once_t key_once = PTHREAD_ONCE_INIT;

void *
mythread(void * arg)
{
  return 0;
}

int
main()
{
  int rc;
  pthread_t t1, t2;
  
  rc = pthread_create(&t1, NULL, mythread, (void *) 1);
  printf("pthread_create returned %d\n", rc);

  rc = pthread_create(&t2, NULL, mythread, (void *) 2);
  printf("pthread_create returned %d\n", rc);

  /* Give threads time to run. */
  Sleep(2000);
  return 0;
}
