#include <pthread.h>
#include <stdio.h>
#include <windows.h>

void * func(void * arg)
{
  printf("Hello world %d\n", (int) arg);
  Sleep(2000);
  return arg;
}
 
main()
{
        int rc;
        pthread_t t1, t2;
        rc = pthread_create(&t1, NULL, func, (void *) 1);
        rc = pthread_create(&t1, NULL, func, (void *) 2);

	puts("testing t1 and t2: ");
        if (pthread_equal(t1, t2))
          printf("equal\n");
	else
	  printf("not equal\n");

	puts("testing t1 on itself: ");
        if (pthread_equal(t1,t1))
          printf("equal\n");
        else
	  printf("not equal\n");

        Sleep(8000);

	return 0;
}
