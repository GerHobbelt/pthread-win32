/*
 * misc.c
 *
 * Description:
 * This translation unit implements miscellaneous thread functions.
 */

#include "pthread.h"

pthread_t pthread_self(void)
{
  /* It looks like a pthread_t needs to be a HANDLE, but Win32 also has
     another way of identifying threads: their thread id.  We hope
     that all of the Win32 functions we are going to use only need
     HANDLEs.  The morons. */

  return GetCurrentThread();
}
