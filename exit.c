/*
 * exit.c
 *
 * Description:
 * This translation unit implements routines associated with exiting from
 * a thread.
 */

#include "pthread.h"

void
pthread_exit(void * value)
{
  /* The semantics are such that additional tasks must be done for
     strict POSIX conformance.  We must add code here later which 
     deals with executing cleanup handlers and such.  For now, the
     following is mostly correct: */

  ExitThread((DWORD) value);
}
