/*
 * File: tsd1.c
 *
 * Test Synopsis:
 * - Thread Specific Data (TSD) key creation and destruction.
 *
 * Description:
 * - 
 *
 * Test Method (validation or falsification):
 * - validation
 *
 * Requirements Tested:
 * - keys are created for each existing thread including the main thread
 * - keys are created for newly created threads
 * - keys are thread specific
 * - destroy routine is called on each thread exit including the main thread
 *
 * Features Tested:
 * - 
 *
 * Cases Tested:
 * - 
 *
 * Environment:
 * - 
 *
 * Input:
 * - none
 *
 * Output:
 * - text to stdout
 *
 * Assumptions:
 * - already validated:     pthread_create()
 *                          pthread_once()
 * - main thread also has a POSIX thread identity
 *
 * Pass Criteria:
 * - stdout matches file reference/tsd1.out
 *
 * Fail Criteria:
 * - fails to match file reference/tsd1.out
 * - output identifies failed component
 */

#include <pthread.h>
#include <stdio.h>

pthread_key_t key = NULL;
pthread_once_t key_once = PTHREAD_ONCE_INIT;

void
destroy_key(void * arg)
{
  /* arg is not NULL if we get to here. */
  printf("SUCCESS: %s: destroying key.\n", (char *) arg);

  free((char *) arg);
}

void
make_key(void)
{
  if (pthread_key_create(&key, destroy_key) != 0)
    {
      printf("Key create failed\n");
      exit(1);
    }
}

void
setkey(void * arg)
{
  void * ptr;

  if ((ptr = pthread_getspecific(key)) != NULL)
    {
      printf("ERROR: Thread %d, Key not initialised to NULL\n",
	     (int) arg);
      exit(1);
    }
  else
    {
      ptr = (void *) malloc(80);
      sprintf((char *) ptr, "Thread %d Key",
	      (int) arg);
      (void) pthread_setspecific(key, ptr);
    }

  if ((ptr = pthread_getspecific(key)) == NULL)
    {
      printf("FAILED: Thread %d Key value set or get failed.\n",
	     (int) arg);
      exit(1);
    }
  else
    {
      printf("SUCCESS: Thread %d Key value set and get succeeded.\n",
	     (int) arg);

      printf("SUCCESS: %s: exiting thread.\n", (char *) ptr);
    }
}

void *
mythread(void * arg)
{
  while (key == NULL)
    {
    }

  printf("Thread %d, Key created\n", (int) arg);

  setkey(arg);

  return 0;

  /* Exiting the thread will call the key destructor. */
}

int
main()
{
  int rc;
  int t;
  pthread_t thread[10];

  for (t = 0; t < 5; t++)
    {
      rc = pthread_create(&thread[t], NULL, mythread, (void *) (t + 1));
      printf("pthread_create returned %d\n", rc);
      if (rc != 0)
	{
	  return 1;
	}
    }

  (void) pthread_once(&key_once, make_key);

  /* Test main thread key. */
  setkey((void *) 0);

  Sleep(500);

  for (t = 5; t < 10; t++)
    {
      rc = pthread_create(&thread[t], NULL, mythread, (void *) (t + 1));
      printf("pthread_create returned %d\n", rc);
      if (rc != 0)
	{
	  return 1;
	}
    }

  Sleep(2000);
  return 0;
}

