#include <pthread.h>
#include <stdio.h>

pthread_once_t once = PTHREAD_ONCE_INIT;

void
myfunc(void)
{
	printf("only see this once\n");
}

void *
mythread(void * arg)
{
   int rc = pthread_once(&once, myfunc);
   printf("returned %d\n", rc); 

   return 0;
}

int
main()
{
  int rc;
  pthread_t t1, t2;
  
  rc = pthread_create(&t1, NULL, mythread, NULL);
  printf("pthread_create returned %d\n", rc);

  rc = pthread_create(&t2, NULL, mythread, NULL);
  printf("pthread_create returned %d\n", rc);

  Sleep(2000);
  return 0;
}
