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
  _pthread_handler_node_t * new_handler;
  _pthread_handler_node_t * next;
  _pthread_handler_node_t ** stacktop;

  stacktop = _PTHREAD_STACK(stack);

  new_handler = 
    (_pthread_handler_node_t *) malloc(sizeof(_pthread_handler_node_t));

  if (new_handler == NULL)
    {
      return 0; /* NOMEM */
    }

  new_handler->routine = routine;
  new_handler->arg = arg;

  if (poporder == _PTHREAD_HANDLER_POP_LIFO)
    {
      /* Add the new node to the start of the list. */
      new_handler->next = *stacktop;
      *stacktop = new_handler;
    }
  else
    {
      /* Add the new node to the end of the list. */
      new_handler->next = NULL;

      if (*stacktop == NULL)
	{
	  *stacktop = new_handler;
	}
      else
	{
	  next = *stacktop;

	  while (next->next != NULL)
	    {
	      next = next->next;
	    }

	  next->next = new_handler;
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
  _pthread_tsd_key_t * key;
  int count;

  /* This threads private keys */
  key = _pthread_tsd_key_table;

  /* Stop destructor execution at a finite time. POSIX allows us
     to ignore this if we like, even at the risk of an infinite loop.

     FIXME: We don't know when to stop yet.
   */
  for (count = 0; count < PTHREAD_DESTRUCTOR_ITERATIONS; count++)
    {
      int k;

      /* Loop through all keys. */
      for (k = 0; k < _POSIX_THREAD_KEYS_MAX; k++)
	{
	  /* If there's no destructor or the key isn't in use, skip it. */
	  if (key->destructor != NULL && key->in_use == _PTHREAD_TSD_KEY_INUSE)
	    {
	      void * arg;

	      arg = pthread_getspecific((pthread_key_t) k);

	      if (arg != NULL)
		{
		  (void) (key->destructor)(arg);
		}
	    }

	  key++;
	}
    }
}
