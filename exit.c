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
  int t;

  t = _pthread_getthreadindex(pthread_self());
  handler = _pthread_threads_table[t]->cleanupstack->first;

  /* Run all the cleanup handlers */
  while (handler != NULL) {
    void (* func)(void *);
    void * arg;
    _pthread_cleanup_node_t * next;

    func = handler->routine;
    arg = handler->arg;
    _pthread_threads_table[t]->cleanupstack->first = next = handler->next;
    free(handler);
    (void) func(arg);
  }

  /* CRITICAL SECTION */
  pthread_mutex_lock(&_pthread_count_mutex);
  free(_pthread_threads_table[t]->attr);
  _pthread_threads_table[t]->thread = NULL;
  pthread_mutex_unlock(&_pthread_count_mutex);
  /* END CRITICAL SECTION */

  _endthreadex((DWORD) value);
}
