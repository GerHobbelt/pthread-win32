/*
 * cleanup.c
 *
 * Description:
 * This translation unit implements routines associated cleaning up
 * threads.
 */

#include "pthread.h"
#include "implement.h"

void
_pthread_cleanup_push(void (*routine)(void *), void *arg)
{
  _pthread_cleanup_node_t * next;
  int t;

  t = _pthread_getthreadindex(pthread_self());

  next = (_pthread_cleanup_node_t *) malloc(sizeof(_pthread_cleanup_node_t));
  if (next == NULL) {
    /* FIXME: INTERNAL ERROR */
  }

  next->next = _pthread_threads_table[t]->cleanupstack->first;
  next->routine = routine;
  next->arg = arg;
  _pthread_threads_table[t]->cleanupstack->first = next;
}

void
_pthread_cleanup_pop(int execute)
{
  _pthread_cleanup_node_t * handler;
  void (* func)(void *);
  void * arg;
  int t;

  t = _pthread_getthreadindex(pthread_self());
  handler = _pthread_threads_table[t]->cleanupstack->first;

  if (handler != NULL) {
    next = handler->next;
    func = handler->routine;
    arg = handler->arg;

    free(handler);

    if (execute != 0)
      (void) func(arg);

    _pthread_threads_table[t]->cleanupstack->first = next;
  }
}

void
_pthread_do_cancellation(int tindex)
{
  _pthread_cleanup_stack_t * stack;

  stack = _pthread_threads_table[tindex]->cleanupstack;

  /* Run all the cleanup handlers */
  while (stack->first != NULL) {
    _pthread_cleanup_pop(1);
  }
}
