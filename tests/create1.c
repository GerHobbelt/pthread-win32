#include <pthread.h>
#include <stdio.h>
#include <windows.h>

void * func(void * arg)
{
  return 0; 
}
 
int
main()
{
  pthread_t t;
  if (pthread_create(&t, NULL, func, NULL) != 0)
    {
      return 1;
    }

  /* A dirty hack, but we cannot rely on pthread_join in this
     primitive test. */
  Sleep(5000);

  return 0;
}
