#include <pthread.h>
#include <stdio.h>
#include <windows.h>

void * func(void * arg)
{
  printf("Hello world\n");
  return 0; 
}
 
int
main()
{
  pthread_t t;
  pthread_create(&t, NULL, func, NULL);

  Sleep(5000);
  return 0;
}
