/*
 * private.c
 *
 * Description:
 * This translation unit implements routines which are private to
 * the implementation and may be used throughout it.
 */

#include <windows.h>
#include <process.h>
#include "pthread.h"
#include "implement.h"

int
_pthread_getthreadindex(pthread_t thread)
{
  /* The hash table works as follows:
     hash into the table,
     if the thread in this slot doesn't match then start single
     stepping from there until we find it.

     The scheme should have these characteristics:
     - if the thread handle is a sequence number then the hash will
       succeed first time every time,
     - if the thread handle is a pseudo randomish value (eg. a pointer)
       then the hash should succeed first time most times.
   */
  int t = _PTHREAD_HASH_INDEX(*thread);
  int it = t; /* The initial thread index */

  while ((_pthread_threads_table[t])->thread != thread) {
    t++;
    if (t == PTHREAD_THREADS_MAX)
      t = 0; /* Wrap around to the first slot */
    if (t == it)
      return -1; /* Failed to find the thread */
  }
  return t;
}
