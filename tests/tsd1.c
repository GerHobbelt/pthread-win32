#include <pthread.h>
#include <stdio.h>

pthread_key_t key;
pthread_once_t key_once = PTHREAD_ONCE_INIT;

void
make_key(void)
{
  (void) pthread_key_create(&key, free);
}

void *
mythread(void * arg)
{
  void *ptr;

  (void) pthread_once(&key_once, make_key);
  if ((ptr = pthread_getspecific(key)) == NULL)
    {
      ptr = malloc(80);
      sprintf(ptr, "Thread %d\n", (int) arg);
      (void) pthread_setspecific(key, ptr);
    }

  if (pthread_getspecific(key) == NULL)
    printf((char *) pthread_getspecific(key));
  else
    printf("Failed %d\n", (int) arg);

  return 0;
}

int
main()
{
  int rc;
  pthread_t t1, t2;
  
  rc = pthread_create(&t1, NULL, mythread, 1);
  printf("pthread_create returned %d\n", rc);

  rc = pthread_create(&t2, NULL, mythread, 2);
  printf("pthread_create returned %d\n", rc);

  Sleep(2000);
  return 0;
}
