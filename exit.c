/*
 * exit.c
 *
 * Description:
 * This translation unit implements routines associated with exiting from
 * a thread.
 */

#include <windows.h>
#include <process.h>
#include "pthread.h"
#include "implement.h"

void
pthread_exit(void * value)
{
  int t;

  t = _pthread_getthreadindex(pthread_self());

  /* Run all the cleanup handlers */
  _pthread_do_cancellation(t);

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_count_mutex);

  /* Frees attr and cleanupstack */
  free(_pthread_threads_table[t]->attr);

  _pthread_threads_table[t]->thread = NULL;

  pthread_mutex_unlock(&_pthread_count_mutex);
  /* END CRITICAL SECTION */

  _endthreadex((DWORD) value);
}
