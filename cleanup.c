/*
 * cleanup.c
 *
 * Description:
 * This translation unit implements routines associated cleaning up
 * threads.
 */

#include <errno.h>

#include <malloc.h>
#include "pthread.h"
#include "implement.h"

int
_pthread_handler_push(int stack,
		      int poporder,
		      void (*routine)(void *), 
		      void *arg)
{
  /* Place the new handler into the list so that handlers are
     popped off in the order given by poporder. */
  _pthread_handler_node_t * new_thread;
  _pthread_handler_node_t * next;
  _pthread_handler_node_t ** stacktop;

  stacktop = _PTHREAD_STACK(stack);

  new_thread = 
    (_pthread_handler_node_t *) malloc(sizeof(_pthread_handler_node_t));

  if (new_thread == NULL)
    {
      return 0; /* NOMEM */
    }

  new_thread->routine = routine;
  new_thread->arg = arg;

  if (poporder == _PTHREAD_HANDLER_POP_LIFO)
    {
      /* Add the new node to the start of the list. */
      new_thread->next = *stacktop;
      *stacktop = next;
    }
  else
    {
      /* Add the new node to the end of the list. */
      new_thread->next = NULL;

      if (*stacktop == NULL)
	{
	  *stacktop = new_thread;
	}
      else
	{
	  next = *stacktop;

	  while (next != NULL)
	    {
	      next = next->next;
	    }

	  next = new_thread;
	}
    }
  return 0;
}

void
_pthread_handler_pop(int stack, int execute)
{
  _pthread_handler_node_t ** stacktop;
  _pthread_handler_node_t * next;
  void (* func)(void *);
  void * arg;

  stacktop = _PTHREAD_STACK(stack);

  if (*stacktop != NULL)
    {
      func = (*stacktop)->routine;
      arg = (*stacktop)->arg;
      next = (*stacktop)->next;

      free(*stacktop);
      *stacktop = next;

      if (execute != 0 && func != NULL)
	{
	  (void) func(arg);
	}
    }
}

void
_pthread_handler_pop_all(int stack, int execute)
{
  /* Pop and possibly run all handlers on the given stack. */
  _pthread_handler_node_t ** stacktop;
  _pthread_handler_node_t * next;
  void (* func)(void *);
  void * arg;

  stacktop = _PTHREAD_STACK(stack);

  while (*stacktop != NULL)
    {
      func = (*stacktop)->routine;
      arg = (*stacktop)->arg;
      next = (*stacktop)->next;

      free(*stacktop);
      *stacktop = next;

      if (execute != 0 && func != NULL)
	{
	  (void) func(arg);
	}
    }
}

/* Run destructors for all non-NULL key values for the calling thread.
 */
void
_pthread_destructor_run_all()
{
  _pthread_tsd_key_t * k;
  void * arg;
  int count;

  k = _pthread_tsd_key_table;

  /* Stop destructor execution at a finite time. POSIX allows us
     to ignore this if we like, even at the risk of an infinite loop.
   */
  for (count = 0; count < PTHREAD_DESTRUCTOR_ITERATIONS; count++)
    {
      /* Loop through all keys. */
      for (key = 0; key < _POSIX_THREAD_KEYS_MAX; key++)
	{
	  if (k->in_use != 1)
	    continue;

	  arg = pthread_getspecific(key);

	  if (arg != NULL && k->destructor != NULL)
	    {
	      (void) (k->destructor)(arg);
	    }

	  k++;
	}
    }
}
