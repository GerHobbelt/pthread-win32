#include <pthread.h>
#include <stdio.h>

pthread_key_t key;
pthread_once_t key_once = PTHREAD_ONCE_INIT;

void
destroy_key(void * arg)
{
  /* arg is not NULL if we get to here. */
  printf("SUCCESS: %s: destroying key.\n", (char *) arg);

  free((char *) arg);

  /* Is it our responsibility to do this? */
  arg = NULL;
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

void *
mythread(void * arg)
{
  void * ptr;

  (void) pthread_once(&key_once, make_key);

  if ((ptr = pthread_getspecific(key)) != NULL)
    {
      printf("ERROR: Thread %d, Key 0x%x not initialised to NULL\n",
	     (int) arg,
	     (int) key);
      exit(1);
    }
  else
    {
      ptr = (void *) malloc(80);
      sprintf((char *) ptr, "Thread %d Key 0x%x",
	      (int) arg,
	      (int) key);
      (void) pthread_setspecific(key, ptr);
    }

  if ((ptr = pthread_getspecific(key)) == NULL)
    {
      printf("FAILED: Thread %d Key 0x%x: key value set or get failed.\n",
	     (int) arg,
	     (int) key);
      exit(1);
    }
  else
    {
      printf("SUCCESS: Thread %d Key 0x%x: key value set and get succeeded.\n",
	     (int) arg,
	     (int) key);

      printf("SUCCESS: %s: exiting thread.\n", (char *) ptr);
    }

  return 0;

  /* Exiting the thread will call the key destructor. */
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

  Sleep(2000);
  return 0;
}
