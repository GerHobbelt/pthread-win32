/*
 * misc.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 */

#include "pthread.h"

pthread_t
pthread_self(void)
{
  /* It looks like a pthread_t needs to be a HANDLE, but Win32 also has
     another way of identifying threads: their thread id.  We hope
     that all of the Win32 functions we are going to use only need
     HANDLEs.  The morons. */

  return GetCurrentThread();
}

int
pthread_equal(pthread_t t1, pthread_t t2)
{
  /* For the time being, assume that HANDLEs can be directly compared.
     If not, then use the appropriate Win32 function for
     comparison. */

  return (t1 != t2);
}
